#ifndef DETECTORTHREAD_H
#define DETECTORTHREAD_H

#include <QThread>
#include "Video.h"

class DetectorThread : public QThread
{
    Q_OBJECT
public:
    DetectorThread();

    void setVideo(Video* video);

signals:
    void detect_finished();

protected:
    void run();

private:
    Video* _video;
};

#endif // DETECTORTHREAD_H
