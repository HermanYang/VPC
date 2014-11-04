#ifndef MOD_H
#define MOD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "define.h"

#define FALSE 0
#define TRUE  1

#define DIFF_THRESHOLD 50
#define FLUCTUATION_THRESHOLD 50

#define IMAGE_MAX  255
#define IMAGE_MIN  0

#define BUFFER_UNIT_SIZE 500


#define MAX_GAP 100
#define MIN_FRAME_SEQUENCE 25

typedef struct _Period
{
    uint32_t begin_index;
    uint32_t end_index;
}Period;

typedef struct _Period_Array
{
    uint32_t size;
    uint32_t _max_size;
    Period* ptr_array;
}Period_Array;

typedef struct _Detector
{
    void* _ptr_last_frame_data;

    uint32_t _dyn_frame_index_array_size;
    uint32_t _dyn_frame_index_array_max_size;
    uint32_t* _ptr_dyn_frame_index_array;

    Period_Array _period_array;
}Detector;

typedef struct _Region
{
    uint32_t left;
    uint32_t top;
    uint32_t right;
    uint32_t bottom;
}Region;


void init_detector(Detector* self);

uint32_t is_dynamic_frame(Detector* self, void* ptr_Y_data, uint32_t width, uint32_t height, Region* ptr_region);

Period_Array* get_dynamic_period_array(Detector* self);

void detect(Detector* self, void* ptr_Y_data, uint32_t frame_index, uint32_t width, uint32_t height);
void detect_region(Detector* self, void* ptr_Y_data, uint32_t frame_index, uint32_t width, uint32_t height, Region region);

void static _push_back_frame_index(Detector* self, uint32_t frame_index);
void static _push_back_period(Detector* self, uint32_t begin_index, uint32_t end_index);

void release_detector(Detector* self);
#ifdef __cplusplus
}
#endif

#endif
