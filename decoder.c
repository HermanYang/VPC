#include "decoder.h"
#include "utils.h"

#include <math.h>
#include <stdlib.h>
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>

void init_decoder(Decoder* self, const char* filename, const char* format_name)
{
    uint32_t stream_id = 0;
    AVInputFormat* input_format = NULL;

    self->_codec = NULL;
    self->_codec_context = NULL;
    self->_frame = NULL;
    self->_stream = NULL;
    self->_format_context = NULL;

    self->decode_handler = NULL;
    self->master = NULL;

    self->_first_frame_index = -1;

    av_register_all();

    avdevice_register_all();

    // create format context
    if(format_name != NULL)
    {
        input_format = av_find_input_format(format_name);
    }

    avformat_open_input(&self->_format_context, filename, input_format, NULL );
    avformat_find_stream_info(self->_format_context, NULL);

    // get stream
    stream_id = av_find_best_stream(self->_format_context, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);

    self->_stream = self->_format_context->streams[stream_id];
    self->_codec_context = self->_stream->codec;
    self->_codec_context->thread_count = 3;

    self->_codec = avcodec_find_decoder(self->_codec_context->codec_id);
    self->_codec->capabilities |= CODEC_CAP_SLICE_THREADS;

    // open decoder
    avcodec_open2(self->_codec_context, self->_codec, NULL);

    // create frame
    self->_frame = av_frame_alloc();

    av_dump_format(self->_format_context, 0, filename, 0);
}

void decode(Decoder* self)
{
    int32_t got_frame = 0;
    uint32_t index = 0;

    int32_t width = self->_codec_context->width;
    int32_t height = self->_codec_context->height;

    struct SwsContext* sws_context = sws_getContext(width, height, self->_codec_context->pix_fmt, width, height, PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

    uint8_t* Y_data = NULL;
    uint8_t* U_data = NULL;
    uint8_t* V_data = NULL;

    AVFrame* yuv_frame = av_frame_alloc();
    uint8_t* buf = av_malloc(avpicture_get_size(PIX_FMT_YUV420P, width, height));
    avpicture_fill((AVPicture*)yuv_frame, buf, PIX_FMT_YUV420P, width, height);

    AVPacket avpkt;

    av_init_packet(&avpkt);

    avpkt.data = NULL;
    avpkt.size = 0;

    while(av_read_frame(self->_format_context, &avpkt) >= 0)
    {
        do
        {
            int32_t ret = avcodec_decode_video2(self->_codec_context, self->_frame, &got_frame, &avpkt);

            if( ret < 0 )
            {
                perror("Error decoding frame");
                exit(EXIT_FAILURE);
            }

            if(got_frame)
            {

                if(self->decode_handler != NULL)
                {
                    index = av_rescale_q_rnd(self->_frame->pkt_pts,  self->_stream->time_base, self->_codec_context->time_base, AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);
                    index = index / self->_codec_context->ticks_per_frame;

                    if( self->_first_frame_index < 0)
                    {
                        self->_first_frame_index = index;
                    }

                    if( self->_first_frame_index > 0 )
                    {
                        index = index - self->_first_frame_index;
                    }

                    sws_scale(sws_context, (const uint8_t* const*)self->_frame->data, self->_frame->linesize, 0, height, yuv_frame->data, yuv_frame->linesize);

                    Y_data = yuv_frame->data[0];
                    U_data = yuv_frame->data[1];
                    V_data = yuv_frame->data[2];

                    self->decode_handler(self->master, Y_data, U_data, V_data, index, width, height);
                }

            }

            avpkt.data += ret;
            avpkt.size -= ret;
        } while (avpkt.size > 0);

        av_free_packet(&avpkt);
    }

    av_free_packet(&avpkt);

    // decode delayed frames
    avpkt.data = NULL;
    avpkt.size = 0;

    do
    {
        int32_t ret = avcodec_decode_video2(self->_codec_context, self->_frame, &got_frame, &avpkt);

        if( ret < 0 )
        {
            perror("Error decoding frame");
            exit(EXIT_FAILURE);
        }

        if(got_frame)
        {

            if(self->decode_handler != NULL)
            {
                index = av_rescale_q_rnd(self->_frame->pkt_pts,  self->_stream->time_base, self->_codec_context->time_base, AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);
                index = index / self->_codec_context->ticks_per_frame;

                sws_scale(sws_context, (const uint8_t* const*)self->_frame->data, self->_frame->linesize, 0, height, yuv_frame->data, yuv_frame->linesize);

                Y_data = yuv_frame->data[0];
                U_data = yuv_frame->data[1];
                V_data = yuv_frame->data[2];

                self->decode_handler(self->master, Y_data, U_data, V_data, index, width, height);
            }

        }

    }
    while(got_frame);

    av_free(buf);
    av_frame_free(&yuv_frame);
    av_free_packet(&avpkt);
    sws_freeContext(sws_context);
}

void decode_period(Decoder* self, uint32_t begin_index, uint32_t end_index)
{
    int32_t got_frame;
    uint32_t index = 0;

    uint32_t width = self->_codec_context->width;
    uint32_t height = self->_codec_context->height;

    struct SwsContext* sws_context = sws_getContext(width, height, self->_codec_context->pix_fmt, width, height, PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

    uint8_t* Y_data = NULL;
    uint8_t* U_data = NULL;
    uint8_t* V_data = NULL;

    AVFrame* yuv_frame = av_frame_alloc();
    uint8_t* buf = av_malloc(avpicture_get_size(PIX_FMT_YUV420P, width, height));
    avpicture_fill((AVPicture*)yuv_frame, buf, PIX_FMT_YUV420P, width, height);


    AVPacket avpkt;

    av_init_packet(&avpkt);

    avpkt.data = NULL;
    avpkt.size = 0;


    int64_t timestamp = begin_index * self->_codec_context->ticks_per_frame;

    timestamp = av_rescale_q_rnd(timestamp, self->_codec_context->time_base, self->_stream->time_base, AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);

    av_seek_frame(self->_format_context, self->_stream->index, timestamp, AVSEEK_FLAG_BACKWARD);

    timestamp = end_index * self->_codec_context->ticks_per_frame;

    timestamp = av_rescale_q_rnd(timestamp, self->_codec_context->time_base, self->_stream->time_base, AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);

    while(av_read_frame(self->_format_context, &avpkt) >= 0)
    {
        do
        {
            int32_t ret = avcodec_decode_video2(self->_codec_context, self->_frame, &got_frame, &avpkt);

            if( ret < 0 )
            {
                perror("Error decoding frame");
                exit(EXIT_FAILURE);
            }


            if(got_frame)
            {
                if(self->decode_handler != NULL)
                {
                    if(self->_frame->pkt_pts > timestamp)
                    {
                        av_free(buf);
                        av_frame_free(&yuv_frame);
                        av_free_packet(&avpkt);
                        sws_freeContext(sws_context);
                        return;
                    }

                    index = av_rescale_q_rnd(self->_frame->pkt_pts,  self->_stream->time_base, self->_codec_context->time_base, AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);
                    index = index / self->_codec_context->ticks_per_frame;

                    sws_scale(sws_context, (const uint8_t* const*)self->_frame->data, self->_frame->linesize, 0, height, yuv_frame->data, yuv_frame->linesize);

                    Y_data = yuv_frame->data[0];
                    U_data = yuv_frame->data[1];
                    V_data = yuv_frame->data[2];

                    self->decode_handler(self->master, Y_data, U_data, V_data, index, width, height);
                }
            }

            avpkt.data += ret;
            avpkt.size -= ret;

        } while (avpkt.size > 0);
        av_free_packet(&avpkt);
    }

    av_free_packet(&avpkt);

    avpkt.data = NULL;
    avpkt.size = 0;

    do
    {
        int32_t ret = avcodec_decode_video2(self->_codec_context, self->_frame, &got_frame, &avpkt);

        if( ret < 0 )
        {
            perror("Error decoding frame");
            exit(EXIT_FAILURE);
        }

        if(got_frame)
        {

            if(self->decode_handler != NULL)
            {
                if(self->_frame->pkt_pts > timestamp)
                {
                    av_free(buf);
                    av_free_packet(&avpkt);
                    av_frame_free(&yuv_frame);
                    sws_freeContext(sws_context);
                    return;
                }

                index = av_rescale_q_rnd(self->_frame->pkt_pts,  self->_stream->time_base, self->_codec_context->time_base, AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);
                index = index / self->_codec_context->ticks_per_frame;


                sws_scale(sws_context, (const uint8_t* const*)self->_frame->data, self->_frame->linesize, 0, height, yuv_frame->data, yuv_frame->linesize);

                Y_data = yuv_frame->data[0];
                U_data = yuv_frame->data[1];
                V_data = yuv_frame->data[2];

                self->decode_handler(self->master, Y_data, U_data, V_data, index, width, height);
            }
        }

    }
    while(got_frame);

    av_free(buf);
    av_free_packet(&avpkt);
    av_frame_free(&yuv_frame);
    sws_freeContext(sws_context);
}

void get_frame_by_index(Decoder* self, uint32_t frame_index, uint8_t* buf, uint32_t max_size)
{
    int32_t got_frame = 0;
    uint32_t index = 0;

    AVPacket avpkt;
    uint32_t width = self->_codec_context->width;
    uint32_t height = self->_codec_context->height;


    uint32_t size = width * height;

    struct SwsContext* sws_context = NULL;

    uint8_t* Y_data = NULL;
    uint8_t* U_data = NULL;
    uint8_t* V_data = NULL;

    AVFrame* yuv_frame = av_frame_alloc();
    uint8_t* yuv_buf = av_malloc(avpicture_get_size(PIX_FMT_YUV420P, width, height));
    avpicture_fill((AVPicture*)yuv_frame, buf, PIX_FMT_YUV420P, width, height);

    av_init_packet(&avpkt);

    avpkt.data = NULL;
    avpkt.size = 0;

    if ( max_size < size * 1.5)
    {
        perror("Buffer size is not big enough");
        return;
    }

    int64_t timestamp = frame_index * self->_codec_context->ticks_per_frame;

    timestamp = av_rescale_q_rnd(timestamp, self->_codec_context->time_base, self->_stream->time_base, AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);

    av_seek_frame(self->_format_context, self->_stream->index, timestamp, AVSEEK_FLAG_BACKWARD);


    while(av_read_frame(self->_format_context, &avpkt) >= 0)
    {
        do
        {
            int32_t ret = avcodec_decode_video2(self->_codec_context, self->_frame, &got_frame, &avpkt);

            if( ret < 0 )
            {
                perror("Error decoding frame");
                exit(EXIT_FAILURE);
            }

            if(got_frame)
            {
                index = av_rescale_q_rnd(self->_frame->pkt_pts,  self->_stream->time_base, self->_codec_context->time_base, AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);
                index = index / self->_codec_context->ticks_per_frame;

                if ( index >= frame_index )
                {
                    sws_context = sws_getContext(width, height, self->_frame->format, width, height, PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
                    sws_scale(sws_context, (const uint8_t* const*)self->_frame->data, self->_frame->linesize, 0, height, yuv_frame->data, yuv_frame->linesize);
                    sws_freeContext(sws_context);

                    Y_data = yuv_frame->data[0];
                    U_data = yuv_frame->data[1];
                    V_data = yuv_frame->data[2];

                    memcpy(buf, Y_data, size);
                    memcpy(buf + size, U_data, size / 4);
                    memcpy(buf + size + size / 4, V_data, size / 4);

                    av_free_packet(&avpkt);
                    av_frame_free(&yuv_frame);
                    av_free(yuv_buf);

                    return;
                }
            }

            avpkt.data += ret;
            avpkt.size -= ret;

        } while (avpkt.size > 0);

    }

    avpkt.data = NULL;
    avpkt.size = 0;

    do
    {
        int32_t ret = avcodec_decode_video2(self->_codec_context, self->_frame, &got_frame, &avpkt);

        if( ret < 0 )
        {
            perror("Error decoding frame");
            exit(EXIT_FAILURE);
        }

        if(got_frame)
        {
            index = av_rescale_q_rnd(self->_frame->pkt_pts,  self->_stream->time_base, self->_codec_context->time_base, AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);
            index = index / self->_codec_context->ticks_per_frame;

            if ( index >= frame_index )
            {
                sws_context = sws_getContext(width, height, self->_frame->format, width, height, PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
                sws_scale(sws_context, (const uint8_t* const*)self->_frame->data, self->_frame->linesize, 0, height, yuv_frame->data, yuv_frame->linesize);
                sws_freeContext(sws_context);

                Y_data = yuv_frame->data[0];
                U_data = yuv_frame->data[1];
                V_data = yuv_frame->data[2];


                memcpy(buf, Y_data, size);
                memcpy(buf + size, U_data, size / 4);
                memcpy(buf + size + size / 4, V_data, size / 4);

                av_free_packet(&avpkt);
                av_frame_free(&yuv_frame);
                av_free(yuv_buf);

                return;
            }
        }

    }
    while(got_frame);

    av_free_packet(&avpkt);
    av_frame_free(&yuv_frame);
    av_free(yuv_buf);
}

int64_t get_duration(Decoder *self)
{
    int64_t duration = self->_format_context->duration;
    int64_t seconds = duration * av_q2d(AV_TIME_BASE_Q);
    return seconds;
}

uint32_t get_fps(Decoder *self)
{
    double time_base = av_q2d(self->_codec_context->time_base) * self->_codec_context->ticks_per_frame;
    uint32_t fps = floor(1.0 / time_base);
    return fps;
}

uint32_t get_width(Decoder* self)
{
    return self->_codec_context->width;
}

uint32_t get_height(Decoder* self)
{
    return self->_codec_context->height;
}

void release_decoder(Decoder* self)
{
    av_frame_free(&self->_frame);
    avcodec_close(self->_codec_context);
    avformat_close_input(&self->_format_context);

    self->_codec = NULL;
    self->_codec_context = NULL;
    self->_frame = NULL;
    self->_stream = NULL;
    self->_format_context = NULL;

    self->decode_handler = NULL;
    self->_first_frame_index = -1;
}
