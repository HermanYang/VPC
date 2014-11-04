#ifndef DECODER_H
#define DECODER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "define.h"

#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavformat/avformat.h>

typedef struct _Decoder
{
    AVCodec* _codec;
    AVFrame* _frame;
    AVStream* _stream;
    AVCodecContext* _codec_context;
    AVFormatContext* _format_context;
    int32_t _first_frame_index;
    void* master;
    void (*decode_handler)(void* master, uint8_t* Y_data, uint8_t* U_data, uint8_t* V_data, uint32_t index, uint32_t width, uint32_t height);
}Decoder;

void init_decoder(Decoder* self, const char* filename, const char* format_name);
void release_decoder(Decoder* self);

void decode(Decoder* self);
void decode_period(Decoder* self, uint32_t begin_index, uint32_t end_index);

void get_frame_by_index(Decoder* self, uint32_t frame_index, uint8_t* buf, uint32_t max_size);

int64_t get_duration(Decoder *self);
uint32_t get_fps(Decoder* self);
uint32_t get_width(Decoder* self);
uint32_t get_height(Decoder* self);

#ifdef __cplusplus
}
#endif

#endif

