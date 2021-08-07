#ifndef __TIME_H__
#define __TIME_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

typedef	uint64_t		clock_t;
typedef	int64_t			time_t;
typedef uint32_t clockid_t;

/* Identifier for system-wide realtime clock.  */
# define CLOCK_REALTIME			0
/* Monotonic system-wide clock.  */
# define CLOCK_MONOTONIC		1
/* High-resolution timer from the CPU.  */
# define CLOCK_PROCESS_CPUTIME_ID	2
/* Thread-specific CPU-time clock.  */
# define CLOCK_THREAD_CPUTIME_ID	3

#define CLOCKS_PER_SEC	(1000000000ULL)

struct tm {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;

	long __tm_gmtoff;
	const char * __tm_zone;
};

struct timeval {
	long tv_sec;
	long tv_usec;
};

// The following structure is used by nanosleep(2), among others:
struct timespec {
	time_t tv_sec;
	long tv_nsec;
};

/* Macros for converting between `struct timeval' and `struct timespec'.  */
# define TIMEVAL_TO_TIMESPEC(tv, ts) {                                   \
        (ts)->tv_sec = (tv)->tv_sec;                                    \
        (ts)->tv_nsec = (tv)->tv_usec * 1000;                           \
}
# define TIMESPEC_TO_TIMEVAL(tv, ts) {                                   \
        (tv)->tv_sec = (ts)->tv_sec;                                    \
        (tv)->tv_usec = (ts)->tv_nsec / 1000;                           \
}

clock_t clock(void);
time_t time(time_t * t);
time_t mktime(struct tm * tm);
double difftime (time_t, time_t);
struct tm * gmtime(const time_t * t);
struct tm * localtime(const time_t * t);
char * asctime(const struct tm * tm);
char * ctime(const time_t * t);
size_t strftime(char * s, size_t max, const char * fmt, const struct tm * t);
int gettimeofday(struct timeval * tv, void * tz);

int __secs_to_tm(long long t, struct tm * tm);
long long __tm_to_secs(const struct tm * tm);

/* Get current value of clock CLOCK_ID and store it in TP.  */
int clock_gettime (clockid_t __clock_id, struct timespec *__tp);

#ifdef __cplusplus
}
#endif

#endif /* __TIME_H__ */
