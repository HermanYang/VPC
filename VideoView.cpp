#include "VideoView.h"
#include <QPainter>
#include <QTimer>
#include <QMutex>

VideoView::VideoView(QWidget *parent) :
    PaintableView(parent)
{
    setFixedSize(800, 600);

    _frame_queue = NULL;
    _playing = false;
    _frame = QImage(":/images/idle_bg.png");
    _fps = 30;
}

void VideoView::startPlay()
{
    _playing = true;
    repaint();
}

void VideoView::stopPlay()
{
    _playing = false;
}

void VideoView::setFrameQueue(QQueue<QImage> *queue)
{
    _frame_queue = queue;
}

void VideoView::paintEvent(QPaintEvent *event)
{
    PaintableView::paintEvent(event);

    if( _frame_queue != NULL )
    {
        if( _playing )
        {
            if( !_frame_queue->isEmpty())
            {
                _mutex.lock();
                _frame = _frame_queue->dequeue();
                _mutex.unlock();

                setPixmap(QPixmap::fromImage(_frame));
            }

            const uint32_t microsecond_per_second = 1000;
            uint32_t interval = microsecond_per_second / _fps;

            QTimer::singleShot(interval, this, SLOT(repaint()));
        }
    }
}

VideoView::~VideoView()
{
}
