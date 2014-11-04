#ifndef VIDEO_H
#define VIDEO_H

#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif

#include "decoder.h"
#include <QImage>
#include <string>
using std::string;

class Video
{
public:
    Video(const char* video_path);
    ~Video();
    Decoder* getDecoder();
    uint32_t getWidth();
    uint32_t getHeight();
    void setCallback(void (*decode_handler)(void* master, uint8_t* ptr_Y_data, uint8_t* ptr_U_data, uint8_t* ptr_V_data, uint32_t index, uint32_t width, uint32_t height), void* master);
    QImage getFrameByIndex(uint32_t index);
    string getVideoPath();
    uint32_t getFPS();
    long getDuration();
    string getVideoLength();
    void decode();

private:
    Decoder _decoder;
    string _video_path;
};

#endif // VIDEO_H
