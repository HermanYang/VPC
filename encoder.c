#include "encoder.h"
#include "utils.h"

void init_encoder(Encoder* self, const char* filename, uint32_t width, uint32_t height, uint32_t fps)
{
    av_register_all();

    self->_format_context = NULL;
    self->_codec_context = NULL; self->_out_format = NULL;
    self->_codec = NULL;
    self->_frame = NULL;
    self->_stream = NULL;

    // create format context
    avformat_alloc_output_context2(&self->_format_context, NULL, NULL, filename);

    // get output format
    self->_out_format = self->_format_context->oformat;

    // find encoder
    self->_codec = avcodec_find_encoder(self->_out_format->video_codec);

    if( !self->_format_context )
    {
        perror("Output format cannot be initialized");
    }

    if( !self->_codec )
    {
        perror("Codec not found");
        exit(EXIT_FAILURE);
    }

    //create stream
    self->_stream = avformat_new_stream(self->_format_context, self->_codec);

    self->_stream->id = self->_format_context->nb_streams;

    // get codec context
    self->_codec_context = self->_stream->codec;


    if( !self->_codec_context )
    {
        perror("Codec not allocate video codec context");
        exit(EXIT_FAILURE);
    }

    // set encode parameters

    self->_codec_context->bit_rate = 800 * 1024;

    self->_codec_context->width = width;
    self->_codec_context->height = height;

    self->_codec_context->time_base.num = 1;
    self->_codec_context->time_base.den = fps;

    self->_codec_context->gop_size = 12;

    self->_codec_context->pix_fmt = AV_PIX_FMT_YUV420P;

    self->_codec_context->flags |= CODEC_FLAG_GLOBAL_HEADER;

    if( av_opt_set(self->_codec_context->priv_data, "preset", "slow", 0) != 0 )
    {
        perror("opt set error");
        exit(EXIT_FAILURE);
    }

    // open codec
    avcodec_open2(self->_codec_context, self->_codec, NULL);

    // create frame
    self->_frame = av_frame_alloc();

    self->_frame->format = self->_codec_context->pix_fmt;
    self->_frame->width = self->_codec_context->width;
    self->_frame->height = self->_codec_context->height;
    self->_frame->pts = 0;

    av_image_alloc(self->_frame->data, self->_frame->linesize, self->_codec_context->width, self->_frame->height, self->_codec_context->pix_fmt, 32);

    if( !(self->_out_format->flags & AVFMT_NOFILE ) )
    {
        avio_open(&self->_format_context->pb, filename, AVIO_FLAG_WRITE);
    }

    // all initializatin job is done

    avformat_write_header(self->_format_context, NULL);

    av_dump_format(self->_format_context, 0, filename, 1);
}

void encode(Encoder* self, uint8_t* Y_data, uint8_t* U_data, uint8_t* V_data)
{
    int32_t ret;
    int32_t got_packet = 0;

    AVPacket avpkt;

    uint32_t width = self->_codec_context->width;
    uint32_t height = self->_codec_context->height;

    av_init_packet(&avpkt);

    avpkt.data = NULL;
    avpkt.size = 0;

    // copy data from bufffer to frame
    uint32_t length = width * height;

    memcpy(self->_frame->data[0], Y_data, length);

    length = length / 4;

    memcpy(self->_frame->data[1], U_data, length);

    memcpy(self->_frame->data[2], V_data, length);

    ret = avcodec_encode_video2(self->_codec_context, &avpkt, self->_frame, &got_packet);

    if( ret < 0 )
    {
        perror("Error encoding frame");
        exit(EXIT_FAILURE);
    }

    ++(self->_frame->pts);

    if( got_packet )
    {
        avpkt.pts = av_rescale_q_rnd(avpkt.pts, self->_codec_context->time_base, self->_stream->time_base, AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);
        avpkt.dts = av_rescale_q_rnd(avpkt.dts, self->_codec_context->time_base, self->_stream->time_base, AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);

        avpkt.duration = av_rescale_q(avpkt.duration, self->_codec_context->time_base, self->_stream->time_base);

        avpkt.stream_index = self->_stream->index;

        av_interleaved_write_frame(self->_format_context, &avpkt);
    }
}

static void _encode_delayed_frames(Encoder* self)
{
    int32_t got_packet = 0;
    int32_t ret;

    AVPacket avpkt;

    av_init_packet(&avpkt);

    do
    {
        avpkt.data = NULL;
        avpkt.size = 0;

        got_packet = 0;

        ret = avcodec_encode_video2(self->_codec_context, &avpkt, NULL, &got_packet);

        if( ret < 0 )
        {
            perror("Error encoding frame");
            exit(EXIT_FAILURE);
        }

        if(got_packet)
        {
            avpkt.pts = av_rescale_q_rnd(avpkt.pts, self->_codec_context->time_base, self->_stream->time_base, AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);
            avpkt.dts = av_rescale_q_rnd(avpkt.dts, self->_codec_context->time_base, self->_stream->time_base, AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);

            avpkt.duration = av_rescale_q(avpkt.duration, self->_codec_context->time_base, self->_stream->time_base);

            avpkt.stream_index = self->_stream->index;

            av_interleaved_write_frame(self->_format_context, &avpkt);

        }
    }
    while(got_packet);

}

bool is_init(Encoder *self)
{
    if(self->_format_context == NULL)
    {
        return false;
    }
    return true;
}

void release_encoder(Encoder* self)
{
    _encode_delayed_frames(self);

    av_write_trailer(self->_format_context);

    av_frame_free(&self->_frame);
    avcodec_close(self->_stream->codec);
    avio_close(self->_format_context->pb);
    avformat_free_context(self->_format_context);

    self->_format_context = NULL;
    self->_codec_context = NULL;
    self->_out_format = NULL;
    self->_codec = NULL;
    self->_frame = NULL;
    self->_stream = NULL;
}
