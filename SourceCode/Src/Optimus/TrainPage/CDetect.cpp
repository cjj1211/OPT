#include "CDetect.h"
#include "COscillograph.h"
#include "../CImpedanceWidget.h"
#include "../CPlaybackToolbar.h"
#include "../CDetectToolbar.h"
#include "../CHeatMap.h"
#include "../CProgressDialog.h"
#include "NxAppService/COscillographService.h"
#include "NxAppService/CPlaybackService.h"
#include "NxEntity/ChannelPlan.h"
#include "NxEntity/TreatmentRecord.h"
#include "NxElectrodeMap/CElectrodePreview.h"
#include "CEventWidget.h"
#include <vector>
#include <algorithm>
#include <QSet>
#include <QRandomGenerator>
#include <QFile>
#include <QDateTime>
#include <QDebug>
#include <QFileDialog>

#include "Optimus/CExportEEGDialog.h"

BEGIN_NX_NAMESPACE
    CDetect::CDetect(const QString& currentPatientUid, QWidget* parent)
	: QWidget(parent)
	, patientUid(currentPatientUid)
	, detectToolbar(new CDetectToolbar(this))
	, impedance(nullptr)
	, service(new COscillographService(currentPatientUid, this))
	, heatMap(nullptr)
	, progressDialog(nullptr)
{
	ui = std::make_unique<Ui::Detect>();
	ui->setupUi(this);
	ui->toolbarLayout->addWidget(detectToolbar);
	currentModel = 0;
}

CDetect::CDetect(const QString& currentPatientUid, const QString& recordUid, QWidget* parent)
	: QWidget(parent)
	, patientUid(currentPatientUid)
	, playbackToolbar(new CPlaybackToolbar(this))
	, impedance(nullptr)
	, service(new CPlaybackService(currentPatientUid, recordUid, this))
	, heatMap(nullptr)
	, progressDialog(nullptr)
{
	ui = std::make_unique<Ui::Detect>();
	ui->setupUi(this);
	ui->toolbarLayout->addWidget(playbackToolbar);
	TreatmentRecord recordInfo;
	service->getRecordInfo(recordInfo);
	playbackToolbar->setTimeLength(recordInfo.TrainTimeLen);
	startTime = QString::fromStdString(recordInfo.StartDatetime);
	trainTimeLen = recordInfo.TrainTimeLen;
	currentModel = 1;
}
CDetect::~CDetect() = default;

void CDetect::initOscillograph()
{
	if(0 == currentModel)
	{
		oscillograph = new COscillograph(service, false, this);
		eventWidget = new CEventWidget(this);
		ui->oscillographContainer->addWidget(oscillograph);

		initChannelSelectorCard();
		initHidedChannelCard();
		QHeaderView* horizontalHeader = ui->eventTable->horizontalHeader();
		horizontalHeader->resizeSection(0, 90); // 自适应模式
		horizontalHeader->resizeSection(1, 50); // 自适应模式
		horizontalHeader->resizeSection(2, 70); // 自适应模式
		horizontalHeader->resizeSection(3, 16); // 自适应模式
		ui->verticalLayout_7->addWidget(eventWidget);
		horizontalHeader->setSectionResizeMode(QHeaderView::Fixed);
		connect(oscillograph, SIGNAL(signalUpdateTimestamp(const uint64_t)), detectToolbar, SLOT(slotUpdateRuntime(const uint64_t)));
		connect(oscillograph, SIGNAL(signalExitOscillograph()), this, SIGNAL(signalExitOscillograph()));
		connect(detectToolbar, SIGNAL(signalRun(bool)), this, SLOT(slotRun(bool)));
		connect(detectToolbar, SIGNAL(signalRecord(bool)), this, SLOT(slotRecord(bool)));
		connect(detectToolbar, SIGNAL(signalSpikeScope()), oscillograph, SLOT(slotToggleSpikeScope()));
		connect(detectToolbar, SIGNAL(signalImpedance()), this, SLOT(slotImpedance()));
		connect(detectToolbar, SIGNAL(signalChannelMap()), this, SLOT(slotChannelMap()));
		connect(eventWidget, &CEventWidget::buttonClick, this, &CDetect::slotButtonClick);
	}
	else
	{
		oscillograph = new COscillograph(service, true, this);
		eventWidget = new CEventWidget();
		ui->oscillographContainer->addWidget(oscillograph);

		initChannelSelectorCard();
		initHidedChannelCard();
		QHeaderView* horizontalHeader = ui->eventTable->horizontalHeader();
		horizontalHeader->resizeSection(0, 90); // 自适应模式
		horizontalHeader->resizeSection(1, 50); // 自适应模式
		horizontalHeader->resizeSection(2, 70); // 自适应模式
		horizontalHeader->resizeSection(3, 16); // 自适应模式
		ui->verticalLayout_7->addWidget(eventWidget);
		horizontalHeader->setSectionResizeMode(QHeaderView::Fixed);
		connect(oscillograph, SIGNAL(signalUpdateTimestamp(const uint64_t)), playbackToolbar, SLOT(slotUpdateRuntime(const uint64_t)));
		connect(oscillograph, SIGNAL(signalServerIsStoped()), this, SLOT(slotServerStoped()));
		connect(oscillograph, SIGNAL(signalExitOscillograph()), this, SIGNAL(signalPlaybackExit()));
		connect(playbackToolbar, SIGNAL(signalPlay(bool)), this, SLOT(slotRun(bool)));
		connect(playbackToolbar, SIGNAL(signalJumpTo(const uint64_t)), this, SLOT(slotJumpTo(const uint64_t)));
		connect(playbackToolbar, SIGNAL(signalSpikeScope()), oscillograph, SLOT(slotToggleSpikeScope()));
		connect(playbackToolbar, SIGNAL(signalImpedance()), this, SLOT(slotPlaybackImpedance()));
		connect(playbackToolbar, SIGNAL(signalExport()), this, SLOT(slotPlaybackExport()));
		connect(playbackToolbar, SIGNAL(signalBack()), this, SIGNAL(signalPlaybackExit()));
		connect(eventWidget, &CEventWidget::buttonClick, this, &CDetect::slotButtonClick);
		connect(service, SIGNAL(signalCompressProgress(QString, int, int)), this, SLOT(slotCompressProgress(QString, int, int)));
		connect(ui->eventTable, &QTableWidget::doubleClicked, this, &CDetect::doubleClickEventTable);
	}
	auto isInit=oscillograph->init();
	if(isInit)
	{
		loadEventRecorder(service->getEventSavePath());
		initTableWidget();
	}
	else
	{
		return;
	}
}

void CDetect::doubleClickEventTable() const
{
    const auto index = ui->eventTable->currentRow();
    const uint64_t time = QDateTime::fromString(startTime, ("yyyy-MM-dd hh:mm:ss")).secsTo(QDateTime::fromString(eventStartTime.at(index), ("yyyy-MM-dd hh:mm:ss")));
	if (trainTimeLen >= time)
	{
		playbackToolbar->jumpByEvent(time);
	}
}
void CDetect::initChannelSelectorCard()
{
    const auto channelPlans = service->getChannelPlans();
	QVector<std::string> areaNums;
	for (auto& plan : channelPlans)
	{
		if (!areaNums.contains(plan.AreaNum))
		{
			areaNums.append(plan.AreaNum);
		}
	}
	std::ranges::sort(areaNums);
	for (auto areaNum : areaNums)
	{
		QVector<ChannelPlan*> plansInArea;
		for (auto& plan : channelPlans)
		{
			if (plan.AreaNum == areaNum)
			{
				plansInArea.append(new ChannelPlan(plan));
			}
		}
		auto card = new CChannelSelectorCard(service, plansInArea, true, this);
		channelSelectorCardVec.append(card);
		ui->channelFilterCardContainer->addWidget(card);
		connect(card, SIGNAL(signalChannelPlanToggled(const QString&, bool)), oscillograph, SLOT(slotChannelPlanToggled(const QString&, bool)));
		connect(card, SIGNAL(signalSampleRateChanged()), oscillograph, SLOT(slotResetSampleRate()));
	}
}
void CDetect::slotRun(bool isRun) {
	if (isRun) {
		service->startRun();
	}
	else {
		service->stopRun();
	}
}

void CDetect::slotRecord(bool isStartRecord) {
	if (isStartRecord) {
		service->startRecord();
		ui->eventTable->clearContents();
		while (!jsonDataArray.isEmpty()) {
			jsonDataArray.removeAt(0);
		}
		loadEventRecorder(service->getEventSavePath());
	}
	else {
		service->stopRecord();
		checkEventRecord();
	}
}
void CDetect::slotImpedance() {
	if (impedance == nullptr)
	{
		impedance = new CImpedanceWidget(service, true, this);
		ui->impedanceLayout->addWidget(impedance);
		connect(impedance, SIGNAL(signalClosed()), SLOT(slotCloseImpedance()));
		connect(impedance, SIGNAL(signalChannelPlanToggled(const QString&, bool)), oscillograph, SLOT(slotChannelPlanToggled(const QString&, bool)));
	}
	ui->stackedWidget->setCurrentWidget(ui->impedancePage);
}

void CDetect::slotChannelMap() {
	// TODO 跳转通道地图后需要停止从后台获取波形数据以提高性能  szj？
	if (nullptr == heatMap)
	{
		heatMap = new CHeatMap(service, this);
		ui->channelMapContainer->addWidget(heatMap);
		connect(heatMap, SIGNAL(signalClosed()), SLOT(slotCloseHeatMap()));
	}
	ui->stackedWidget->setCurrentWidget(ui->channelMapPage);
	service->enableMeasureHeatScale(true);
}
void CDetect::initHidedChannelCard()
{
	hideChannelCard = new CHidedChannelCard(service, this);
	ui->hideChannelContainer->addWidget(hideChannelCard);
	connect(oscillograph, SIGNAL(signalUpdateDisabledWaveforms(const QStringList&)), this, SLOT(slotUpdateDisabledWaveforms(const QStringList&)));
	connect(hideChannelCard, SIGNAL(signalShowChannel(const QString&)), oscillograph, SLOT(slotEnableChannel(const QString&)));
}

void CDetect::slotUpdateDisabledWaveforms(const QStringList& customWavenames)
{
	auto oldHidedWavenames = hideChannelCard->getHidedWavenaems();
	for (auto& customWavename : oldHidedWavenames) { // 检查当前隐藏列表，如果有项目不再新的隐藏列表，则需要显示
		if (!customWavenames.contains(customWavename))
		{
			hideChannelCard->showChannel(customWavename);
		}
	}

	for (auto& customWavename : customWavenames) {
		if (!oldHidedWavenames.contains(customWavename)) {
			hideChannelCard->hideChannel(customWavename);
		}
	}
	hideChannelCard->saveHidedChannels();
}

void CDetect::saveEventRecorder()
{
	QFile file(configFilePath);
	if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QJsonDocument doc(jsonDataArray);
		file.write(doc.toJson());
		file.close();
	}
	initTableWidget();
}
void CDetect::loadEventRecorder(const QString& path)
{
	configFilePath = path;
	QFile file(configFilePath);
	if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QByteArray jsonData = file.readAll();
		QJsonDocument doc = QJsonDocument::fromJson(jsonData);
		jsonDataArray = doc.array();
		file.close();
	}
}
void CDetect::initTableWidget()
{
	eventStartTime.clear();
	if (jsonDataArray.isEmpty())
	{
		ui->eventTable->removeRow(0);
		return;
	}
	else
	{
		ui->eventTable->clearContents();
		ui->eventTable->setRowCount(0);
		for (int i = 0; i < jsonDataArray.size(); i++)
		{
			auto row = ui->eventTable->rowCount();
			ui->eventTable->insertRow(row);
			QTableWidgetItem* begintime = new QTableWidgetItem(QDateTime::fromString(jsonDataArray[i].toObject().value("begintime").toString(), ("yyyy-MM-dd hh:mm:ss")).toString("MM.dd hh:mm:ss"));
			ui->eventTable->setItem(row, 0, begintime);
			QTableWidgetItem* eventname = new QTableWidgetItem(jsonDataArray[i].toObject().value("eventname").toString());
			ui->eventTable->setItem(row, 1, eventname);
			QTableWidgetItem* duration = new QTableWidgetItem(jsonDataArray[i].toObject().value("duration").toString());
			ui->eventTable->setItem(row, 2, duration);
			eventStartTime.append(QDateTime::fromString(jsonDataArray[i].toObject().value("begintime").toString(), ("yyyy-MM-dd hh:mm:ss")).toString("yyyy-MM-dd hh:mm:ss"));
			QPushButton* deleteButton = new QPushButton(this);
			deleteButton->setIcon(QIcon(":/Optimus/images/trainPage/close.svg"));
			deleteButton->setStyleSheet("QPushButton {"
				"border: none;" // 无边框
				"background-color: transparent;" // 背景颜色
				"width: 6px;" // 最小宽度为10像素
				"height: 6px;" // 最小高度为10像素
				"border-radius: 3px;" // 圆角半径为5像素
				"}");
			ui->eventTable->setCellWidget(row, 3, deleteButton);
			connect(deleteButton, &QPushButton::clicked, this, &CDetect::slotDeleteEvent);
		}
	}
}
void CDetect::slotButtonClick(const QString& name, int count)
{
	if (currentModel == 0)
	{
		if (service->getRunStatus() == RS_RECORDING)
		{
			if (count == 1)
			{
				QJsonObject obj;
				obj["begintime"] = setEventTime(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
				obj["eventname"] = name;
				obj["duration"] = "00";
				jsonDataArray.append(obj);
			}
			if (count == 2)
			{
				for (int i = 0; i < jsonDataArray.size(); i++)
				{
					if (name == jsonDataArray[i].toObject().value("eventname").toString() && jsonDataArray[i].toObject().value("duration").toString() == "00")
					{
						QJsonObject obj;
						obj["begintime"] = jsonDataArray[i].toObject().value("begintime");
						obj["eventname"] = jsonDataArray[i].toObject().value("eventname");
						auto a = jsonDataArray[i].toObject().value("begintime").toString();
						QDateTime begintime = QDateTime::fromString(jsonDataArray[i].toObject().value("begintime").toString(), "yyyy-MM-dd hh:mm:ss");
						QDateTime endTime = QDateTime::currentDateTime();
						// 计算时间差
						qint64 milliseconds = begintime.msecsTo(endTime);
						QTime timeDifference(0, 0, 0, 0);
						timeDifference = timeDifference.addMSecs(milliseconds);
						// 格式化时间显示
						QString formattedTime;
						if (timeDifference.hour() > 0) {
							formattedTime = timeDifference.toString("hh:mm:ss");

							int firstColonIndex = formattedTime.indexOf(":"); // 获取第一个冒号的索引位置
							int secondColonIndex = formattedTime.indexOf(":", firstColonIndex + 1); // 获取第二个冒号的索引位置，从第一个冒号之后开始查找
							if(firstColonIndex != -1 && secondColonIndex != -1) {
								formattedTime.replace(firstColonIndex, 1, "h"); // 将第一个冒号替换为"h"
								formattedTime.replace(secondColonIndex, 1, "'"); // 将第二个冒号替换为“’”
							}
							formattedTime = formattedTime + "''";
						}
						else if (timeDifference.minute() > 0) {
							formattedTime = timeDifference.toString("mm:ss");
							formattedTime = formattedTime.replace(":", "'");
							formattedTime = formattedTime + "''";
						}
						else {
							formattedTime = timeDifference.toString("ss");
							if (formattedTime == "00")
							{
								formattedTime = "01";
							}
							formattedTime = formattedTime + "''";
						}
						obj["duration"] = formattedTime;
						jsonDataArray.replace(i, obj);
					}
				}
			}
			saveEventRecorder();
		}
	}
	else
	{
		if (service->getRunStatus() == RS_RUNNING)
		{
			if (count == 1)
			{
				QJsonObject obj;
				obj["begintime"] = setEventTime(service->getRecordStartDateTime().addSecs(service->getRuntimeInSecs()).toString("yyyy-MM-dd hh:mm:ss"));
				obj["eventname"] = name;
				obj["duration"] = "00";
				jsonDataArray.append(obj);
			}
			if (count == 2)
			{
				for (int i = 0; i < jsonDataArray.size(); i++)
				{
					if (name == jsonDataArray[i].toObject().value("eventname").toString() && jsonDataArray[i].toObject().value("duration").toString() == "00")
					{
						QJsonObject obj;
						obj["begintime"] = jsonDataArray[i].toObject().value("begintime");
						obj["eventname"] = jsonDataArray[i].toObject().value("eventname");
						QDateTime begintime = QDateTime::fromString(jsonDataArray[i].toObject().value("begintime").toString(), "yyyy-MM-dd hh:mm:ss");
						QDateTime endTime = service->getRecordStartDateTime().addSecs(service->getRuntimeInSecs());
						// 计算时间差
						qint64 milliseconds = begintime.msecsTo(endTime);
						if (milliseconds < 0)
						{
							return;
						}
						QTime timeDifference(0, 0, 0, 0);
						timeDifference = timeDifference.addMSecs(milliseconds);
						// 格式化时间显示
						QString formattedTime;
						if (timeDifference.hour() > 0) {
							formattedTime = timeDifference.toString("hh");
							formattedTime += "h";
						}
						else if (timeDifference.minute() > 0) {
							formattedTime = timeDifference.toString("mm:ss");
							formattedTime = formattedTime.replace(":", "'");
							formattedTime = formattedTime + "''";
						}
						else {
							formattedTime = timeDifference.toString("ss");
							if (formattedTime == "00")
							{
								formattedTime = "01";
							}
							formattedTime = formattedTime + "''";
						}
						obj["duration"] = formattedTime;
						jsonDataArray.replace(i, obj);
					}
				}
			}
			saveEventRecorder();
		}
	}
}
QString CDetect::setEventTime(const QString& time)
{
	return time;
}
void CDetect::checkEventRecord()
{
	if (currentModel == 0)
	{
		for (int i = 0; i < jsonDataArray.size(); i++)
		{
			if (jsonDataArray[i].toObject().value("duration").toString() == "00")
			{
				QJsonObject obj;
				obj["begintime"] = jsonDataArray[i].toObject().value("begintime");
				obj["eventname"] = jsonDataArray[i].toObject().value("eventname");
				auto a = jsonDataArray[i].toObject().value("begintime").toString();
				QDateTime begintime = QDateTime::fromString(jsonDataArray[i].toObject().value("begintime").toString(), "yyyy-MM-dd hh:mm:ss");
				QDateTime endTime = QDateTime::currentDateTime();
				// 计算时间差
				qint64 milliseconds = begintime.msecsTo(endTime);
				QTime timeDifference(0, 0, 0, 0);
				timeDifference = timeDifference.addMSecs(milliseconds);
				// 格式化时间显示
				QString formattedTime;
				if (timeDifference.hour() > 0) {
					formattedTime = timeDifference.toString("hh");
					formattedTime += "h";
				}
				else if (timeDifference.minute() > 0) {
					formattedTime = timeDifference.toString("mm:ss");
					formattedTime = formattedTime.replace(":", "'");
					formattedTime = formattedTime + "''";
				}
				else {
					formattedTime = timeDifference.toString("ss");
					if (formattedTime == "00")
					{
						formattedTime = "01";
					}
					formattedTime = formattedTime + "''";
				}
				obj["duration"] = formattedTime;
				jsonDataArray.replace(i, obj);
			}
		}
	}
	else
	{
		for (int i = 0; i < jsonDataArray.size(); i++)
		{
			if (jsonDataArray[i].toObject().value("duration").toString() == "00")
			{
				QJsonObject obj;
				obj["begintime"] = jsonDataArray[i].toObject().value("begintime");
				obj["eventname"] = jsonDataArray[i].toObject().value("eventname");
				auto a = jsonDataArray[i].toObject().value("begintime").toString();
				QDateTime begintime = QDateTime::fromString(jsonDataArray[i].toObject().value("begintime").toString(), "yyyy-MM-dd hh:mm:ss");
				QDateTime endTime = service->getRecordStartDateTime().addSecs(service->getRuntimeInSecs());
				// 计算时间差
				qint64 milliseconds = begintime.msecsTo(endTime);
				if (milliseconds < 0)
				{
					return;
				}
				QTime timeDifference(0, 0, 0, 0);
				timeDifference = timeDifference.addMSecs(milliseconds);
				// 格式化时间显示
				QString formattedTime;
				if (timeDifference.hour() > 0) {
					formattedTime = timeDifference.toString("hh:mm:ss");
					formattedTime = formattedTime.replace(":", "h");
					formattedTime = formattedTime.replace(":", "'");
					formattedTime = formattedTime + "''";
				}
				else if (timeDifference.minute() > 0) {
					formattedTime = timeDifference.toString("mm:ss");
					formattedTime = formattedTime.replace(":", "'");
					formattedTime = formattedTime + "''";
				}
				else {
					formattedTime = timeDifference.toString("ss");
					if (formattedTime == "00")
					{
						formattedTime = "01";
					}
					formattedTime = formattedTime + "''";
				}
				obj["duration"] = formattedTime;
				jsonDataArray.replace(i, obj);
			}
		}
	}
	saveEventRecorder();
}

void CDetect::slotChannelPlanToggled(const QString& planUid, bool enabled)
{
	qDebug() << planUid << " " << enabled;
}

void CDetect::slotCloseImpedance()
{
	ui->stackedWidget->setCurrentWidget(ui->waveformPage);
	ui->impedanceLayout->removeWidget(impedance);
	delete impedance;
	impedance = nullptr;
	updateEnabledStatus();
}
void CDetect::slotCloseHeatMap()
{
	service->enableMeasureHeatScale(false);
	ui->stackedWidget->setCurrentWidget(ui->waveformPage);
	ui->impedanceLayout->removeWidget(heatMap);
	delete heatMap;
	heatMap = nullptr;
	updateEnabledStatus();
}

void CDetect::slotJumpTo(const uint64_t timeInSecs) const
{
	if (service->getRunStatus() == RS_RUNNING)
	{
		service->stopRun();
	}

	service->jumpToSecs(timeInSecs);
	oscillograph->resetWaveform();
}

void CDetect::slotPlaybackImpedance()
{
	if (impedance == nullptr)
	{
		impedance = new CImpedanceWidget(service, false, this);
		ui->impedanceLayout->addWidget(impedance);
		connect(impedance, SIGNAL(signalClosed()), SLOT(slotCloseImpedance()));
	}
	ui->stackedWidget->setCurrentWidget(ui->impedancePage);
}
void CDetect::slotPlaybackExport()
{
	exportDialog = new CExportEEGDialog(service,this);
	exportDialog->exec();
	//auto selectDir = QFileDialog::getExistingDirectory();
	//if (!selectDir.isEmpty())
	//{
	//	if (progressDialog == nullptr)
	//	{
	//		progressDialog = new CProgressDialog(Qt::WindowModal, this);
	//		progressDialog->setContent(tr("Export directory to ") + selectDir);
	//	}
	//	service->exportToDir(selectDir);
	//	progressDialog->show();
	//}
}
// 回放时，后端向前端发送回放结束消息
void CDetect::slotServerStoped()
{
	if (playbackToolbar)
	{
		service->stopRun();
		playbackToolbar->slotServerStoped();
		if (currentModel == 1)
		{
			checkEventRecord();
		}
	}
}

void CDetect::slotCompressProgress(QString fileName, int currentIndex, int allIndex)
{
	if (nullptr != progressDialog)
	{
		QString content = tr("Exporting File:") + QString::number(currentIndex) + "/" + QString::number(allIndex) + " " + fileName;
		progressDialog->setContent(content);
		if (currentIndex == allIndex)
		{
			progressDialog->close();
		}
	}
}
void CDetect::slotDeleteEvent()
{
	auto currentRow = ui->eventTable->currentRow();
	jsonDataArray.removeAt(currentRow);
	saveEventRecorder();
}

void CDetect::updateEnabledStatus()
{
	for (auto card : channelSelectorCardVec) {
		card->updateEnabledStatus();
	}
}

bool CDetect::isRecording()
{
	return service->getRunStatus() != RS_IDLE;
}

void CDetect::stopRecord() {
	service->stopRecord();
}
END_NX_NAMESPACE


