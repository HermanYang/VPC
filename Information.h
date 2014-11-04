#ifndef INFORMATION_H
#define INFORMATION_H

#include <string>
#include <QRect>

#include "detector.h"

using std::string;

class Information
{
public:
    Information();
    ~Information();

    void setVideoPath(string file_path);
    string getVideoPath();

    void setRecordRegion(QRect rect);
    QRect getRecordRegion();

    Period_Array* getPeriods();
    void setPeriods(Period_Array* periods);

private:
    string _video_path;
    Period_Array* _periods;
    QRect _record_region;
};

#endif // INFORMATION_H
