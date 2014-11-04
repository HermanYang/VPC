#include "Recorder.h"
#include "utils.h"
#include <QPainter>
#include <QMutex>
#include <Qt>

Recorder::Recorder()
{
    _decoder = {0};
    _encoder = {0};
    _mod = {0};
    _record_region = {0};

    _queue = NULL;
    _thread = NULL;
    _data = NULL;

    _queue = new QQueue<QImage>;
    _current_record_type = None;
}

QImage Recorder::recordAFrame()
{
#ifdef Q_OS_WIN
    init_decoder(&_decoder, "video=Chicony USB 2.0 Camera", "dshow");
#endif

#ifdef Q_OS_LINUX
    init_decoder(&_decoder, "/dev/video0", "video4linux2");
#endif

    init_filter(&_filter, "logo.yuv");

    _decoder.master = this;
    _decoder.decode_handler = Recorder::handler;

    _current_record_type = None;

    _queue->clear();

    if(_data != NULL)
    {
        delete[] _data;
        _data = NULL;
    }

    uint32_t width = get_width(&_decoder);
    uint32_t height = get_height(&_decoder);

    _data = new uint8_t[width * height * 3];

    if (_thread != NULL)
    {
        delete _thread;
        _thread = NULL;
    }

    _thread = new RecordThread;

    _thread->setDecoder(&_decoder);
    _thread->start();

    while(_queue->empty())
    {
        QThread::sleep(1);
    }

    if (_thread != NULL)
    {
        _thread->terminate();
        delete _thread;
        _thread = NULL;
    }

    release_decoder(&_decoder);
    release_filter(&_filter);

    return _queue->dequeue();
}

void Recorder::setRecordRegion(QRect rect)
{
    _record_region.left = rect.left();
    _record_region.top = rect.top();
    _record_region.right = rect.right();
    _record_region.bottom = rect.bottom();
}

void Recorder::recordFrame(uint8_t* y_data, uint8_t* u_data, uint8_t* v_data, uint32_t width, uint32_t height)
{
    filter_overlay(&_filter, 16, 16, width, height, y_data, u_data, v_data);
    encode(&_encoder, y_data, u_data, v_data);
}

void Recorder::recordDynamicPeriod(uint8_t* y_data, uint8_t* u_data, uint8_t* v_data, uint32_t width, uint32_t height, uint32_t index)
{
    char filename_buf[128];
    static uint32_t last_dynamic_index = 0;
    uint32_t length = width * height;

    uint8_t* ptr_Y_data = y_data;
    uint8_t* ptr_U_data = u_data;
    uint8_t* ptr_V_data = v_data;

    Region* region = NULL;

    if( isRegionValid() )
    {
        region = &_record_region;
    }

    if( is_dynamic_frame(&_mod, y_data, width, height, region) )
    {
        if(index > 0)
        {
            if(index - last_dynamic_index >= MAX_GAP)
            {
                //period start
                clear(&_yuv_queue);
                get_current_time_string(filename_buf, sizeof(filename_buf));
                strcat(filename_buf, ".mkv");
                init_encoder(&_encoder, filename_buf, width, height, get_fps(&_decoder));
            }

            last_dynamic_index = index;
        }
    }

    if( last_dynamic_index > 0 && index - last_dynamic_index < MAX_GAP )
    {
        if(queue_size(&_yuv_queue) == MIN_FRAME_SEQUENCE)
        {
            uint8_t* frame_data = (uint8_t*)dequeue(&_yuv_queue);

            ptr_Y_data = frame_data;
            ptr_U_data = frame_data + length;
            ptr_V_data = frame_data + length + length / 4;

            filter_overlay(&_filter, 16, 16, width, height, ptr_Y_data, ptr_U_data, ptr_V_data);

            encode(&_encoder, ptr_Y_data, ptr_U_data, ptr_V_data);

        }

        enqueue(&_yuv_queue, y_data);
    }
    else
    {
        // period end
        while( !queue_empty(&_yuv_queue) )
        {
            uint8_t* frame_data = (uint8_t*)dequeue(&_yuv_queue);

            ptr_Y_data = frame_data;
            ptr_U_data = frame_data + length;
            ptr_V_data = frame_data + length + length / 4;

            filter_overlay(&_filter, 16, 16, width, height, ptr_Y_data, ptr_U_data, ptr_V_data);

            encode(&_encoder, ptr_Y_data, ptr_U_data, ptr_V_data);


            if(queue_empty(&_yuv_queue))
            {
                release_encoder(&_encoder);
            }
        }

    }

}

void Recorder::recordDynamicFrame(uint8_t* y_data, uint8_t* u_data, uint8_t* v_data, uint32_t width, uint32_t height, uint32_t index)
{
    static uint32_t last_dynamic_index = 0;
    uint32_t length = width * height;

    uint8_t* ptr_Y_data = y_data;
    uint8_t* ptr_U_data = u_data;
    uint8_t* ptr_V_data = v_data;

    Region* region = NULL;

    if( isRegionValid() )
    {
        region = &_record_region;
    }

    if( is_dynamic_frame(&_mod, y_data, width, height, region) )
    {
        if(index > 0)
        {
            if(index - last_dynamic_index >= MAX_GAP)
            {
                //period start
                clear(&_yuv_queue);
            }

            last_dynamic_index = index;
        }
    }

    if( last_dynamic_index > 0 && index - last_dynamic_index < MAX_GAP )
    {

        if(queue_size(&_yuv_queue) == MIN_FRAME_SEQUENCE)
        {
            uint8_t* frame_data = (uint8_t*)dequeue(&_yuv_queue);

            ptr_Y_data = frame_data;
            ptr_U_data = frame_data + length;
            ptr_V_data = frame_data + length + length / 4;

            filter_overlay(&_filter, 16, 16, width, height, ptr_Y_data, ptr_U_data, ptr_V_data);

            encode(&_encoder, ptr_Y_data, ptr_U_data, ptr_V_data);

        }

        enqueue(&_yuv_queue, y_data);
    }
    else
    {
        // period end
        while( !queue_empty(&_yuv_queue) )
        {
            uint8_t* frame_data = (uint8_t*)dequeue(&_yuv_queue);

            ptr_Y_data = frame_data;
            ptr_U_data = frame_data + length;
            ptr_V_data = frame_data + length + length / 4;

            filter_overlay(&_filter, 16, 16, width, height, ptr_Y_data, ptr_U_data, ptr_V_data);

            encode(&_encoder, ptr_Y_data, ptr_U_data, ptr_V_data);

        }

    }
}

void Recorder::recordStart(RecordType type)
{
    char filename_buf[128];

    _current_record_type = type;

#ifdef Q_OS_WIN
    init_decoder(&_decoder, "video=Chicony USB 2.0 Camera", "dshow");
#endif

#ifdef Q_OS_LINUX
    init_decoder(&_decoder, "/dev/video0", "video4linux2");
#endif

    _decoder.master = this;
    _decoder.decode_handler = Recorder::handler;
    _queue->clear();

    if(_data != NULL)
    {
        delete[] _data;
        _data = NULL;
    }

    uint32_t width = get_width(&_decoder);
    uint32_t height = get_height(&_decoder);
    uint32_t fps = get_fps(&_decoder);

    _data = new uint8_t[width * height * 3];

    init_queue(&_yuv_queue, width * height * 1.5 );
    init_filter(&_filter, "logo.yuv");
    init_detector(&_mod);

    get_current_time_string(filename_buf, sizeof(filename_buf));

    strcat(filename_buf, ".mkv");


    switch (type)
    {
    case Normal:
    {
        init_encoder(&_encoder, filename_buf, width, height, fps);
        break;
    }

    case DynamicFrame:
    {
        init_encoder(&_encoder, filename_buf, width, height, fps);
        break;
    }

    case DynamicPeriod:
    {
        break;
    }

    default:
        break;
    }

    if (_thread != NULL)
    {
        delete _thread;
        _thread = NULL;
    }

    _thread = new RecordThread;

    _thread->setDecoder(&_decoder);
    _thread->start();
}

void Recorder::recordFinished()
{
    if (_thread != NULL)
    {
        _thread->terminate();
        delete _thread;
        _thread = NULL;
    }

    if(_data != NULL)
    {
        delete[] _data;
        _data = NULL;
    }

    uint32_t width = get_width(&_decoder);
    uint32_t height = get_height(&_decoder);
    uint32_t length = width * height;

    uint8_t* ptr_Y_data = NULL;
    uint8_t* ptr_U_data = NULL;
    uint8_t* ptr_V_data = NULL;

    while( !queue_empty(&_yuv_queue) )
    {
        uint8_t* frame_data = (uint8_t*)dequeue(&_yuv_queue);

        ptr_Y_data = frame_data;
        ptr_U_data = frame_data + length;
        ptr_V_data = frame_data + length + length / 4;

        filter_overlay(&_filter, 16, 16, width, height, ptr_Y_data, ptr_U_data, ptr_V_data);

        encode(&_encoder, ptr_Y_data, ptr_U_data, ptr_V_data);
    }

    switch (_current_record_type)
    {

    case Normal:
    {
        release_encoder(&_encoder);
        break;
    }

    case DynamicFrame:
    {
        release_encoder(&_encoder);
        break;
    }

    case DynamicPeriod:
    {
        if( is_init(&_encoder) )
        {
            release_encoder(&_encoder);
        }
        break;
    }

    default:
        break;
    }

    _current_record_type = None;

    release_decoder(&_decoder);
    release_detector(&_mod);
    release_queue(&_yuv_queue);
    release_filter(&_filter);

}


QQueue<QImage>* Recorder::getFrameQueue()
{
    return _queue;
}

void Recorder::pushFrame(uint8_t *y_data, uint8_t* u_data, uint8_t* v_data, uint32_t width, uint32_t height)
{
    filter_overlay(&_filter, 16, 16, width, height, y_data, u_data, v_data);

    uint32_t index = 0;

    for(uint32_t i = 0; i < height; ++i)
    {
        for(uint32_t j = 0; j < width; ++j)
        {
            int32_t y = y_data[i * width + j];
            int32_t u = u_data[i / 2 * width / 2 + j / 2];
            int32_t v = v_data[i / 2 * width / 2 + j / 2];

            int32_t c = y - 16;
            int32_t d = u - 128;
            int32_t e = v - 128;

            int32_t r = (298 * c + 409 * e + 128) >> 8;
            int32_t g = (298 * c - 100 * d - 208 * e + 128) >> 8;
            int32_t b = (298 * c + 516 * d + 128) >> 8;

            r = r>255? 255 : r<0 ? 0 : r;
            g = g>255? 255 : g<0 ? 0 : g;
            b = b>255? 255 : b<0 ? 0 : b;

            _data[index++] = (uchar)r;
            _data[index++] = (uchar)g;
            _data[index++] = (uchar)b;
        }
    }

    _mutex.lock();
    _queue->enqueue(QImage(_data, width, height, QImage::Format_RGB888));
    _mutex.unlock();
}

void Recorder::record(uint8_t *y_data, uint8_t *u_data, uint8_t *v_data, uint32_t width, uint32_t height, uint32_t index)
{
    switch (_current_record_type) {
    case Normal:
    {
        recordFrame(y_data, u_data, v_data, width, height);
        break;
    }

    case DynamicFrame:
    {
        recordDynamicFrame(y_data, u_data, v_data, width, height, index);
        break;
    }
    case DynamicPeriod:
    {
        recordDynamicPeriod(y_data, u_data, v_data, width, height, index);
        break;
    }

    default:
        break;
    }
}

void Recorder::handler(void* master, uint8_t* frame_y, uint8_t* frame_u, uint8_t* frame_v, uint32_t index, uint32_t width, uint32_t height)
{
    Recorder* recorder = (Recorder*)master;
    recorder->pushFrame(frame_y, frame_u, frame_v, width, height);
    recorder->record(frame_y, frame_u, frame_v, width, height, index);
}

bool Recorder::isRegionValid()
{
    if( _record_region.right - _record_region.left == 0 || _record_region.bottom - _record_region.top == 0)
    {
        return false;
    }
    return true;
}

Recorder::~Recorder()
{
    release_decoder(&_decoder);
    release_encoder(&_encoder);

    if( _queue != NULL)
    {
        delete _queue;
        _queue = NULL;
    }

    if( _thread != NULL )
    {
        _thread->terminate();
        delete _thread;
        _thread = NULL;
    }

    if( _data != NULL)
    {
        delete[] _data;
    }
}
