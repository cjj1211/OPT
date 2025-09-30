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
private:
    Ui::CShowCurrentChannelWave *ui;

    QChart *chart;//= nullptr;
    QChartView   *chartView;
    QValueAxis* axisX ;
    QValueAxis* axisY = nullptr;
  


};
#endif // CSHOWCURRENTCHANNELWAVE_H
