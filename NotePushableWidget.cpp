#include "NotePushableWidget.h"
#include <QVBoxLayout>
#include <QIcon>

using std::string;

NotePushableWidget::NotePushableWidget(QWidget *parent) :
    QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout;
    _button = new QPushButton;
    _label = new QLabel;

    layout->setSpacing(0);
    layout->addWidget(_button);
    layout->addWidget(_label);

    setLayout(layout);

    QObject::connect(_button, SIGNAL(clicked()), this, SIGNAL(clicked()));
}

void NotePushableWidget::setNote(QString note)
{
    _label->setText(note);
}

void NotePushableWidget::setPixmap(QPixmap& pixmap, QSize size)
{
    _button->setIcon(QIcon(pixmap));
    _button->setIconSize(size);
}

NotePushableWidget::~NotePushableWidget()
{
    if(_button != NULL)
    {
        delete _button;
        _button = NULL;
    }

    if(_label != NULL)
    {
        delete _label;
        _label = NULL;
    }
}
