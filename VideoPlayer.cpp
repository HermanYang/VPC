#include "VideoPlayer.h"
#include "decoder.h"

#include <string>
#include <sstream>
#include <Qt>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#ifdef Q_OS_LINUX
#include <stdlib.h>
#endif

using std::string;
using std::ostringstream;

VideoPlayer::VideoPlayer()
{
    _player_path = "vlc.exe";
}

VideoPlayer::VideoPlayer(const char* player_path)
{
    _player_path = string(player_path);
}

void VideoPlayer::playPeriod(const char *video_path, Period period)
{

    uint32_t fps = get_video_fps(video_path);

    float start_time = (float)period.begin_index / fps;
    float stop_time = (float)period.end_index / fps;

    ostringstream buf;

    buf << _player_path.c_str() << " " << video_path << " --start-time=" << start_time << " --stop-time=" << stop_time << " vlc://quit";

    string cmd = buf.str();

#ifdef Q_OS_WIN
    WinExec(cmd.c_str(),SW_SHOW);
#endif

#ifdef Q_OS_LINUX
    system(cmd.c_str());
#endif
}

uint32_t VideoPlayer::get_video_fps(const char *video_path)
{
    Decoder decoder = {0};
    init_decoder(&decoder, video_path, NULL);
    uint32_t fps = get_fps(&decoder);

    return fps;
}

void VideoPlayer::set_player_path(const char *player_path)
{
    _player_path = string(player_path);
}
