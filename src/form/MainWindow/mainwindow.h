#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QGraphicsScene>
#include <QMainWindow>
#include <QStandardItem>

class EnviReader;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
private slots:
    void on_actionFromFile_triggered();
    void on_actionFromFolder_triggered();

    void on_treeViewItem_clicked(const QModelIndex &index);

    void on_tBtnOriginalImg_clicked();
    void on_tBtnZoomIn_clicked();
    void on_tBtnZoomOut_clicked();
    void on_tBtnCrop_clicked();
    void on_tBtnPick_clicked();

private:
    void init();

private:
    Ui::MainWindow *ui;
    QStandardItemModel* m_model = nullptr;
    QGraphicsScene* m_scene = nullptr;
    double m_scaleFactor = 1.15;

    QPointF m_cropStart;
    QGraphicsRectItem* m_cropRect = nullptr;
    bool m_isCropping = false;
};
#endif // MAINWINDOW_H
