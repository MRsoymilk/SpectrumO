#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QImage>
#include <QPixmap>
#include "EnviReader/envireader.h"
#include "spectrumview.h"

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

void MainWindow::init()
{
    m_spectrumView = new SpectrumView;
    m_model = new QStandardItemModel(this);
    m_scene = new QGraphicsScene(this);

    ui->treeViewItem->setModel(m_model);

    ui->graphicsView->setScene(m_scene);
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    connect(ui->graphicsView,
            &MyGraphicsView::pixelClicked,
            this,
            &MainWindow::showSpectrum);
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
    QStandardItem* rootItem = new QStandardItem(info.fileName());
    m_model->appendRow(rootItem);

    const auto& h = m_reader.header();

    for (int i = 0; i < h.wavelength.size(); ++i)
    {
        QStandardItem* bandItem =
            new QStandardItem(h.wavelength[i]);

        bandItem->setData(i, Qt::UserRole);
        rootItem->appendRow(bandItem);
    }

    ui->treeViewItem->expandAll();
}

void MainWindow::on_actionFromFolder_triggered()
{

}

void MainWindow::on_treeViewItem_clicked(const QModelIndex &index)
{
    if (!index.parent().isValid())
        return;

    int band = index.data(Qt::UserRole).toInt();

    const auto& h = m_reader.header();
    int width = h.samples;
    int height = h.lines;

    QImage img(width, height, QImage::Format_Grayscale8);

    quint16 maxVal = 1;

    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
            maxVal = qMax(maxVal, m_reader.value(band, y, x));

    for (int y = 0; y < height; ++y)
    {
        uchar* line = img.scanLine(y);
        for (int x = 0; x < width; ++x)
        {
            quint16 v = m_reader.value(band, y, x);
            line[x] = static_cast<uchar>((double)v / maxVal * 255.0);
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
    ui->graphicsView->setCropEnabled(true);
}

void MainWindow::on_tBtnPick_clicked()
{

}

void MainWindow::showSpectrum(int x, int y)
{
    const auto& h = m_reader.header();

    if (x < 0 || y < 0 ||
        x >= h.samples ||
        y >= h.lines)
        return;

    // 如果没有 wavelength，直接返回
    if (h.wavelength.size() != h.bands)
        return;

    QList<QPointF> curve;
    curve.reserve(h.bands);

    for (int b = 0; b < h.bands; ++b)
    {
        double wl = h.wavelength[b].toDouble();      // ENVI波长
        double val = m_reader.value(b, y, x);

        curve.append(QPointF(wl, val));
    }

    m_spectrumView->addSpectrum(curve);
    m_spectrumView->show();
}
