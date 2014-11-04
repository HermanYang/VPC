#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif

#include "detector.h"
#include "string"

using std::string;

class VideoPlayer
{
public:
    VideoPlayer();
    VideoPlayer(const char* player_path);
    void playPeriod(const char* video_path, Period period);
    void set_player_path(const char* player_path);

private:
    uint32_t get_video_fps(const char* video_path);
    string _player_path;

};

#endif // VIDEOPLAYER_H
