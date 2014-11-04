#include "DetectorThread.h"
#include "VideoDetector.h"

DetectorThread::DetectorThread()
{
    _video = NULL;
}

void DetectorThread::setVideo(Video *video)
{
    _video = video;
}


void DetectorThread::run()
{
    _video->decode();
    emit(detect_finished());
}
