#include "CShowCurrentChannelWave.h"

#include <mutex>

#include "ui_CShowCurrentChannelWave.h"

CShowCurrentChannelWave::CShowCurrentChannelWave(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CShowCurrentChannelWave)
{
    ui->setupUi(this);
	chart = new QChart();
	chart->legend()->setVisible(true);
	seriesList = new QLineSeries();
	chart->addSeries(seriesList);
	// 创建X轴（横轴）: 这里假设显示100个采样点
	if (!axisX)
	{
		axisX = new QValueAxis;
		axisX->setTitleText(QString::fromLocal8Bit("采样点"));
		axisX->setRange(0, 500);
		chart->addAxis(axisX, Qt::AlignBottom);
		seriesList->attachAxis(axisX);
	}

	if (!axisY)
	{
		axisY = new QValueAxis;
		axisY->setTitleText(QString::fromLocal8Bit("振幅/mv"));
		axisY->setRange(-7000, 7000);
		axisY->setTickInterval(1000);
		chart->addAxis(axisY, Qt::AlignLeft);
		seriesList->attachAxis(axisY);
	}
	chartView = new QChartView(chart);
	chartView->setRenderHint(QPainter::Antialiasing);
	ui->verticalLayout->addWidget(chartView);
	chartView->show();

}

CShowCurrentChannelWave::~CShowCurrentChannelWave()
{
    delete ui;
}

void CShowCurrentChannelWave::clearWave()
{
	seriesList->clear();

}

void CShowCurrentChannelWave::setData(int sampleCount, double channelData)
{
;
	seriesList->append(sampleCount, channelData);
}

void CShowCurrentChannelWave::setCurrentWindowsTitle(QString title)
{
	setWindowTitle(title);
}

void CShowCurrentChannelWave::closeShowChannel()
{
	this->close();
}



