// Struct Header
//
// K Labe, September 24 2014
// K Labe, February 4 2015 - Add hitinfo struct

#include <stdint.h>

// Structure to hold all the things we count
struct counts
{
uint64_t passn;
uint64_t eventn;
/* Number of events with caen. */
uint64_t caen;
uint64_t nhit;
};

// Structure to hold all the information we want to read out of the hits
// object of a ZDAB record
struct hitinfo
{
uint64_t time50;
uint64_t time10;
uint32_t triggertype;
uint16_t nhit;
uint32_t reclen;
uint32_t gtid;
uint32_t run;
int caen;
};
