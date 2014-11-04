#ifndef DETECTOR_H
#define DETECTOR_H

#include "DetectorThread.h"

#include "detector.h"
#include "Video.h"

class VideoDetector
{
public:
    VideoDetector();
    ~VideoDetector();

    Period_Array* getPeriods();
    void detectRegion(Video* video, Region region);
    void detectDefault(Video* video);
    void stop();

public:
    Detector _mod;
    Region _region;
    bool _detect_region;
    DetectorThread* getWorkingThread();

public:
    static VideoDetector* getInstance();

public:
    static void detectionHandler(void* master, uint8_t* frame_y, uint8_t* frame_u, uint8_t* frame_v, uint32_t index, uint32_t width, uint32_t height);

private:
    DetectorThread* _thread;
    Period_Array* _periods;

private:
    static VideoDetector* _instance;

};

#endif // DETECTOR_H
