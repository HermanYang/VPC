#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSignalMapper>
#include <QLabel>
#include <QScrollArea>
#include <QGroupBox>
#include <QRadioButton>

#include "Recorder.h"
#include "utils.h"
#include "Video.h"
#include "Information.h"
#include "VideoDetector.h"
#include "VideoPlayer.h"
#include "LoadingDialog.h"
#include "NotePushableWidget.h"
#include "PaintableView.h"

enum State
{
    Idel,
    ReadyToDetect,
    BusyDetecting,
    ShowDetectionResult,
    SelectingtRegion,
    Recording
};


class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:

public slots:
    void load();
    void detect();
    void cancel();
    void playPeriod(int i);
    void prepareToShowResult();
    void recordStart();
    void recordEnd();
    void selectRegion();
    void handleQuit();

protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

private:
    void setCurrentState(State state);
    void stateChanged();

    QWidget* createModPanel();
    QWidget* createRecordPanel();

    void deleteMutables();

private:
    QHBoxLayout* _layout_main;
    QVBoxLayout* _layout_right;
    QVBoxLayout* _layout_left;
    QGridLayout* _layout_result;

    QWidget* _container;
    QLabel* _mod_info;

    QPushButton* _btn_load;
    QPushButton* _btn_detect;
    QPushButton* _btn_quit;
    QPushButton* _btn_select_region;
    QPushButton* _btn_record;
    QPushButton* _btn_stop;

    QRadioButton* _radio_btn_record_normal;
    QRadioButton* _radio_btn_record_dynamic_frame;
    QRadioButton* _radio_btn_record_dynamic_period;

    NotePushableWidget** _results;

    QWidget* _content_view;
    LoadingDialog* _loading_dialog;
    QScrollArea* _scroll_container;

    QSignalMapper* _signalMapper;

    Information* _info;
    State _current_state;

    Video* _video;
    VideoDetector* _detector;
    VideoPlayer* _player;
    Recorder* _recorder;

    QSize _button_size;
    QSize _display_pic_size;

    TimePeriod _time_period;
};

#endif // MAINWINDOW_H
