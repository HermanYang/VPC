HEADERS += decoder.h \
    utils.h \
    MainWindow.h \
    Information.h \
    Video.h \
    VideoPlayer.h \
    DetectorThread.h \
    LoadingDialog.h \
    NotePushableWidget.h \
    encoder.h \
    define.h \
    queue.h \
    Recorder.h \
    RecordThread.h \
    VideoView.h \
    PaintableView.h \
    filter.h \
    detector.h \
    VideoDetector.h

SOURCES += main.cpp \
    decoder.c \
    utils.c \
    Information.cpp \
    Video.cpp \
    VideoPlayer.cpp \
    MainWindow.cpp \
    DetectorThread.cpp \
    LoadingDialog.cpp \
    NotePushableWidget.cpp \
    encoder.c \
    queue.c \
    Recorder.cpp \
    RecordThread.cpp \
    VideoView.cpp \
    PaintableView.cpp \
    filter.c \
    detector.c \
    VideoDetector.cpp
    filter.c

CONFIG += qt
CONFIG += debug
CONFIG += release
CONFIG += build_all

QT += widgets

win32{
    INCLUDEPATH += $$PWD/ffmpeg/win/32-bit/include
    LIBS += -L$$PWD/ffmpeg/win/32-bit/lib -lavcodec -lavformat -lavutil -lavdevice -lswscale
    LIBS += -L$$PWD/ffmpeg/win/32-bit/bin
    LIBS += -L$$PWD/vlc
}

unix:!macx{
    INCLUDEPATH += $$PWD/ffmpeg/linux/64-bit/include
    LIBS += -L$$PWD/ffmpeg/linux/64-bit/lib -lavcodec -lavformat -lavdevice -lavfilter -lpostproc -lavformat -lavcodec -lx264 -lbz2 -lz -lrt -lswresample -lswscale -lavutil -lm
    LIBS += -L$$PWD/vlc
}

RESOURCES += res.qrc
RESOURCES += style.qrc

RC_FILE += vpc.rc
