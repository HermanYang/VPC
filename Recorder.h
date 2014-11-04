#ifndef CAMERA_H
#define CAMERA_H

#include "decoder.h"
#include "encoder.h"

#include "define.h"
#include "RecordThread.h"
#include "queue.h"
#include "detector.h"
#include "filter.h"

#include <QString>
#include <QImage>
#include <QWidget>
#include <QQueue>
#include <QMutex>
#include <QRect>

enum RecordType
{
    None,
    Normal,
    DynamicFrame,
    DynamicPeriod
};


class Recorder
{
public:
    Recorder();
    ~Recorder();

    void setRecordRegion(QRect rect);
    void recordStart(RecordType type);
    void recordFinished();
    void record(uint8_t* y_data, uint8_t* u_data, uint8_t* v_data, uint32_t width, uint32_t height, uint32_t index);
    void pushFrame(uint8_t *y_data, uint8_t* u_data, uint8_t* v_data, uint32_t width, uint32_t height);
    QImage recordAFrame();
    QQueue<QImage>* getFrameQueue();

private:
    bool isRegionValid();
    void recordFrame(uint8_t* y_data, uint8_t* u_data, uint8_t* v_data, uint32_t width, uint32_t height);
    void recordDynamicFrame(uint8_t* y_data, uint8_t* u_data, uint8_t* v_data, uint32_t width, uint32_t height, uint32_t index);
    void recordDynamicPeriod(uint8_t* y_data, uint8_t* u_data, uint8_t* v_data, uint32_t width, uint32_t height, uint32_t index);

private:
    Decoder _decoder;
    Encoder _encoder;
    Queue _yuv_queue;
    Filter _filter;
    Detector _mod;
    QQueue<QImage>* _queue;
    RecordThread* _thread;
    QMutex _mutex;
    uint8_t* _data;
    Region _record_region;
    RecordType _current_record_type;


public:
    static void handler(void* master, uint8_t* frame_y, uint8_t* frame_u, uint8_t* frame_v, uint32_t index, uint32_t width, uint32_t height);

};

#endif // CAMERA_H
