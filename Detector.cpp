#include "Detector.h"
#include "DetectorThread.h"

Detector* Detector::_instance = NULL;

Detector* Detector::getInstance()
{
    if(Detector::_instance == NULL)
    {
        Detector::_instance = new Detector();
    }
    return Detector::_instance;
}

Detector::Detector()
{
    _mod = {0};
    _region = {0};
    _detect_region = false;
    _thread = NULL;

    init_mod(&_mod);
}

void Detector::detectDefault(Video *video)
{
    _mod = {0};
    _region = {0};
    init_mod(&_mod);
    _detect_region = false;

    if (_thread != NULL)
    {
        delete _thread;
        _thread = NULL;
    }
    _thread = new DetectorThread;

    video->setCallback(Detector::detectionHandler, this);
    _thread->setVideo(video);
    _thread->start();
}

Period_Array* Detector::getPeriods()
{
    if(_thread->isFinished())
    {
        return get_dynamic_period_array(&_mod);
    }
    return NULL;
}

void Detector::detectRegion(Video *video, Region region)
{
    _mod = {0};
    _region = region;
    init_mod(&_mod);
    _detect_region = true;

    if (_thread != NULL)
    {
        delete _thread;
        _thread = NULL;
    }
    _thread = new DetectorThread;

    video->setCallback(Detector::detectionHandler, this);
    _thread->setVideo(video);
    _thread->start();
}

DetectorThread* Detector::getWorkingThread()
{
    return _thread;
}

void Detector::stop()
{
    if(_thread->isRunning())
    {
        _thread->terminate();
    }
}

void Detector::detectionHandler(void* master, uint8_t *frame_y, uint8_t *frame_u, uint8_t *frame_v, uint32_t index, uint32_t width, uint32_t height)
{
    Detector* detector = (Detector*)master;
    if(detector->_detect_region)
    {
        detect_region(&(detector->_mod), frame_y, index, width, height, detector->_region);
    }
    else
    {
        detect(&(detector->_mod), frame_y, index, width, height);
    }
}

Detector::~Detector()
{

    if( _thread != NULL )
    {
        delete _thread;
    }

    release_mod(&_mod);
}
