#include "VideoDetector.h"
#include "DetectorThread.h"

VideoDetector* VideoDetector::_instance = NULL;

VideoDetector* VideoDetector::getInstance()
{
    if(VideoDetector::_instance == NULL)
    {
        VideoDetector::_instance = new VideoDetector();
    }
    return VideoDetector::_instance;
}

VideoDetector::VideoDetector()
{
    _mod = {0};
    _region = {0};
    _detect_region = false;
    _thread = NULL;

    init_detector(&_mod);
}

void VideoDetector::detectDefault(Video *video)
{
    _mod = {0};
    _region = {0};
    init_detector(&_mod);
    _detect_region = false;

    if (_thread != NULL)
    {
        delete _thread;
        _thread = NULL;
    }
    _thread = new DetectorThread;

    video->setCallback(VideoDetector::detectionHandler, this);
    _thread->setVideo(video);
    _thread->start();
}

Period_Array* VideoDetector::getPeriods()
{
    if(_thread->isFinished())
    {
        return get_dynamic_period_array(&_mod);
    }
    return NULL;
}

void VideoDetector::detectRegion(Video *video, Region region)
{
    _mod = {0};
    _region = region;
    init_detector(&_mod);
    _detect_region = true;

    if (_thread != NULL)
    {
        delete _thread;
        _thread = NULL;
    }
    _thread = new DetectorThread;

    video->setCallback(VideoDetector::detectionHandler, this);
    _thread->setVideo(video);
    _thread->start();
}

DetectorThread* VideoDetector::getWorkingThread()
{
    return _thread;
}

void VideoDetector::stop()
{
    if(_thread->isRunning())
    {
        _thread->terminate();
    }
}

void VideoDetector::detectionHandler(void* master, uint8_t *frame_y, uint8_t *frame_u, uint8_t *frame_v, uint32_t index, uint32_t width, uint32_t height)
{
    VideoDetector* detector = (VideoDetector*)master;
    if(detector->_detect_region)
    {
        detect_region(&(detector->_mod), frame_y, index, width, height, detector->_region);
    }
    else
    {
        detect(&(detector->_mod), frame_y, index, width, height);
    }
}

VideoDetector::~VideoDetector()
{

    if( _thread != NULL )
    {
        delete _thread;
    }

    release_detector(&_mod);
}
