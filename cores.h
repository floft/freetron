/*
 * Determine number of CPU cores on different operating systems
 *
 * This is separated into a different headers since this is one of the
 * only parts of the program that is OS specific. It is messy, so I might
 * as well only have one really messy file.
 */

#ifndef H_CORES
#define H_CORES

// Default to using two threads
#define DEFAULT_CORES 2

// Different for every OS...
int core_count();

#endif
