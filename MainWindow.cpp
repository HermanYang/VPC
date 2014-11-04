#include "MainWindow.h"
#include "Video.h"
#include "QApplication"
#include "PaintableView.h"
#include "VideoView.h"

#include <QFileDialog>
#include <QPixmap>
#include <QPalette>
#include <QPainter>
#include <QMouseEvent>
#include <QTextFormat>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    _signalMapper = NULL;
    _video = NULL;
    _recorder = NULL;

    _results = NULL;
    _content_view = NULL;
    _scroll_container = NULL;

    _layout_result = NULL;

    _recorder = new Recorder;

    _display_pic_size = QSize(256, 192);

    _info = new Information();

    _detector = VideoDetector::getInstance();
    _player = new VideoPlayer();

    _loading_dialog = new LoadingDialog(this);
    _loading_dialog->setModal(true);

    // initialize widgets
    _container = new QWidget();

    _btn_quit = new QPushButton("Quit");

    // layouts
    _layout_main = new QHBoxLayout();
    _layout_right = new QVBoxLayout();
    _layout_left = new QVBoxLayout();

    _layout_main->addLayout(_layout_left);
    _layout_main->addLayout(_layout_right);
    _layout_main->setSpacing(10);

    _layout_left->setSpacing(10);

    _layout_right->addWidget(createModPanel());
    _layout_right->addStretch(1);
    _layout_right->addWidget(createRecordPanel());
    _layout_right->addStretch(5);
    _layout_right->addWidget(_btn_quit);
    _layout_right->setSpacing(10);

    _container->setLayout(_layout_main);

    // connections
    QObject::connect(_btn_load, SIGNAL(clicked()), this, SLOT(load()));
    QObject::connect(_btn_detect, SIGNAL(clicked()), this, SLOT(detect()));
    QObject::connect(_loading_dialog, SIGNAL(cancel()), this, SLOT(cancel()));
    QObject::connect(_btn_select_region, SIGNAL(clicked()), this, SLOT(selectRegion()));
    QObject::connect(_btn_record, SIGNAL(clicked()), this, SLOT(recordStart()));
    QObject::connect(_btn_stop, SIGNAL(clicked()), this, SLOT(recordEnd()));
    QObject::connect(_btn_quit, SIGNAL(clicked()), this, SLOT(handleQuit()));
    QObject::connect(_radio_btn_record_normal, SIGNAL(toggled(bool)), _btn_select_region, SLOT(setDisabled(bool)));
    QObject::connect(_radio_btn_record_dynamic_frame, SIGNAL(toggled(bool)), _btn_select_region, SLOT(setEnabled(bool)));
    QObject::connect(_radio_btn_record_dynamic_period, SIGNAL(toggled(bool)), _btn_select_region, SLOT(setEnabled(bool)));

    // set main window
    setWindowIcon(QIcon(":images/logo.png"));
    setWindowTitle("Moving Object Detector");
    setCentralWidget(_container);
    setFixedSize(950, 620);

    setCurrentState(Idel);
}

void MainWindow::handleQuit()
{
    switch ( _current_state )
    {

    case Recording:
    {
        recordEnd();
        close();
        break;
    }

    case Idel:
    case SelectingtRegion:
    case ReadyToDetect:
    case BusyDetecting:
    case ShowDetectionResult:
    default:
        close();
        break;
    }
}

void MainWindow::load()
{
    using std::string;
    QString file_name = QFileDialog::getOpenFileName(this, tr("Open Video"), "." );
    if ( !file_name.isEmpty() )
    {
        _info->setVideoPath(string(QDir::toNativeSeparators(file_name).toUtf8().constData()));
        setCurrentState(ReadyToDetect);
    }
}

void MainWindow::detect()
{
    PaintableView* paintable_view =  dynamic_cast<PaintableView*>(_content_view);

    if( paintable_view )
    {
        QRect rect = paintable_view->getLogicalRect();
        if( rect.isValid() )
        {
            Region region;
            region.left = rect.left();
            region.top = rect.top();
            region.right = rect.right();
            region.bottom = rect.bottom();

            _detector->detectRegion(_video, region);
        }
        else
        {
            _detector->detectDefault(_video);
        }
    }


    time_record_start(&_time_period);
    setCurrentState(BusyDetecting);
}

void MainWindow::prepareToShowResult()
{
    VideoDetector* detector = VideoDetector::getInstance();
    _info->setPeriods(detector->getPeriods());
    QObject::disconnect(detector->getWorkingThread(), SIGNAL(detect_finished()), this, SLOT(prepareToShowResult()));

    setCurrentState(ShowDetectionResult);
}

void MainWindow::cancel()
{
    VideoDetector::getInstance()->stop();
    setCurrentState(ReadyToDetect);
}

void MainWindow::recordStart()
{
    PaintableView* paintable_view =  dynamic_cast<PaintableView*>(_content_view);

    if( paintable_view )
    {
        QRect rect = paintable_view->getLogicalRect();
        if( rect.isValid() )
        {
            _info->setRecordRegion(paintable_view->getPaintingRect());
            _recorder->setRecordRegion( rect );
        }
    }

    if( _radio_btn_record_normal->isChecked() )
    {
        _recorder->recordStart(Normal);
    }
    else if( _radio_btn_record_dynamic_frame->isChecked() )
    {
        _recorder->recordStart(DynamicFrame);
    }
    else
    {
        _recorder->recordStart(DynamicPeriod);
    }

    _content_view->repaint();

    setCurrentState(Recording);
}

void MainWindow::recordEnd()
{
    _recorder->recordFinished();
    setCurrentState(Idel);
}

void MainWindow::playPeriod(int i)
{
    Period_Array* periods = _info->getPeriods();
    _player->playPeriod(_video->getVideoPath().c_str(), periods->ptr_array[i]);
}

void MainWindow::selectRegion()
{
    QPixmap pixmap = QPixmap::fromImage( _recorder->recordAFrame() );
    PaintableView* paintable_view =  dynamic_cast<PaintableView*>(_content_view);
    if( paintable_view)
    {
        paintable_view->setPixmap(pixmap);
    }

    setCurrentState(SelectingtRegion);
}

void MainWindow::stateChanged()
{
    switch (_current_state)
    {
    case Idel:
    {
        deleteMutables();

        _btn_detect->setDisabled(true);
        _btn_load->setEnabled(true);
        _btn_select_region->setEnabled(true);
        _btn_record->setEnabled(true);
        _btn_stop->setDisabled(true);
        _btn_quit->setEnabled(true);

        _radio_btn_record_normal->setEnabled(true);
        _radio_btn_record_dynamic_frame->setEnabled(true);
        _radio_btn_record_dynamic_period->setEnabled(true);

        _mod_info->clear();

        if( _radio_btn_record_normal->isChecked() )
        {
            _btn_select_region->setDisabled(true);
        }

        if(_loading_dialog->isHidden())
        {
            _loading_dialog->hide();
        }

        _content_view = new PaintableView;

        ((PaintableView*)_content_view)->setPaintable(false);
        _layout_left->addWidget(_content_view, Qt::AlignCenter);

        QPixmap pixmap(":images/idle_bg.png");
        ((PaintableView*)_content_view)->setPixmap(pixmap);

        break;
    }

    case SelectingtRegion:
    {
        _btn_detect->setDisabled(true);
        _btn_load->setEnabled(true);
        _btn_select_region->setDisabled(true);
        _btn_record->setEnabled(true);
        _btn_stop->setDisabled(true);
        _btn_quit->setEnabled(true);

        _radio_btn_record_normal->setEnabled(true);
        _radio_btn_record_dynamic_frame->setEnabled(true);
        _radio_btn_record_dynamic_period->setEnabled(true);

        _mod_info->clear();

        if( _radio_btn_record_normal->isChecked() )
        {
            _btn_select_region->setDisabled(true);
        }

        if(_loading_dialog->isHidden())
        {
            _loading_dialog->hide();
        }

        PaintableView* paintable_view =  dynamic_cast<PaintableView*>(_content_view);
        paintable_view->setPaintable(true);

        break;
    }

    case Recording:
    {
        deleteMutables();

        _btn_detect->setDisabled(true);
        _btn_load->setDisabled(true);
        _btn_select_region->setDisabled(true);
        _btn_record->setDisabled(true);
        _btn_stop->setEnabled(true);
        _btn_quit->setEnabled(true);

        _radio_btn_record_normal->setDisabled(true);
        _radio_btn_record_dynamic_frame->setDisabled(true);
        _radio_btn_record_dynamic_period->setDisabled(true);

        if(_loading_dialog->isVisible())
        {
            _loading_dialog->hide();
        }

        _mod_info->clear();

        _content_view = new VideoView();

        PaintableView* paintable_view = dynamic_cast<PaintableView*>(_content_view);

        if(paintable_view)
        {
            QRect record_region = _info->getRecordRegion();
            if( record_region.isValid() )
            {
                paintable_view->setPaintable(true);
                paintable_view->setPaintingRect(record_region);
                _info->setRecordRegion(QRect());
            }
            else
            {
                paintable_view->setPaintable(false);
            }

        }

        VideoView* video_view = dynamic_cast<VideoView*>(_content_view);

        if( video_view )
        {
            video_view->setFrameQueue(_recorder->getFrameQueue());
            video_view->startPlay();
        }

        _layout_left->addWidget(_content_view, Qt::AlignCenter);

        break;
    }

    case ReadyToDetect:
    {
        deleteMutables();

        // ui changed:
        _btn_detect->setEnabled(true);
        _btn_load->setEnabled(true);
        _btn_select_region->setEnabled(true);
        _btn_record->setEnabled(true);
        _btn_stop->setDisabled(true);
        _btn_quit->setEnabled(true);

        _radio_btn_record_normal->setEnabled(true);
        _radio_btn_record_dynamic_frame->setEnabled(true);
        _radio_btn_record_dynamic_period->setEnabled(true);

        if(_loading_dialog->isVisible())
        {
            _loading_dialog->hide();
        }

        if( _radio_btn_record_normal->isChecked() )
        {
            _btn_select_region->setDisabled(true);
        }

        // initilze video
        if( _video != NULL )
        {
            delete _video;
            _video = NULL;
        }

        _video = new Video(_info->getVideoPath().c_str());

        _content_view = new PaintableView();

        ((PaintableView*)_content_view)->setPaintable(true);
        ((PaintableView*)_content_view)->setPixmap(QPixmap::fromImage(_video->getFrameByIndex(0)));

        _layout_left->addWidget(_content_view, Qt::AlignCenter);

        const uint32_t second_per_day = 3600 * 24;
        const uint32_t second_per_hour = 3600;
        const uint32_t second_per_min = 60;

        uint32_t fps = _video->getFPS();
        long duration = _video->getDuration();
        uint32_t day = duration / second_per_day;
        uint32_t hour = (duration - day * second_per_day) / second_per_hour;
        uint32_t min = (duration - day * second_per_day - hour * second_per_hour) / second_per_min;
        uint32_t second = duration - day * second_per_day - hour * second_per_hour - min * second_per_min;
        QString video_info = "FPS:" + QString::number(fps) +"\nVideo Duration:\n Day:" + QString::number(day) + "\n Hour:" + QString::number(hour) + "\n Min:" + QString::number(min) + "\n Second:" + QString::number(second);

        _mod_info->setText(video_info);

        break;
    }

    case BusyDetecting:
    {
        // ui changed:
        _btn_detect->setDisabled(true);
        _btn_load->setDisabled(true);
        _btn_record->setDisabled(true);
        _btn_select_region->setDisabled(true);
        _btn_stop->setDisabled(true);
        _btn_quit->setEnabled(true);

        _radio_btn_record_normal->setDisabled(true);
        _radio_btn_record_dynamic_frame->setDisabled(true);
        _radio_btn_record_dynamic_period->setDisabled(true);

        if(_loading_dialog->isHidden())
        {
            _loading_dialog->show();
        }

        if( _radio_btn_record_normal->isChecked() )
        {
            _btn_select_region->setDisabled(true);
        }


        VideoDetector* detector = VideoDetector::getInstance();
        DetectorThread* thread = detector->getWorkingThread();

        QObject::connect(thread, SIGNAL(detect_finished()), this, SLOT(prepareToShowResult()));

        break;
    }

    case ShowDetectionResult:
    {
        deleteMutables();

        // ui changed:
        _btn_detect->setDisabled(true);
        _btn_load->setEnabled(true);
        _btn_select_region->setEnabled(true);
        _btn_record->setEnabled(true);
        _btn_stop->setDisabled(true);
        _btn_quit->setEnabled(true);

        _radio_btn_record_normal->setEnabled(true);
        _radio_btn_record_dynamic_frame->setEnabled(true);
        _radio_btn_record_dynamic_period->setEnabled(true);

        if(_loading_dialog->isVisible())
        {
            _loading_dialog->hide();
        }

        if( _radio_btn_record_normal->isChecked() )
        {
            _btn_select_region->setDisabled(true);
        }


        Period_Array* periods = _info->getPeriods();

        // add results
        _signalMapper = new QSignalMapper;
        _layout_result = new QGridLayout;
        _scroll_container = new QScrollArea;
        _content_view = new QWidget;

        _layout_left->addWidget(_scroll_container, Qt::AlignCenter);


        NotePushableWidget* period_btn = NULL;
        _results = new NotePushableWidget*[periods->size];

        const uint32_t second_per_day = 3600 * 24;
        const uint32_t second_per_hour = 3600;
        const uint32_t second_per_min = 60;

        const uint32_t max_column = 3;

        long dynamic_duration = 0;

        uint32_t frame_index = 0;


        for(uint32_t i = 0; i < periods->size; ++i)
        {
            period_btn = new NotePushableWidget;
            period_btn->setFixedSize(_display_pic_size);

            frame_index = (periods->ptr_array[i].begin_index + periods->ptr_array[i].end_index) / 2;

            QPixmap pixmap = QPixmap::fromImage(_video->getFrameByIndex(frame_index));
            period_btn->setPixmap(pixmap, _display_pic_size);

            uint32_t fps = _video->getFPS();
            long duration = periods->ptr_array[i].begin_index / fps;
            uint32_t day = duration / second_per_day;
            uint32_t hour = (duration - day * second_per_day) / second_per_hour;
            uint32_t min = (duration - day * second_per_day - hour * second_per_hour) / second_per_min;
            uint32_t second = duration - day * second_per_day - hour * second_per_hour - min * second_per_min;


            QString time_info = "Start Time " + QString::number(day) + ":" + QString::number(hour) + ":" + QString::number(min) + ":" + QString::number(second);

            period_btn->setNote(time_info);

            _layout_result->addWidget(period_btn, i / max_column, i % max_column, Qt::AlignLeft | Qt::AlignTop);


            QObject::connect(period_btn, SIGNAL(clicked()), _signalMapper, SLOT(map()));
            _signalMapper->setMapping(period_btn, i);

            _results[i] = period_btn;

            dynamic_duration += (periods->ptr_array[i].end_index - periods->ptr_array[i].begin_index) / fps;
        }

        _content_view->setLayout(_layout_result);
        _scroll_container->setWidget(_content_view);
        _scroll_container->setFixedSize(800, 600);

        double elapse = time_record_end(&_time_period);
        QString result_info = "ELapse:\n " + QString::number(elapse) + "s" +"\nDynamic Duration:\n " + QString::number(dynamic_duration) + "s" + "\nPeriod Amount:\n " + QString::number(periods->size);

        _mod_info->setText(result_info);

        connect(_signalMapper, SIGNAL(mapped(int)), this, SLOT(playPeriod(int)));

        break;
    }

    default:
        break;
    }
}

void MainWindow::setCurrentState(State state)
{
    _current_state = state;
    stateChanged();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if( _current_state == ReadyToDetect || _current_state == SelectingtRegion )
    {
        uint32_t right = _content_view->geometry().right();
        uint32_t bottom = _content_view->geometry().bottom();

        if(event->x() < right && event->y() < bottom && event->button() == Qt::LeftButton)
        {
            uint32_t left = _content_view->geometry().left();
            uint32_t top = _content_view->geometry().top();

            ((PaintableView*)_content_view)->setPaintingRectLeftTop(event->x() - left, event->y() - top);
        }

        if(event->button() == Qt::RightButton)
        {
            _content_view->repaint();
        }
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if( _current_state == ReadyToDetect || _current_state == SelectingtRegion)
    {

        uint32_t right = _content_view->geometry().right();
        uint32_t bottom = _content_view->geometry().bottom();

        if( event->x() < right && event->y() < bottom )
        {
            uint32_t left = _content_view->geometry().left();
            uint32_t top = _content_view->geometry().top();

            ((PaintableView*)_content_view)->setPaintingRectRightBottom(event->x() - left, event->y() - top);
            _content_view->repaint();
        }
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if( _current_state == ReadyToDetect || _current_state == SelectingtRegion)
    {
        uint32_t right = _content_view->geometry().right();
        uint32_t bottom = _content_view->geometry().bottom();

        if(event->x() < right && event->y() < bottom && event->button() == Qt::LeftButton)
        {
            uint32_t left = _content_view->geometry().left();
            uint32_t top = _content_view->geometry().top();

            ((PaintableView*)_content_view)->setPaintingRectRightBottom(event->x() - left, event->y() - top);
            _content_view->repaint();

        }
    }
}

QWidget* MainWindow::createModPanel()
{
    _btn_load = new QPushButton("Load Video");

    _btn_detect = new QPushButton("Detect");

    QGroupBox* mod_group = new QGroupBox("Detection");

    QVBoxLayout* layout = new QVBoxLayout;

    _mod_info= new QLabel();

    layout->addWidget(_mod_info);
    layout->addWidget(_btn_load);
    layout->addWidget(_btn_detect);

    layout->setSpacing(10);

    mod_group->setLayout(layout);
    mod_group->setFixedWidth(125);

    return mod_group;
}

QWidget* MainWindow::createRecordPanel()
{
    _radio_btn_record_normal = new QRadioButton("Normal");
    _radio_btn_record_dynamic_frame = new QRadioButton("Dynamic Frames");
    _radio_btn_record_dynamic_period = new QRadioButton("Dynamic Periods");

    _btn_select_region = new QPushButton("Select Region");
    _btn_record = new QPushButton("Record");
    _btn_stop = new QPushButton("Stop");

    _radio_btn_record_normal->toggle();
    _btn_select_region->setDisabled(true);

    QGroupBox* record_group = new QGroupBox("Record");

    QVBoxLayout* layout = new QVBoxLayout;

    layout->addWidget(_radio_btn_record_normal);
    layout->addWidget(_radio_btn_record_dynamic_frame);
    layout->addWidget(_radio_btn_record_dynamic_period);

    layout->addWidget(_btn_select_region);
    layout->addWidget(_btn_record);
    layout->addWidget(_btn_stop);
    layout->setSpacing(10);

    record_group->setLayout(layout);
    record_group->setFixedWidth(125);

    return record_group;
}


void MainWindow::deleteMutables()
{
    if( _results != NULL )
    {
        delete[] _results;
        _results = NULL;
    }

    if(_content_view != NULL)
    {
        delete _content_view;
        _content_view = NULL;
    }

    if(_scroll_container != NULL)
    {
        delete _scroll_container;
        _scroll_container = NULL;
    }

    if( _signalMapper != NULL)
    {
        delete _signalMapper;
        _signalMapper = NULL;
    }
}

MainWindow::~MainWindow()
{
    _layout_main = NULL;
    _layout_left = NULL;
    _layout_right = NULL;

    _layout_result = NULL;

    if(_container != NULL)
    {
        delete _container;
        _container = NULL;
    }

    if(_mod_info != NULL)
    {
        delete _mod_info;
        _mod_info = NULL;
    }

    if(_btn_load != NULL)
    {
        delete _btn_load;
        _btn_load = NULL;
    }

    if(_btn_detect != NULL)
    {
        delete _btn_detect;
        _btn_detect = NULL;
    }

    if(_btn_quit != NULL)
    {
        delete _btn_quit;
        _btn_quit = NULL;
    }

    if(_btn_select_region != NULL)
    {
        delete _btn_select_region;
        _btn_select_region = NULL;
    }

    if(_btn_record != NULL)
    {
        delete _btn_record;
        _btn_record = NULL;
    }

    if(_btn_stop != NULL)
    {
        delete _btn_stop;
        _btn_stop = NULL;
    }

    if( _radio_btn_record_normal != NULL)
    {
        delete _radio_btn_record_normal;
        _radio_btn_record_normal = NULL;
    }

    if( _radio_btn_record_dynamic_frame != NULL)
    {
        delete _radio_btn_record_dynamic_frame;
        _radio_btn_record_dynamic_frame = NULL;
    }

    if( _radio_btn_record_dynamic_period != NULL)
    {
        delete _radio_btn_record_dynamic_period;
        _radio_btn_record_dynamic_period = NULL;
    }

    if(_results != NULL)
    {
        delete[] _results;
        _results = NULL;
    }

    if(_content_view != NULL)
    {
        delete _content_view;
        _content_view = NULL;
    }

    if(_loading_dialog != NULL)
    {
        delete _loading_dialog;
        _loading_dialog = NULL;
    }

    if(_scroll_container != NULL)
    {
        delete _scroll_container;
        _scroll_container = NULL;
    }

    if(_signalMapper != NULL)
    {
        delete _signalMapper;
        _signalMapper = NULL;
    }

    if(_info != NULL)
    {
        delete _info;
        _info = NULL;
    }

    if( _video != NULL)
    {
        delete _video;
        _video = NULL;
    }

    if( _detector != NULL)
    {
        delete _detector;
        _detector = NULL;
    }

    if(_player != NULL)
    {
        delete _player;
        _player = NULL;
    }

    if(_recorder != NULL)
    {
        delete _recorder;
        _recorder = NULL;
    }
}
