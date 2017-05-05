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
#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>
#include <arpa/inet.h>
#include "struct.h"
#include <vector>

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

static void usage(void) {
    fprintf(stderr,
"triggercounter [FILE]..."
"\n");
    exit(1);
}

// This function zeros out the counters
void CountInit(counts *count)
{
  count->eventn = 0;
  count->passn = 0;
  count->caen = 0;
  count->nhit = 0;
}

// This function just puts a bunch of zeros in a hitinfo struct
// to initialize it
static void InitHits(hitinfo *hits)
{
  hits->time50 = 0;
  hits->time10 = 0;
  hits->triggertype = 0;
  hits->nhit = 0;
  hits->reclen = 0;
  hits->gtid = 0;
  hits->run = 0;
  hits->caen = 0;
  hits->nhit = 0;
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
    if (zrec->bank_name != ZDAB_RECORD) {
        return 1;
    }

    pmtEventPtr = (PmtEventRecord*) (zrec + 1);

    // Read nhit and check that it is sensible
    // If not, throw alarm and return empty object
    SWAP_PMT_RECORD(pmtEventPtr);
    hit.nhit = pmtEventPtr->NPmtHit;
    if (hit.nhit > MAX_NHIT) {
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
    while (*sub_header & SUB_NOT_LAST) {
        uint32_t jump = (*sub_header & SUB_LENGTH_MASK);
        if (jump > MAX_BUFFSIZE/4) {
            fprintf(stderr, "Error: wanted to jump past the end of the buffer\n");
            return(0);
        }
        SWAP_INT32(sub_header, 1);
        sub_header += jump;
        SWAP_INT32(sub_header, 1);
        if ((*sub_header >> SUB_TYPE_BITNUM) == SUB_TYPE_CAEN) {
            hit.caen = 1;
        }
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

int main(int argc, char *argv[])
{
    char filename[256];
    uint64_t clock10 = 0;
    int i;

    if (argc == 0) usage();

    for (i = 0; i < argc; i++) {
        strcpy(filename, argv[i]);
    }
    
    FILE* infile = fopen(filename, "rb");

    PZdabFile* zfile = new PZdabFile();
    if (zfile->Init(infile) < 0) {
        fprintf(stderr, "Did not open file\n");
        exit(1);
    }

    // Initialize the hitinfo object
    hitinfo hits;
    hitinfo hits_last;
    InitHits(&hits);
    InitHits(&hits_last);

    std::vector<uint64_t> clock10_array;

    // Loop over ZDAB Records
    counts count;
    CountInit(&count);
    while (nZDAB * const zrec = zfile->NextRecord()) {
        // If the record has an associated time, compute all the time
        // variables.  Non-hit records don't have times.
        InitHits(&hits);
        if (ReadHits(zrec, hits) == 0) {
            if (hits.triggertype == 0) continue;

            count.eventn++;

            count.nhit += hits.nhit;

            if (hits.caen) count.caen += 1;

            clock10_array.push_back(hits.time10);

            /* Get rid of all events older than 100 ms. */
            for (i = 0; i < clock10_array.size(); i++) {
                if (clock10_array.at(i) < (hits.time10 - 1000000)) {
                    clock10_array.erase(clock10_array.begin()+i);
                }
            }

            int num_events_in_last_second = clock10_array.size();

            /* Print the time between the last event and this event, and
             * the number of events in the last 100 ms. */
            if (!hits.caen) {
                printf("%" PRId64 " %zu\n", (int64_t) hits.time50 - hits_last.time50, clock10_array.size());
            }

            hits_last = hits;

            if ((hits.triggertype & TRIG_PEDESTAL) == 0) {
                if (hits.triggertype & TRIG_NHIT_100_MED) {
                    count.passn += 1;
                }
            }

            if (clock10 == 0) {
                /* First event. Initialize the last 10 MHz clock. */
                clock10 = hits.time10;
            } else if (hits.time10 > clock10 + 1e6) {
                /* print time, events, rate, avg. nhit, caen %. */
                //printf("%" PRIu64 " %-10" PRIu64 " %-10" PRIu64 " %-10.2f %-10.2f\n",
                //       hits.time10,
                //       count.eventn*10,
                //       count.passn*10,
                //       count.nhit/(float) count.eventn,
                //       count.caen*100.0/(float) count.eventn);
                CountInit(&count);
                clock10 = hits.time10;
            }
        } // End Loop for Event Records
    } // End of the Event Loop for this subrun file
    delete zfile;

    return 0;
}
