#ifndef CSHOWCURRENTCHANNELWAVE_H
#define CSHOWCURRENTCHANNELWAVE_H

#include <mutex>
#include <QChartView>
#include <QLineSeries>
#include <QWidget>
#include <QValueAxis>
QT_BEGIN_NAMESPACE
namespace Ui { class CShowCurrentChannelWave; }
QT_END_NAMESPACE
using namespace  QtCharts;
class CShowCurrentChannelWave : public QWidget
{
    Q_OBJECT

public:
    CShowCurrentChannelWave(QWidget *parent = nullptr);
    ~CShowCurrentChannelWave();
    void clearWave();
    void setData(int x, double data);
    QLineSeries* seriesList = nullptr;
    void setCurrentWindowsTitle(QString title);
    void closeShowChannel();
    void setAxisXName(QString  namne);
    void setAxisYName(QString  name);
    void setAxisXRange(int min, int max)
    {
        if (axisX)
        {
            axisX->setRange(min, max);
        }
	}
    void setAxisYRange(int min, int max)
    {
        if (axisY)
        {
            axisY->setRange(min, max);
        }
	}
    void setAxisYTickInterval(int interval)
    {
        if (axisY)
        {
            axisY->setTickInterval(interval);
        }
	}
    void setAxisXTickInterval(int interval)
    {
        if (axisX)
        {
            axisX->setTickInterval(interval);
        }
	}
	int rangeMinX = 5000;
    int rangeMaxY = 3000;
private:
    Ui::CShowCurrentChannelWave *ui;

    QChart *chart;//= nullptr;
    QChartView   *chartView;
    QValueAxis* axisX ;
    QValueAxis* axisY = nullptr;
  


};
#endif // CSHOWCURRENTCHANNELWAVE_H
