#include "CHistoryDataWidget.h"
#include <QtCharts>
BEGIN_NX_NAMESPACE
CHistoryDataWidget::CHistoryDataWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

}

CHistoryDataWidget::~CHistoryDataWidget()
{


}

void CHistoryDataWidget::showData(QMap<QString, QString> data)
{
    // 创建一个QLineSeries对象，用于存储折线图数据
    QtCharts::QLineSeries* series = new QtCharts::QLineSeries();
    series->setPointLabelsVisible(true); // 显示点标签
    // 设置点标签的格式
    series->setPointLabelsFormat("@yPoint"); // 只显示Y值
    series->setColor(Qt::green);
    series->setPointLabelsColor(Qt::white);
    // 设置折线图数据
    QStringList categories; // 存储日期
    QMap<QString, qreal> dataMap; // 存储数据
    for (auto it = data.begin(); it != data.end(); ++it) {
        QDate date = QDate::fromString(it.key(), "yyyy-MM-dd");
        if (!date.isValid()) continue; // 无效的日期，跳过
        categories << date.toString("yyyy-MM-dd");
        dataMap.insert(date.toString("yyyy-MM-dd"), it.value().toDouble());
    }

    // 设置折线图数据
    for (int i = 0; i < categories.size(); ++i) {
        qreal y = dataMap.value(categories.at(i));
        series->append(i, y);
    }

    // 创建一个QChart对象，并将折线图添加到其中
    QtCharts::QChart* chart = new QtCharts::QChart();
    chart->addSeries(series);
    // 创建一个QCategoryAxis对象，并设置为折线图的横坐标轴
    QtCharts::QCategoryAxis* axisX = new QtCharts::QCategoryAxis();
    for (int i = 0; i < categories.size(); ++i) {
        axisX->append(categories.at(i), i+1);
    }
    // 计算横坐标轴的范围
    double minX = -0.5; // 设置起始位置为-0.5，以保证第一个数据点显示在图表中间
    double maxX = categories.size(); // 设置结束位置为数据点的数量减0.5
    axisX->setRange(minX, maxX);
    // 添加一个格子的间隔，确保起始位置和结束位置多出一个格子
    minX -= 1;
    maxX += 1;
    axisX->setTitleText(tr("Date")); // 设置横坐标标题
    chart->addAxis(axisX, Qt::AlignBottom); // 将横坐标轴设置在底部

    chart->setBackgroundBrush(QBrush(QColor("#162053")));


    QBrush brush(Qt::white); // 创建白色画笔
    axisX->setTitleBrush(brush); // 将标题文本的画笔颜色设置为白色

    // 去掉纵坐标轴的网格线

    series->attachAxis(axisX); // 将折线图关联到横坐标轴
    // 创建一个QValueAxis对象，并设置为折线图的纵坐标轴
    QtCharts::QValueAxis* axisY = new QtCharts::QValueAxis();
    axisY->setTitleText(tr("Value"));
    axisX->setGridLineVisible(false);
    chart->addAxis(axisY, Qt::AlignLeft); // 将纵坐标轴设置在左边
    // 设置横坐标轴的网格线为虚线
    axisY->setGridLineVisible(true);
    axisX->setGridLinePen(QPen(Qt::white,1, Qt::DashLine)); // 设置虚线样式
    axisY->setTitleBrush(brush); // 将标题文本的画笔颜色设置为白色
// 创建并设置横轴的字体和颜色
    QFont xAxisFont;
    xAxisFont.setBold(true);
    xAxisFont.setPointSize(10);
    QColor fontColor(Qt::white); // 设置字体颜色为白色
    axisX->setLabelsFont(xAxisFont);
    axisX->setLabelsBrush(fontColor);

    // 创建并设置纵轴的字体和颜色
    QFont yAxisFont;
    yAxisFont.setBold(true);
    yAxisFont.setPointSize(10);
    axisY->setLabelsFont(yAxisFont);
    axisY->setLabelsBrush(fontColor);

    series->attachAxis(axisY); // 将折线图关联到纵坐标轴
    // 创建一个QChartView对象，并将QChart对象添加到其中
    QtCharts::QChartView* chartView = new QtCharts::QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing); // 设置抗锯齿渲染
    chart->legend()->hide();

ui.datachart->addWidget(chartView);
}
END_NX_NAMESPACE