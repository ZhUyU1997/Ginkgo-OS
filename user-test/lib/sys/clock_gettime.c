#include <time.h>
#include <syscall.h>

int clock_gettime(clockid_t clock_id, struct timespec *tp)
{
    struct timeval tv;
    int retval = -1;

    switch (clock_id)
    {
    case CLOCK_REALTIME:
        retval = gettimeofday(&tv, NULL);
        if (retval == 0)
            /* Convert into `timespec'.  */
            TIMEVAL_TO_TIMESPEC(&tv, tp);
        break;
    default:
        break;
    }
    return retval;
}