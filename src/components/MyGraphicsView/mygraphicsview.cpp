#include "mygraphicsview.h"
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QBrush>

MyGraphicsView::MyGraphicsView(QWidget *parent)
    : QGraphicsView(parent)
{
    setDragMode(QGraphicsView::ScrollHandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
}

void MyGraphicsView::setCropEnabled(bool enabled)
{
    m_cropEnabled = enabled;

    if (enabled)
        setDragMode(QGraphicsView::NoDrag);
    else
        setDragMode(QGraphicsView::ScrollHandDrag);
}

void MyGraphicsView::mousePressEvent(QMouseEvent *event)
{
    if (!scene())
        return;

    if (event->button() == Qt::RightButton)
    {
        QPointF scenePos = mapToScene(event->pos());
        emit pixelClicked(int(scenePos.x()), int(scenePos.y()));
        return;
    }

    if (m_cropEnabled && event->button() == Qt::LeftButton)
    {
        m_isCropping = true;
        m_cropStart = mapToScene(event->pos());

        m_cropRect = scene()->addRect(
            QRectF(m_cropStart, QSizeF()),
            QPen(QColor(0,120,255), 2),
            QBrush(QColor(0,120,255,80)));

        return;
    }

    QGraphicsView::mousePressEvent(event);
}

void MyGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    if (!scene())
        return;

    m_mousePos = event->pos();
    m_showCrosshair = true;
    viewport()->update();   // 触发重绘

    QPointF scenePos = mapToScene(event->pos());

    emit pixelInfoChanged(int(scenePos.x()), int(scenePos.y()));

    if (m_isCropping && m_cropRect)
    {
        QRectF rect(m_cropStart, scenePos);
        m_cropRect->setRect(rect.normalized());
        return;
    }

    QGraphicsView::mouseMoveEvent(event);
}

void MyGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_isCropping && m_cropRect)
    {
        QRectF rect = m_cropRect->rect().normalized();

        if (rect.width() > 10 && rect.height() > 10)
            fitInView(rect, Qt::KeepAspectRatio);

        scene()->removeItem(m_cropRect);
        delete m_cropRect;
        m_cropRect = nullptr;

        m_isCropping = false;
        m_cropEnabled = false;
        setDragMode(QGraphicsView::ScrollHandDrag);
        return;
    }

    QGraphicsView::mouseReleaseEvent(event);
}

void MyGraphicsView::mouseDoubleClickEvent(QMouseEvent *)
{
    resetView();
}

void MyGraphicsView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape && m_cropRect)
    {
        scene()->removeItem(m_cropRect);
        delete m_cropRect;
        m_cropRect = nullptr;

        m_isCropping = false;
        m_cropEnabled = false;
        setDragMode(QGraphicsView::ScrollHandDrag);
        return;
    }

    QGraphicsView::keyPressEvent(event);
}

void MyGraphicsView::leaveEvent(QEvent *)
{
    m_showCrosshair = false;
    viewport()->update();
}

void MyGraphicsView::drawForeground(QPainter *painter, const QRectF &)
{
    if (!m_showCrosshair || !scene())
        return;

    painter->save();

    QPen pen(Qt::red);
    pen.setWidthF(0);   // 始终1像素
    painter->setPen(pen);

    // 把 viewport 坐标转换为 scene 坐标
    QPointF scenePos = mapToScene(m_mousePos);

    QRectF bounds = scene()->itemsBoundingRect();

    // 横线
    painter->drawLine(bounds.left(),
                      scenePos.y(),
                      bounds.right(),
                      scenePos.y());

    // 竖线
    painter->drawLine(scenePos.x(),
                      bounds.top(),
                      scenePos.x(),
                      bounds.bottom());

    painter->restore();
}

void MyGraphicsView::resetView()
{
    resetTransform();
    if (scene())
        fitInView(scene()->itemsBoundingRect(),
                  Qt::KeepAspectRatio);
}
