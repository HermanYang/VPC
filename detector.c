#ifdef __cplusplus
extern "C" {
#endif


#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "detector.h"
#include "utils.h"


void init_detector(Detector* self)
{
    self->_ptr_last_frame_data = NULL;
    self->_dyn_frame_index_array_size = 0;
    self->_dyn_frame_index_array_max_size = BUFFER_UNIT_SIZE;
    self->_ptr_dyn_frame_index_array = NULL;

    self->_period_array.ptr_array = NULL;
    self->_period_array._max_size = BUFFER_UNIT_SIZE;
    self->_period_array.size = 0;

    self->_ptr_dyn_frame_index_array = calloc(BUFFER_UNIT_SIZE, sizeof(uint32_t));

    self->_period_array.ptr_array = calloc(BUFFER_UNIT_SIZE, sizeof(Period));

}

uint32_t is_dynamic_frame(Detector* self, void* ptr_Y_data, uint32_t width, uint32_t height, Region* ptr_region)
{
    if(self->_ptr_last_frame_data == NULL)
    {
        self->_ptr_last_frame_data = malloc(width * height * 1);
        memcpy(self->_ptr_last_frame_data, ptr_Y_data, width * height);
        return TRUE;
    }

    uint32_t i;
    uint32_t fluctuated_pixels = 0;

    uint32_t length = width * height;

    unsigned char* ptr_last_frame_data = self->_ptr_last_frame_data;
    unsigned char* ptr_current_frame_data = ptr_Y_data;

    unsigned char diff;


    if (ptr_region == NULL)
    {
        for(i = 0; i < length; ++i)
        {
            diff = abs(ptr_current_frame_data[i] - ptr_last_frame_data[i]);

            if( diff > DIFF_THRESHOLD )
            {
                ++fluctuated_pixels;
            }
        }
    }
    else
    {
        uint32_t j;
        uint32_t left = ptr_region->left;
        uint32_t top = ptr_region->top;
        uint32_t right = ptr_region->right;
        uint32_t bottom = ptr_region->bottom;
        uint32_t index;

        for(i = top; i <= bottom; ++i)
        {
            for(j = left; j <= right; ++j)
            {
                index = i * width + j;
                diff = abs(ptr_current_frame_data[index] - ptr_last_frame_data[index]);

                if( diff > DIFF_THRESHOLD )
                {
                    ++fluctuated_pixels;
                }

            }
        }

    }

    memcpy(self->_ptr_last_frame_data, ptr_Y_data, width * height);

    if( fluctuated_pixels > FLUCTUATION_THRESHOLD )
    {
        return TRUE;
    }

    return FALSE;
}

Period_Array* get_dynamic_period_array(Detector* self)
{
    // needn't do it twice
    if(self->_period_array.size > 0)
    {
        return &(self->_period_array);
    }

    uint32_t i;
    uint32_t size = self->_dyn_frame_index_array_size;
    uint32_t frame_index;

    Period period;

    period.begin_index = -1;
    period.end_index = -1;


    for(i = 0; i < size;)
    {
        frame_index = self->_ptr_dyn_frame_index_array[i];

        if(period.begin_index == -1 && period.end_index == -1)
        {
            period.begin_index = frame_index;
            period.end_index = frame_index;
        }

        if(frame_index - period.end_index <= MAX_GAP)
        {
            period.end_index = frame_index;
            ++i;
        }
        else
        {
            if(period.end_index - period.begin_index >= MIN_FRAME_SEQUENCE)
            {
                _push_back_period(self, period.begin_index, period.end_index);
            }

            period.begin_index = -1;
            period.end_index = -1;
        }

    }

    if(period.begin_index != -1 && period.end_index != -1)
    {
        if(period.end_index - period.begin_index >= MIN_FRAME_SEQUENCE)
        {
            _push_back_period(self, period.begin_index, period.end_index);
        }
    }

    return &(self->_period_array);
}

void detect(Detector* self, void* ptr_Y_data, uint32_t frame_index, uint32_t width, uint32_t height)
{
    if( is_dynamic_frame(self, ptr_Y_data, width, height, NULL) )
    {
        _push_back_frame_index(self, frame_index);
    }
}

void detect_region(Detector* self, void* ptr_Y_data, uint32_t frame_index, uint32_t width, uint32_t height, Region region)
{
    if( is_dynamic_frame(self, ptr_Y_data, width, height, &region) )
    {
        _push_back_frame_index(self, frame_index);
    }
}

void static _push_back_frame_index(Detector* self, uint32_t frame_index)
{
    if( self->_dyn_frame_index_array_size < self->_dyn_frame_index_array_max_size )
    {
        self->_ptr_dyn_frame_index_array[self->_dyn_frame_index_array_size] = frame_index;
    }
    else
    {
        self->_dyn_frame_index_array_max_size += BUFFER_UNIT_SIZE;

        self->_ptr_dyn_frame_index_array = realloc(self->_ptr_dyn_frame_index_array, self->_dyn_frame_index_array_max_size * sizeof(uint32_t));

        self->_ptr_dyn_frame_index_array[self->_dyn_frame_index_array_size] = frame_index;
    }

    ++(self->_dyn_frame_index_array_size);
}

void static _push_back_period(Detector* self, uint32_t begin_index, uint32_t end_index)
{
    if( self->_period_array.size < self->_period_array._max_size)
    {
        self->_period_array.ptr_array[self->_period_array.size].begin_index = begin_index;
        self->_period_array.ptr_array[self->_period_array.size].end_index = end_index;
    }
    else
    {
        self->_period_array._max_size += BUFFER_UNIT_SIZE;

        self->_period_array.ptr_array = realloc(self->_period_array.ptr_array, self->_period_array._max_size * sizeof(Period));

        self->_period_array.ptr_array[self->_period_array.size].begin_index = begin_index;
        self->_period_array.ptr_array[self->_period_array.size].end_index = end_index;
    }

    ++(self->_period_array.size);
}

void release_detector(Detector* self)
{
    free(self->_ptr_last_frame_data);
    self->_ptr_last_frame_data = NULL;

    free(self->_ptr_dyn_frame_index_array);

    self->_ptr_dyn_frame_index_array = NULL;
}

#ifdef __cplusplus
}
#endif
