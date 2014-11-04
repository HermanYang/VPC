#ifndef VIDEOVIEW_H
#define VIDEOVIEW_H

#include <QLabel>
#include <QQueue>
#include <QImage>
#include <QMutex>

#include "define.h"
#include "PaintableView.h"

class VideoView : public PaintableView
{
    Q_OBJECT
public:
    explicit VideoView(QWidget* parent = 0);
    ~VideoView();

    void startPlay();
    void stopPlay();
    void setFrameQueue(QQueue<QImage>* queue);
signals:

public slots:

protected:
    void paintEvent(QPaintEvent* event);

private:
    QQueue<QImage>* _frame_queue;
    QImage _frame;
    bool _playing;
    uint32_t _fps;
    QMutex _mutex;
};

#endif // VIDEOVIEW_H
