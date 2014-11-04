#include "RecordThread.h"

RecordThread::RecordThread(QObject *parent) :
    QThread(parent)
{
}

void RecordThread::setDecoder(Decoder *decoder)
{
    _decoder = decoder;
}

void RecordThread::run()
{
    if(_decoder != NULL)
    {
        decode(_decoder);
    }
}
