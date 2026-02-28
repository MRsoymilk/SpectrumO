#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFile>
#include <QFileDialog>
#include <QVector>

#include <QMouseEvent>
#include <QGraphicsItem>
#include "EnviReader/envireader.h"


EnviReader m_reader;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init() {
    m_model = new QStandardItemModel(this);
    m_scene = new QGraphicsScene(this);

    ui->treeViewItem->setModel(m_model);
    ui->graphicsView->setScene(m_scene);
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
       ui->graphicsView->viewport()->installEventFilter(this);
}

void MainWindow::on_actionFromFile_triggered()
{
    QString hdrPath = QFileDialog::getOpenFileName(
        this,
        "Open HDR",
        "",
        "HDR Files (*.hdr)");

    if (hdrPath.isEmpty())
        return;

    if (!m_reader.open(hdrPath))
        return;

    m_model->clear();

    QFileInfo info(hdrPath);

    QStandardItem* rootItem =
        new QStandardItem(info.fileName());

    m_model->appendRow(rootItem);

    const auto& h = m_reader.header();

    for (int i = 0; i < h.wavelength.size(); ++i)
    {
        QStandardItem* bandItem =
            new QStandardItem(h.wavelength[i]);

        bandItem->setData(i, Qt::UserRole);  // 保存 band index
        rootItem->appendRow(bandItem);
    }

    ui->treeViewItem->expandAll();
}

void MainWindow::on_actionFromFolder_triggered()
{

}

void MainWindow::on_treeViewItem_clicked(const QModelIndex &index)
{
    if (!index.parent().isValid()) {
        ui->stackedWidget->setCurrentWidget(ui->pageInfo);
        ui->textBrowserInfo->setText(m_reader.header().toString());
        return;
    }
    ui->stackedWidget->setCurrentWidget(ui->pageGraphicsView);

    int band = index.data(Qt::UserRole).toInt();

    const auto& h = m_reader.header();

    int width = h.samples;
    int height = h.lines;

    QImage img(width, height, QImage::Format_Grayscale8);

    // 找最大值用于归一化
    quint16 maxVal = 1;

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            quint16 v = m_reader.value(band, y, x);
            if (v > maxVal)
                maxVal = v;
        }
    }

    // 生成8位灰度图
    for (int y = 0; y < height; ++y)
    {
        uchar* line = img.scanLine(y);
        for (int x = 0; x < width; ++x)
        {
            quint16 v = m_reader.value(band, y, x);
            line[x] = static_cast<uchar>(
                (double)v / maxVal * 255.0);
        }
    }

    m_scene->clear();
    m_scene->addPixmap(QPixmap::fromImage(img));

    ui->graphicsView->fitInView(
        m_scene->itemsBoundingRect(),
        Qt::KeepAspectRatio);
}

void MainWindow::on_tBtnOriginalImg_clicked()
{

}

void MainWindow::on_tBtnZoomIn_clicked()
{
    ui->graphicsView->scale(m_scaleFactor, m_scaleFactor);
}

void MainWindow::on_tBtnZoomOut_clicked()
{
    ui->graphicsView->scale(1.0 / m_scaleFactor,
                            1.0 / m_scaleFactor);
}

void MainWindow::on_tBtnCrop_clicked()
{
    m_isCropping = true;
    ui->graphicsView->setDragMode(QGraphicsView::NoDrag);
}

void MainWindow::on_tBtnPick_clicked()
{

}



void MainWindow::mousePressEvent(QMouseEvent* event)
{
    if (!m_isCropping)
        return;

    m_cropStart =
        ui->graphicsView->mapToScene(event->pos());

    m_cropRect = m_scene->addRect(
        QRectF(m_cropStart, QSizeF()),
        QPen(QColor(0, 120, 255), 2),
        QBrush(QColor(0, 120, 255, 80))   // 蓝色透明
        );
}

void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_isCropping || !m_cropRect)
        return QMainWindow::mouseMoveEvent(event);

    QPointF current =
        ui->graphicsView->mapToScene(event->pos());

    QRectF rect(m_cropStart, current);
    m_cropRect->setRect(rect.normalized());
}

void MainWindow::mouseReleaseEvent(QMouseEvent* event)
{
    if (!m_isCropping || !m_cropRect)
        return QMainWindow::mouseReleaseEvent(event);

    QRectF rect = m_cropRect->rect().normalized();

    if (rect.width() > 10 && rect.height() > 10)
    {
        // 计算缩放比例
        double viewWidth  = ui->graphicsView->viewport()->width();
        double viewHeight = ui->graphicsView->viewport()->height();

        double scaleX = viewWidth  / rect.width();
        double scaleY = viewHeight / rect.height();

        double scaleFactor = qMin(scaleX, scaleY);

        // 居中到选区中心
        ui->graphicsView->centerOn(rect.center());

        // 叠加缩放（不会重置）
        ui->graphicsView->scale(scaleFactor, scaleFactor);
    }

    m_scene->removeItem(m_cropRect);
    delete m_cropRect;
    m_cropRect = nullptr;

    m_isCropping = false;
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
}
