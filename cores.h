/*
 * Determine number of CPU cores on different operating systems
 *
 * This is seperated into a different headers since this is one of the
 * only parts of the program that is OS specific. It is messy, so I might
 * as well only have one really messy file.
 */

#ifndef H_CORES
#define H_CORES

// This mess is to determine CPU cores
#if defined(linux) || defined(__linux) || defined(__linux__) || \
    defined(sun) || defined(__sun) || \
    defined(__APPLE__)
#include <unistd.h>
unsigned int core_count()
{
	static unsigned int count = 0;

	if (count == 0)
		count = sysconf(_SC_NPROCESSORS_ONLN);
	
	// What if that says we don't have a processor?
	// Will that ever happen?
	if (count == 0)
		return 1;
	
	return count;
}
#elif defined(__WIN32) || defined(__WIN32__)
// TODO: don't include hyperthreading
#include <windows.h>
unsigned int core_count()
{
	static unsigned int count = 0;

	if (count == 0)
	{
		SYSTEM_INFO sysinfo;
		GetSystemInfo(&sysinfo);
		count = sysinfo.dwNumberOfProcessors;
	}
	
	if (count == 0)
		return 1;

	return count;
}
//#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
// TODO: figure out how to get # of cores on BSD
#else
// Default to using two threads
// TODO: is this bad?
inline unsigned int core_count() { return 2; }
#endif

#endif
