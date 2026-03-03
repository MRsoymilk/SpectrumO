#include "spectrumview.h"
#include "ui_spectrumview.h"
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QRandomGenerator>

SpectrumView::SpectrumView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SpectrumView)
{
    ui->setupUi(this);
    init();
}

SpectrumView::~SpectrumView()
{
    delete ui;
}

void SpectrumView::init()
{
    m_chart = new QChart();
    m_chart->setTitle("Pixel Spectrum");

    m_chartView = new MyChartView(m_chart, this);
    m_chartView->setRenderHint(QPainter::Antialiasing);


    ui->gLayChart->addWidget(m_chartView);
}

void SpectrumView::addSpectrum(const QList<QPointF> &curve)
{
    if (curve.isEmpty())
        return;

    QLineSeries *series = new QLineSeries();

    for (const QPointF &pt : curve)
        series->append(pt);

    QColor color = QColor::fromHsv(
        QRandomGenerator::global()->bounded(360),
        200,
        230);

    series->setColor(color);
    series->setName(QString("Curve %1")
                        .arg(m_chart->series().size() + 1));

    m_chart->addSeries(series);

    if (m_chart->axes().isEmpty())
    {
        QValueAxis *axisX = new QValueAxis;
        QValueAxis *axisY = new QValueAxis;

        axisX->setTitleText("Band");
        axisY->setTitleText("DN Value");

        m_chart->addAxis(axisX, Qt::AlignBottom);
        m_chart->addAxis(axisY, Qt::AlignLeft);

        series->attachAxis(axisX);
        series->attachAxis(axisY);
    }
    else
    {
        for (QAbstractAxis *axis : m_chart->axes())
            series->attachAxis(axis);
    }

    updateAxisRange(curve);
}

void SpectrumView::updateAxisRange(const QList<QPointF> &curve)
{
    if (curve.isEmpty())
        return;

    double minX = curve.first().x();
    double maxX = curve.first().x();
    double minY = curve.first().y();
    double maxY = curve.first().y();

    for (const QPointF &pt : curve)
    {
        minX = qMin(minX, pt.x());
        maxX = qMax(maxX, pt.x());
        minY = qMin(minY, pt.y());
        maxY = qMax(maxY, pt.y());
    }

    QValueAxis *axisX =
        qobject_cast<QValueAxis*>(m_chart->axes(Qt::Horizontal).first());

    QValueAxis *axisY =
        qobject_cast<QValueAxis*>(m_chart->axes(Qt::Vertical).first());

    if (!axisX || !axisY)
        return;

    if (m_chart->series().size() == 1)
    {
        axisX->setRange(minX, maxX);
        axisY->setRange(minY, maxY);
    }
    else
    {
        axisX->setRange(qMin(axisX->min(), minX),
                        qMax(axisX->max(), maxX));

        axisY->setRange(qMin(axisY->min(), minY),
                        qMax(axisY->max(), maxY));
    }
}


void SpectrumView::on_tBtnClear_clicked()
{
    if (!m_chart)
        return;

    m_chart->removeAllSeries();
}

