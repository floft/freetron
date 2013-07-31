#include "cores.h"

#if defined(linux) || defined(__linux) || defined(__linux__) || \
    defined(sun) || defined(__sun) || \
    defined(__APPLE__)
#include <unistd.h>
int core_count()
{
    static int count = 0;

    if (count == 0)
        count = sysconf(_SC_NPROCESSORS_ONLN);

    // What if that says we don't have a processor?
    // Will that ever happen?
    if (count == 0)
        return DEFAULT_CORES;

    return count;
}
#elif defined(__WIN32) || defined(__WIN32__)
// TODO: don't include hyperthreading
#include <windows.h>
int core_count()
{
    static int count = 0;

    if (count == 0)
    {
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        count = sysinfo.dwNumberOfProcessors;
    }

    if (count == 0)
        return DEFAULT_CORES;

    return count;
}
//#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
// TODO: figure out how to get # of cores on BSD
#else
int core_count() { return DEFAULT_CORES; }
#endif
