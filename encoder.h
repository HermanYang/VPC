#ifndef ENCODER_H
#define ENCODER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "define.h"

#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>

typedef struct _Encoder
{
    uint32_t frame_count;
    AVFormatContext* 	_format_context;
    AVCodecContext* 	_codec_context;
    AVOutputFormat*		_out_format;
    AVStream*			_stream;
    AVCodec* 			_codec;
    AVFrame* 			_frame;
}Encoder;

void init_encoder(Encoder* self, const char* filename, uint32_t width, uint32_t height, uint32_t fps);

bool is_init(Encoder* self);

void encode(Encoder* self, uint8_t* Y_data, uint8_t* U_data, uint8_t* V_data);

void release_encoder(Encoder* self);

static void _encode_delay_frame(Encoder* self);

#ifdef __cplusplus
}
#endif

#endif
