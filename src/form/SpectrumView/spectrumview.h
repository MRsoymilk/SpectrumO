#ifndef SPECTRUMVIEW_H
#define SPECTRUMVIEW_H

#include <QWidget>

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include "MyChartView/mychartview.h"

namespace Ui {
class SpectrumView;
}

class SpectrumView : public QWidget
{
    Q_OBJECT

public:
    explicit SpectrumView(QWidget *parent = nullptr);
    ~SpectrumView();
    void addSpectrum(const QList<QPointF> &curve);

private slots:
    void on_tBtnClear_clicked();

private:
    void init();

private:
    Ui::SpectrumView *ui;
    QChart *m_chart;
    MyChartView *m_chartView;
    void updateAxisRange(const QList<QPointF> &curve);
};

#endif // SPECTRUMVIEW_H
