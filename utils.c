#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

#include "utils.h"


void time_record_start(TimePeriod* self)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
    self->_start = tv.tv_sec + tv.tv_usec / 1000000.0;
}

double time_record_end(TimePeriod* self)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
    self->_end= tv.tv_sec + tv.tv_usec / 1000000.0;

    return self->_end - self->_start;
}

void get_current_time_string(char buf[], uint32_t size)
{
	time_t t;
	struct tm* ts;
	time(&t);
	ts = localtime(&t);
    strftime(buf, size, "%Y_%m_%d_%H_%M_%S", ts);
}

#ifdef __cplusplus
}
#endif
