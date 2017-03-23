// Triggercounter
// K Labe, 24 Mar 2017 

#include "PZdabFile.h"
#include "PZdabWriter.h"
#include <string>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <limits.h>
#include <fstream>
#include <signal.h>
#include <time.h>
#include "struct.h"

/* constants */
#define MAX_BUFFSIZE        0x400000UL  // maximum size of zdab record buffer (4 MB)

// the builder won't put out events with NHIT > 10000
// (note that these are possible due to hardware problems)
// but XSNOED can write an event with up to 10240 channels
#define MAX_NHIT            10240

// Tells us when the 50MHz clock rolls over
static const uint64_t maxtime = (1UL << 43);

// Maximum time allowed between events without a complaint
static const uint64_t maxjump = 10*50000000; // 50 MHz time

// Maximum time drift allowed between two clocks without a complaint
static const int maxdrift = 5000; // 50 MHz ticks (1 us)

// Function to assist in parsing the input variables                  
static double getcmdline_d(const char opt)
{
  char * endptr;
  errno = 0;
  const double answer = strtod(optarg, &endptr);
  if((errno==ERANGE && (fabs(answer) == HUGE_VAL)) ||
     (errno != 0 && answer == 0) ||
     endptr == optarg || *endptr != '\0'){
    char buff[128];
    sprintf(buff, "Triggercounter input %s (given with -%c) isn't a number I"
                  " can handle\n", optarg, opt);
    fprintf(stderr, buff);
    exit(1);
  }
  return answer;
}

// Another function to assist in parsing the input variables
static int getcmdline_l(const char opt)
{
  char * endptr;
  errno = 0;
  const unsigned int answer = strtol(optarg, &endptr, 16);
  if((errno == ERANGE && (answer == UINT_MAX)) || 
     (errno != 0 && answer == 0) || 
     endptr == optarg || *endptr != '\0'){
    char buff[128];
    sprintf(buff, "Triggercounter input %s (given with -%c) isn't a number I"
                  " can handle.\n", optarg, opt);
    fprintf(stderr, buff);
    exit(1);
  }
  return answer;
}

// Prints the Command Line help text
static void printhelp()
{
  printf(
  "TriggerCounter: A ZDAB Utility.\n"
  "\n"
  "Mandatory options:\n"
  "  -i [string]: Input file\n"
  "\n"
  "Misc options\n"
  "  -a [%x] : checkflag\n"
  "  -b [%x] : resultflag\n"
  "  -h: This help text\n"
  );
}

// This function prints some information at the end of the file
static void PrintClosing(char* infilename, counts & count, int checkflag,
                         int resultflag){
  char messg[2048];
  sprintf(messg, "Triggercounter: Subfile %s finished."
                 "  %lu records,  %lu events processed.\n"
                 "%i events pass trigger cut:\n"
                 "checkflag: %x \t resultflag: %x\n",
         infilename, count.recordn, count.eventn,
         count.passn, checkflag, resultflag);
  fprintf(stderr, messg);
}

// This function interprets the command line arguments to the program
static void parse_cmdline(int argc, char ** argv, char * & infilename,
                          int & checkflag, int & resultflag)
{
  const char * const opts = "hi:a:b:";

  bool done = false;
  
  infilename = NULL;
  checkflag = resultflag = 0;

  while(!done){ 
    const char ch = getopt(argc, argv, opts);
    switch(ch){
      case -1: done = true; break;

      case 'i': infilename = optarg; break;
      case 'a': checkflag = getcmdline_l(ch); break;
      case 'b': resultflag = getcmdline_l(ch); break;

      case 'h': printhelp(); exit(0);
      default:  printhelp(); exit(1);
    }
  }

  if(!infilename){
    char buff[128];
    sprintf(buff, "Triggercounter: Must give an input file with -i.  Aborting.\n");
    fprintf(stderr, buff);
    printhelp();
    exit(1);
  }
}

// This function checks the clocks for various anomalies and raises alarms.
// It returns true if the event passes the tests, false otherwise
bool IsConsistent(alltimes & newat, alltimes standard, const int dd){
  // Check for time running backward:
  if(newat.time50 < standard.time50){
    // Is it reasonable that the clock rolled over?
    if((standard.time50 + newat.time50 < maxtime + maxjump) &&
        dd < maxdrift && (standard.time50 > maxtime - maxjump) ){
      fprintf(stderr, "New Epoch\n");
      newat.epoch++;
    }
    else{
      const char msg[128] = "Triggercounter: Time running backward!\n";
      fprintf(stderr, msg);
      return false;
    }  
  }
  // Check that time has not jumped too far ahead
  if(newat.time50 - standard.time50 > maxjump){
    char msg[128] = "Triggercounter: Large time gap between events!\n";
    fprintf(stderr, msg);
    return false;
  }
  else
    return true;
}

// This function calculates the time of an event as measured by the
// varlous clocks we are interested in.
static alltimes compute_times(hitinfo hits, alltimes oldat, counts & count)
{
  static alltimes standard; // Previous unproblematic timestamp
  static bool problem;      // Was there a problem with previous timestamp?
  alltimes newat = oldat;
  // For first event
  if(count.eventn == 1){
    newat.time50 = hits.time50;
    newat.time10 = hits.time10;
    if(newat.time50 == 0) fprintf(stderr, "orphan!\n");
    newat.longtime = newat.time50;
    standard = newat;
    problem = false;
  }
  // Otherwise
  else{
    // Get the current 50MHz Clock Time
    // Implementing Part of Method Get50MHzTime() 
    // from PZdabFile.cxx
    newat.time50 = hits.time50;

    // Get the current 10MHz Clock Time
    // Method taken from zdab_convert.cpp
    newat.time10 = hits.time10;

    // Check for consistency between clocks
    const int dd = ( (oldat.time10 - newat.time10)*5 > oldat.time50 - newat.time50 ? 
                     (oldat.time10 - newat.time10)*5 - (oldat.time50 - newat.time50) :
                     (oldat.time50 - newat.time50) - (oldat.time10 - newat.time10)*5 );
    if (dd > maxdrift){
      char msg[128];
      sprintf(msg, "Triggercounter: The 50MHz clock jumped by %i ticks relative"
                   " to the 10MHz clock!\n", dd);
      fprintf(stderr, msg);
    }

    // Check for retriggers
    if (newat.time50 - oldat.time50 > 0 &&
        newat.time50 - oldat.time50 <= 23){
      fprintf(stderr, "retrigger!\n");
    }

    // Check for pathological case
    if (newat.time50 == 0){
      newat.time50 = oldat.time50;
      fprintf(stderr, "orphan!\n");
      return newat;
    }

    // Check for well-orderedness
    if(IsConsistent(newat, standard, dd)){
      newat.longtime = newat.time50 + maxtime*newat.epoch;
      standard = newat;
      problem = false;
    }
    else if(problem){
      // RESET EVERYTHING
      fprintf(stderr, "Triggercounter: Events out of order - Resetting buffers.\n");
      newat.epoch = 0;
      newat.longtime = newat.time50;
      newat.exptime = 0; 
      standard = newat;
      problem = false;
    }
    else{
      problem = true;
      newat = standard;
    }
  }
  return newat;
}

// This function zeros out the counters
counts CountInit(){
  counts count;
  count.eventn = 0;
  count.recordn = 0;
  count.passn = 0;
  return count;
}

// This function initialzes the time object
static alltimes InitTime(){
  alltimes alltime;
  alltime.walltime = 0;
  alltime.oldwalltime = 0;
  alltime.exptime = 0;
  alltime.epoch = 0;
  return alltime;
}

// This function just puts a bunch of zeros in a hitinfo struct
// to initialize it
static hitinfo InitHit(){
  hitinfo hit;
  hit.time50 = 0;
  hit.time10 = 0;
  hit.triggertype = 0;
  hit.nhit = 0;
  hit.reclen = 0;
  hit.gtid = 0;
  hit.run = 0;
  return hit;
}

// This function reads out the information about each event that we need
// for making decisions/processing.  It then restores the hit data to its
// external format.
// This is based on PZdabFile::GetPmtRecord but it does not leave things
// in a byte-swapped state.
// If the function is passed a non ZDAB_RECORD it returns 1.
static int ReadHits(nZDAB* zrec, hitinfo& hit){
  PmtEventRecord* pmtEventPtr;
  // Check that the record is a ZDAB bank
  if( zrec->bank_name != ZDAB_RECORD ){
    return 1;
  }

  pmtEventPtr = (PmtEventRecord*) (zrec + 1);

  // Read nhit and check that it is sensible
  // If not, throw alarm and return empty object
  SWAP_PMT_RECORD( pmtEventPtr );
  hit.nhit = pmtEventPtr->NPmtHit;
  if(hit.nhit > MAX_NHIT){
    fprintf(stderr, "Read error: Bad ZDAB -- %d pmt hit!\x07\n", hit.nhit);
    fprintf(stderr, "Too many hits found!\n");
    return 1;
  }

  // Read the gtid and run number
  hit.gtid = pmtEventPtr->TriggerCardData.BcGT;
  hit.run  = pmtEventPtr->RunNumber;

  // Read the 50 MHz and 10 MHz clock times
  // This method copied from PZdabFile
  hit.time50 = (uint64_t(pmtEventPtr->TriggerCardData.Bc50_2) << 11)
                        + pmtEventPtr->TriggerCardData.Bc50_1;
  hit.time10 = (uint64_t(pmtEventPtr->TriggerCardData.Bc10_2) << 32)
                        + pmtEventPtr->TriggerCardData.Bc10_1;

  // Next retrieve the trigger word
  // This method copied from zdab_convert
  uint32_t mtcwords[6];
  memcpy(mtcwords, &(pmtEventPtr->TriggerCardData), 6*sizeof(uint32_t));
  hit.triggertype = ((mtcwords[3] & 0xff000000) >> 24) |
                    ((mtcwords[4] & 0x3ffff) << 8);

  // Then report the length of the record in words
  // 9 words for nZDAB, 8 words for PmtEventRecord, 3 words per nhit
  // plus the length of any subrecords
  // This method copied from PZdabFile
  uint32_t event_size = 17 + 3*hit.nhit;
  uint32_t* sub_header = &pmtEventPtr->CalPckType;
  while( *sub_header & SUB_NOT_LAST ){
    uint32_t jump = (*sub_header & SUB_LENGTH_MASK);
    if( jump > MAX_BUFFSIZE/4 ){
      fprintf(stderr, "Error: wanted to jump past the end of the buffer\n");
      return(0);
    }
    SWAP_INT32(sub_header, 1);
    sub_header += jump;
    SWAP_INT32(sub_header, 1);
    uint32_t datawords = (*sub_header & SUB_LENGTH_MASK);
    event_size += datawords;
    SWAP_INT32(sub_header+1, datawords-1);
  }
  SWAP_INT32(sub_header, 1);
  hit.reclen = event_size;

  // Finally, restore the record to its external state
  SWAP_PMT_RECORD( pmtEventPtr );
  SWAP_INT32( pmtEventPtr+1, 3*hit.nhit);
  return 0;
}

// MAIN FUCTION 
int main(int argc, char *argv[])
{
  // Configure the system
  char * infilename = NULL;
  int checkflag = 0;
  int resultflag = 0;

  parse_cmdline(argc, argv, infilename, checkflag, resultflag);

  FILE* infile = fopen(infilename, "rb");

  PZdabFile* zfile = new PZdabFile();
  if (zfile->Init(infile) < 0){
    fprintf(stderr, "Did not open file\n");
    exit(1);
  }

  // Initialize the various clocks and the hitinfo object
  alltimes alltime = InitTime();
  hitinfo hits = InitHit();

  // Loop over ZDAB Records
  counts count = CountInit();
  while(nZDAB * const zrec = zfile->NextRecord()){
    // If the record has an associated time, compute all the time
    // variables.  Non-hit records don't have times.
    if(! ReadHits(zrec, hits)){
      count.eventn++;
      alltime = compute_times(hits, alltime, count);

      uint32_t word = hits.triggertype; 

      // Check trigger word conditions
      if((word & checkflag) == resultflag){
        count.passn++;
      }
    } // End Loop for Event Records
    count.recordn++;
  } // End of the Event Loop for this subrun file
  delete zfile;

  PrintClosing(infilename, count, checkflag, resultflag);
  return 0;
}
