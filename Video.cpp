#include "Video.h"

using std::string;

Video::Video(const char* video_path)
{
    _video_path = string(video_path);
    _decoder = {0};
    init_decoder(&_decoder, video_path, NULL);
}

long Video::getDuration()
{
    return get_duration(&_decoder);
}

uint32_t Video::getFPS()
{
    return get_fps(&_decoder);
}

QImage Video::getFrameByIndex(uint32_t index)
{
    uint32_t width = get_width(&_decoder);
    uint32_t height = get_height(&_decoder);

    uint32_t size = width * height * 1.5;

    uint8_t* frame_yuv = new uint8_t[size];

    get_frame_by_index(&_decoder, index, frame_yuv, size);

    QImage image(width, height, QImage::Format_ARGB32);

    uint8_t* frame_y = frame_yuv;
    uint8_t* frame_u = frame_y + width * height;
    uint8_t* frame_v = frame_u + width * height / 4;

    for(uint32_t i = 0; i < height; ++i)
    {
        for(uint32_t j = 0; j < width; ++j)
        {
            int32_t y = frame_y[i * width + j];
            int32_t u = frame_u[i / 2 * width / 2 + j / 2];
            int32_t v = frame_v[i / 2 * width / 2 + j / 2];

            int32_t c = y - 16;
            int32_t d = u - 128;
            int32_t e = v - 128;

            int32_t r = (298 * c + 409 * e + 128) >> 8;
            int32_t g = (298 * c - 100 * d - 208 * e + 128) >> 8;
            int32_t b = (298 * c + 516 * d + 128) >> 8;

            r = r>255? 255 : r<0 ? 0 : r;
            g = g>255? 255 : g<0 ? 0 : g;
            b = b>255? 255 : b<0 ? 0 : b;

            image.setPixel(j, i,  0xff000000 | (r<<16) | (g << 8) | b);
        }
    }

    return image;
}

void Video::decode()
{
    ::decode(&_decoder);
}

uint32_t Video::getWidth()
{
    return get_width(&_decoder);
}

uint32_t Video::getHeight()
{
    return get_height(&_decoder);
}

void Video::setCallback(void (*decode_handler)(void* master, uint8_t* ptr_Y_data, uint8_t* ptr_U_data, uint8_t* ptr_V_data, uint32_t index, uint32_t width, uint32_t height), void* master)
{
    _decoder.decode_handler = decode_handler;
    _decoder.master = master;
}

string Video::getVideoPath()
{
    return _video_path;
}

Video::~Video()
{
    release_decoder(&_decoder);
}
