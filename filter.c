#include "filter.h"
#include "utils.h"

#include <stdio.h>

void init_filter(Filter* ptr_self, const char* filename)
{
    FILE* file = fopen(filename, "r");
    int ret;

    if( file == NULL )
    {
        perror("cannot open file");
    }

    ptr_self->_ptr_data = NULL;
    ptr_self->_width = 64;
    ptr_self->_height = 64;
    uint32_t size = ptr_self->_width * ptr_self->_height * 1.5;

    ptr_self->_ptr_data = malloc(size);

    ret = fread(ptr_self->_ptr_data, size, 1, file);

    if (ret == 0)
    {
        perror("file read error");
        return;
    }

    fclose(file);
}


void filter_overlay(Filter* ptr_self, uint32_t x, unsigned y, uint32_t width, uint32_t height, uint8_t* ptr_frame_Y_data, uint8_t* ptr_frame_U_data, uint8_t* ptr_frame_V_data)
{
    uint32_t size = ptr_self->_width * ptr_self->_height;

    uint8_t* ptr_Y_data = ptr_self->_ptr_data;
    uint8_t* ptr_U_data = ptr_Y_data + size;
    uint8_t* ptr_V_data = ptr_Y_data + size + size/4;
    uint32_t i, j;
    uint8_t Y, U, V;

    for(i = 0; i < ptr_self->_height; ++i)
    {
        for(j = 0; j < ptr_self->_width ; ++j)
        {
            Y = ptr_Y_data[i * ptr_self->_width + j];

            if(Y != YUV_Y_BLACK)
            {
                ptr_frame_Y_data[(i + x) * width + j + y] = Y;
            }
        }
    }

    for(i = 0; i < ptr_self->_height / 2; ++i)
    {
        for(j = 0; j < ptr_self->_width / 2; ++j)
        {
            U = ptr_U_data[i * ptr_self->_width / 2 + j];
            V = ptr_V_data[i * ptr_self->_width / 2 + j];

            if (U != YUV_U_BLACK)
            {
                ptr_frame_U_data[(i + x / 2) * width / 2 + j + y / 2] = U;
            }

            if (V != YUV_V_BLACK)
            {
                ptr_frame_V_data[(i + x / 2) * width / 2 + j + y / 2] = V;
            }
        }
    }

}

void release_filter(Filter* ptr_self)
{
    free(ptr_self->_ptr_data);
}
