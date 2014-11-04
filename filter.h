#ifndef FILTER_H
#define FILTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "define.h"

#define YUV_Y_BLACK 16
#define YUV_U_BLACK 128
#define YUV_V_BLACK 128

typedef struct _Filter
{
    void* _ptr_data;
    int _width;
    int _height;
}Filter;

void init_filter(Filter* ptr_self, const char* filename);

void filter_overlay(Filter* ptr_self, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint8_t* Y_data, uint8_t* U_data, uint8_t* V_data);

void release_filter(Filter* ptr_self);

#ifdef __cplusplus
}
#endif

#endif
