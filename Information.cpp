#include "Information.h"

Information::Information()
{
    _periods = NULL;
}

void Information::setVideoPath(string video_path)
{
    _video_path = string(video_path);
}

void Information::setPeriods(Period_Array* period)
{
    _periods = period;
}

void Information::setRecordRegion(QRect rect)
{
    _record_region = rect;
}

Period_Array* Information::getPeriods()
{
    return _periods;
}

string Information::getVideoPath()
{
    return _video_path;
}

QRect Information::getRecordRegion()
{
    return _record_region;
}

Information::~Information()
{
    _periods = NULL;
}
