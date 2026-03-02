#pragma once

#include <QGraphicsView>
#include <QGraphicsRectItem>

class MyGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit MyGraphicsView(QWidget *parent = nullptr);

    void setCropEnabled(bool enabled);

signals:
    void pixelInfoChanged(int x, int y);
    void pixelClicked(int x, int y);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;

private:
    void resetView();

private:
    bool m_cropEnabled = false;
    bool m_isCropping = false;

    QPointF m_cropStart;
    QGraphicsRectItem *m_cropRect = nullptr;

    QPoint m_mousePos;
    bool m_showCrosshair = false;
};
