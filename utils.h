#ifndef UTILS_H 
#define UTILS_H 

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include "define.h"

#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define FPS 15

typedef struct _TimePeriod
{
	double _start;
	double _end;
}TimePeriod;

void print_error(const char* format, ...);
void print_info(const char* format, ...);

void time_record_start(TimePeriod* self);
double time_record_end(TimePeriod* self);

void get_current_time_string(char buf[],uint32_t size);

#ifdef __cplusplus
}
#endif

#endif
