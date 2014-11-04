#ifndef NOTEPUSHABLEWIDGET_H
#define NOTEPUSHABLEWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>

class NotePushableWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NotePushableWidget(QWidget *parent = 0);
    ~NotePushableWidget();
    void setNote(QString note);
    void setPixmap(QPixmap& pixmap, QSize size);

signals:
    void clicked();

public slots:

private:
    QPushButton* _button;
    QLabel* _label;

};

#endif // NOTEPUSHABLEWIDGET_H
