#ifndef PAINTABLELABEL_H
#define PAINTABLELABEL_H

#include <QLabel>
#include <QRect>

#include "define.h"

class PaintableView : public QLabel
{
    Q_OBJECT
public:
    explicit PaintableView(QWidget *parent = 0);
    ~PaintableView();

    void setPaintingRectLeftTop(uint32_t x, uint32_t y);
    void setPaintingRectRightBottom(uint32_t x, uint32_t y);
    void setPaintingRect(QRect rect);
    void setPaintable(bool paintable);

    QRect getPaintingRect();
    QRect getLogicalRect();

protected:
    void paintEvent(QPaintEvent* event);

signals:

public slots:

private:
    bool _paintable;
    QRect* _rect;

    float _x_ratio;
    float _y_ratio;
};

#endif // PAINTABLELABEL_H
