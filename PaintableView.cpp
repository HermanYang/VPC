#include "PaintableView.h"
#include <QPainter>
#include <QPaintEvent>

PaintableView::PaintableView(QWidget *parent) :
    QLabel(parent)
{
    _paintable = false;
    _rect = new QRect;

    _x_ratio = 1.0f;
    _y_ratio = 1.0f;

    setFixedSize(800, 600);
}

void PaintableView::setPaintingRectLeftTop(uint32_t x, uint32_t y)
{
    _rect->setCoords(0, -1, 0, -1);
    _rect->setTopLeft(QPoint(x, y));
}

void PaintableView::setPaintingRectRightBottom(uint32_t x, uint32_t y)
{
    _rect->setBottomRight(QPoint(x, y));
}

void PaintableView::setPaintable(bool paintable)
{
    _paintable = paintable;
}

void PaintableView::paintEvent(QPaintEvent *event)
{
    const QPixmap* raw_bg = pixmap();

    if( raw_bg != NULL)
    {
        QPainter widget_painter(this);

        _x_ratio = (float)width()/(float)raw_bg->width();
        _y_ratio = (float)height()/(float)raw_bg->height();

        QPixmap scale_bg = raw_bg->scaled(size());

        QPixmap mask = QPixmap(scale_bg.width(), scale_bg.height());

        QPainter pixmap_painter(&scale_bg);

        QPainter mask_painter(&mask);

        mask_painter.setRenderHint(QPainter::Antialiasing);
        pixmap_painter.setRenderHint(QPainter::Antialiasing);
        widget_painter.setRenderHint(QPainter::Antialiasing);

        if( _paintable && _rect->isValid() && _rect->width() > 1 && _rect->height() > 1 )
        {
            mask_painter.fillRect(QRect(0, 0, scale_bg.width(), scale_bg.height()), QBrush(QColor(128, 128, 128, 128)));
            mask_painter.eraseRect(*_rect);
            pixmap_painter.setCompositionMode(QPainter::CompositionMode_Multiply);
            pixmap_painter.drawPixmap(0, 0, mask);
        }

        widget_painter.drawPixmap(0, 0, scale_bg);
    }
}

void PaintableView::setPaintingRect(QRect rect)
{
    *_rect = rect;
}

QRect PaintableView::getPaintingRect()
{
    return *_rect;
}

QRect PaintableView::getLogicalRect()
{
    QRect rect;

    if( _rect->isValid() && _rect->width() > 1 && _rect->height() > 1 )
    {
        rect.setTopLeft(QPoint( (float)_rect->left() / _x_ratio, (float)_rect->top() / _y_ratio ));
        rect.setBottomRight(QPoint( (float)_rect->right() / _x_ratio, (float)_rect->bottom() / _y_ratio));
    }

    return rect;
}

PaintableView::~PaintableView()
{
    if(_rect != NULL)
    {
        delete _rect;
        _rect = NULL;
    }
}
