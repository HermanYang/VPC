#include "LoadingDialog.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QMovie>
#include <QPushButton>
#include <QGridLayout>

LoadingDialog::LoadingDialog(QWidget *parent) :
        QDialog(parent)
{
    setWindowFlags(Qt::SplashScreen);

    QLabel*	loading = new QLabel();
    QMovie* movie = new QMovie(":images/loading.gif");
    loading->setMovie(movie);
    movie->start();

    QPushButton* btn_cancel = new QPushButton("Cancel");
    btn_cancel->setFixedSize(60, 25);

    QGridLayout* layout = new QGridLayout;

    layout->addWidget(loading, 0, 0);
    layout->addWidget(btn_cancel, 1, 0, Qt::AlignHCenter);

    setLayout(layout);
    setFixedSize(250, 70);

    QObject::connect(btn_cancel, SIGNAL(clicked()), this, SIGNAL(cancel()));
}
