#ifndef CAMERATHREAD_H
#define CAMERATHREAD_H

#include "decoder.h"
#include <QThread>

class RecordThread : public QThread
{
    Q_OBJECT
public:
    explicit RecordThread(QObject *parent = 0);
    void setDecoder(Decoder* decoder);

signals:

protected:
    void run();

public slots:

private:
    Decoder* _decoder;

};

#endif // CAMERATHREAD_H
