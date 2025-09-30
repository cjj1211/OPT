#include "CChannelPlanWidget.h"
#include "NxAppService/CChannelPlanService.h"
#include "NxAppService/CChannelNameService.h"
#include "NxEntity/ChannelPlan.h"
#include "NxEntity/ChannelName.h"
#include "NxElectrodeMap/CElectrodeSettingInterface.h"
#include "NxBrainAreaWidget/CBrainAreaWidget.h"
#include "NxBrainAreaWidget/AtlasConfiguration.h"
#include "../CProgressDialog.h"
#include <QUuid>
#include <QDateTime>
#include <QMessageBox>
#include <QDir>
#include <QLineEdit>
#include <QVector3D>

#include "../CNxMessageBox.h"
#include "CLogger.h"
BEGIN_NX_NAMESPACE

const QString kHemisphereNames[] = {"Left Hemisphere","Right Hemisphere"};
const QString kHemisphereNames_CN[] = { QString::fromLocal8Bit("左脑"), QString::fromLocal8Bit("右脑") };

CChannelPlanWidget::CChannelPlanWidget(QString CurrentPatientUid, QWidget* parent)
    :QWidget(parent)
    , currentPatientUid(CurrentPatientUid)
    , channelService(new CChannelPlanService(this))
	, channelNameService(new CChannelNameService(this))
    , brainArea(new CBrainAreaWidget(this))
{
    ui = std::make_unique<Ui::CChannelPlanWidget>();
    ui->setupUi(this);
	
	update();
	qApp->processEvents();
    ui->brainAreaContainer->addWidget(brainArea);
    connect(ui->deletebtn, &QPushButton::clicked, this, &CChannelPlanWidget::deleteChannelPlan);
    connect(ui->savebtn, &QPushButton::clicked, this, &CChannelPlanWidget::saveChannelPlan);
    connect(ui->mapbtn, &QPushButton::clicked, this, &CChannelPlanWidget::showChannelMap);
    connect(ui->insert, &QPushButton::clicked, this, &CChannelPlanWidget::insertRow);
    setting = new  ElectrodeMapSettings();
    electrodeSettingInterface = new CElectrodeSettingInterface(setting);
    findXML("../config/ElectrodeMapConfig");
	validator = new QDoubleValidator(this);
    const auto cfg = systemConfig.GetSystemCfg();
    const auto port = cfg.Common.FPGAPort;
    for (auto current : port)
    {
        fpgaPort.push_back(QString::fromStdString(current));
        fpgaList.append(0);
    }

    setTableWidgetStyle();
    if (!allMapInfo.empty())
    {
        initTable();
    }
	ui->savebtn->setEnabled(false);
	ui->savebtn->setStyleSheet(buttonStyle);
}
CChannelPlanWidget::~CChannelPlanWidget() = default;

void CChannelPlanWidget::showChannelMap()
{
	channelMap.clear();
    const auto currentRow = ui->tableWidget->currentRow();
	if (currentRow < 0)
	{
		CNxMessageBox msgBox(QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("请选中一行"));
		msgBox.exec();
		return;
	}
	else
	{
        const auto baseTypeComboBox = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(currentRow, 0));
        const auto channel = channelService->getChannelPlanByIndexs(currentPatientUid, baseTypeComboBox->currentText());
		channelPlanFK = QString::fromStdString(channel.UID);
	}
	if (saveState==1)
	{
		CNxMessageBox msgBox(QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("通道规划未保存，是否保存?"));
		msgBox.exec();
        const bool confirmed = msgBox.isConfirmed();
		if (confirmed)
		{
			saveChannelPlan();
			return;
		}
		else
		{
			for (int i = 0; i < ui->tableWidget->rowCount(); i++)
			{
                const auto baseTypeComboBox = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(i, 4));
				if (baseTypeComboBox)
				{
					// 使用 QComboBox 的 currentIndex 方法获取索引
					QPair<QString, QString> current;
                    const auto ComboBox = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(i, 0));
					current.first = ComboBox->currentText();
					current.second = baseTypeComboBox->currentText();
					channelMap.append(current);
				}
			}
			LOG_INFO("emit chanelmap show ");
			emit channelMapShow(currentRow);
			return;
		}
		
	}
	for (int i = 0; i < ui->tableWidget->rowCount(); i++)
	{
        const auto baseTypeComboBox = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(i, 4));
			if (baseTypeComboBox)
			{
				// 使用 QComboBox 的 currentIndex 方法获取索引
				QPair<QString, QString> current;
                const auto ComboBox = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(i, 0));
				current.first = ComboBox->currentText();
				current.second = baseTypeComboBox->currentText();
				channelMap.append(current);
			}
	}
	LOG_INFO("emit..................channelMapShow.");
	emit channelMapShow(currentRow);
}
void CChannelPlanWidget::setTableWidgetStyle()
{
	ui->tableWidget->setShowGrid(false); //设置显示格子线
	ui->tableWidget->setColumnCount(11); //设置列数
	ui->tableWidget->hideColumn(10);
	ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows); //整行选中的方式
	ui->tableWidget->setColumnWidth(1, 150);
	ui->tableWidget->setColumnWidth(2, 150);
	ui->tableWidget->setColumnWidth(3, 150);
	ui->tableWidget->setColumnWidth(4, 170);
	ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui->tableWidget->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui->tableWidget->verticalHeader()->setVisible(false);
	ui->tableWidget->setAlternatingRowColors(true);
	setTableHeader();
}
void CChannelPlanWidget::setTableHeader() const
{
	if (QString::fromStdString(systemConfig.GetSystemCfg().Common.Language) == "zh-CN")
	{
        const QStringList headerList = { QString::fromLocal8Bit("通道号"),QString::fromLocal8Bit("大脑半球"), QString::fromLocal8Bit("脑叶"),
								QString::fromLocal8Bit("脑区"), QString::fromLocal8Bit("探头型号"), QString::fromLocal8Bit("采样率(kHz)"),
								QString::fromLocal8Bit("低切(Hz)"), QString::fromLocal8Bit("高切(Hz)"), QString::fromLocal8Bit("陷波(Hz)"),QString::fromLocal8Bit("信号类型") };
	
		ui->tableWidget->setHorizontalHeaderLabels(headerList);
	}
	else
	{
        const QStringList headerList = { QString::fromLocal8Bit("Index"), QString::fromLocal8Bit("Brain Hemisphere"), QString::fromLocal8Bit("Lobe"),
	  QString::fromLocal8Bit("Area"), QString::fromLocal8Bit("Base Type"), QString::fromLocal8Bit("Sampling Rate(kHz)"),
							   QString::fromLocal8Bit("Low Cut(Hz)"), QString::fromLocal8Bit("High Cut(Hz)"), QString::fromLocal8Bit("Notch(Hz)"),QString::fromLocal8Bit("Signal Type") };
		ui->tableWidget->setHorizontalHeaderLabels(headerList);
	}
	ui->tableWidget->horizontalHeader()->setStyleSheet("QHeaderView::section {"
		"   width: 156;"
		"   height: 36px;"
		"   background: #283c82;"
		"   font-size: 16px;"
		"   font-family: 'PingFangSC-Semibold', 'PingFang SC Semibold', 'PingFang SC', sans-serif, 'PingFangSC-Semibold', 'PingFang SC Semibold', 'PingFang SC', sans-serif-650;"
		"   font-weight: bold;"
		"   color: #ffffff;"
		"   line-height: 53px;"
		" border: none;"
		"   border-bottom: 0.5px solid gray;"
		" text-align: left; "
		"}"
		"QHeaderView::section::disabled  {"
		"    background: #283c82;"
		"}");
	// 合并样式表，包括隔行换色和表格的其他样式
    const QString styleSheet = "QTableWidget::item:alternate { background-color: #172b73; }"
		"QTableWidget::item:alternate:selected { background-color: rgba(54, 157, 251, 1);}"
		"QTableWidget::item:selected { background-color: rgba(54, 157, 251, 1);}"
		"QTableView::item { height: 54px; }"
		"QTableWidget { background: #283c82; border-radius:4px; font-size: 16px; color: #ffffff; text-align: left;font-family: 'PingFangSC-Regular', 'PingFang SC', sans-serif, 'PingFangSC-Regular', 'PingFang SC', sans-serif-400;    border: 1px solid gray;}";

	ui->tableWidget->setStyleSheet(styleSheet);
}

void CChannelPlanWidget::setBrainHemisphere()
{
	brainHemisphere.clear();
	detectorType.clear();
	for (const auto& i : allMapInfo)
	{
		detectorType.append(i.first);
	}

	if (QString::fromStdString(systemConfig.GetSystemCfg().Common.Language) == "zh-CN")
	{
		brainHemisphere << kHemisphereNames_CN[0] << kHemisphereNames_CN[1];
	}
	else
	{
		brainHemisphere << kHemisphereNames[0] << kHemisphereNames[1];
	}
}

void CChannelPlanWidget::setLobe(const int hemisphereIndex)
{
    const auto atlasCfg = brainAtlasCfg.getAtlas();
	auto lobeConfig = atlasCfg.LeftHemisphere;
	if(hemisphereIndex == 1)
	{
		lobeConfig = atlasCfg.RightHemisphere;
	}
	lobeType.clear();
	if (QString::fromStdString(systemConfig.GetSystemCfg().Common.Language) == "zh-CN")
	{
		for(auto lobe : lobeConfig)
		{
			lobeType << QString::fromStdString(lobe.ChineseName);
		}
	}
	else
	{
		for (auto lobe : lobeConfig)
		{
			lobeType << QString::fromStdString(lobe.Name);
		}
	}
}

void CChannelPlanWidget::setArea(const int hemisphereIndex, const int lobeIndex)
{
	const auto atlasCfg = brainAtlasCfg.getAtlas();
	areaType.clear();

	std::vector<BrainLobeInfo> hemisphereCfg;
	if(hemisphereIndex == 0) // 左脑
	{
		hemisphereCfg = atlasCfg.LeftHemisphere;
	}
	else
	{
		hemisphereCfg = atlasCfg.RightHemisphere;
	}

	auto lobeCfg = hemisphereCfg[lobeIndex];
	if(QString::fromStdString(systemConfig.GetSystemCfg().Common.Language) == "zh-CN")
	{
		for (auto area : lobeCfg.Areas)
		{
			areaType << QString::fromStdString(area.ChineseName);
		}
	}
	else
	{
		for (auto area : lobeCfg.Areas)
		{
			areaType << QString::fromStdString(area.Name);
		}
	}
}

void CChannelPlanWidget::initTable()
{
	ui->savebtn->setEnabled(false);
	setBrainHemisphere();
	channelPlanVec = channelService->getChannelPlanByPatient(currentPatientUid);
	if (channelPlanVec.empty())
	{
		return;
	}
	else
	{
		currentFpgaPort.clear();
		for (int& i : fpgaList)
        {
            i = 0;
		}
		for (auto& i : channelPlanVec)
        {
            const int index = fpgaPort.indexOf(QString::fromStdString(i.Indexs));
			if (index != -1 )
			{
				fpgaList.replace(index, 1);
			}
		}
		for (int j = 0; j < fpgaList.size(); j++)
		{
			if (fpgaList.at(j) == 0)
			{
				currentFpgaPort.append(fpgaPort.at(j));
			}
		}
		for (auto& plan : channelPlanVec)
        {
            const int rowCount = ui->tableWidget->rowCount();
			ui->tableWidget->insertRow(rowCount);
            const auto fpgaComboBox = new QComboBox(this);
			fpgaComboBox->setFixedWidth(60);
			QStringList thisRowFpgaPort = currentFpgaPort;
			thisRowFpgaPort.append(QString::fromStdString(plan.Indexs));
			fpgaComboBox->addItems(thisRowFpgaPort);
			fpgaComboBox->setCurrentText(QString::fromStdString(plan.Indexs));
			connect(fpgaComboBox, &QComboBox::currentTextChanged, this, [=]() {
				changeCurrentFpgaPort(fpgaComboBox->currentIndex());
				if (fpgaComboBox->currentText()== QString::fromStdString(plan.Indexs))
				{
					ui->savebtn->setEnabled(false);
					saveState = 0;
				}
				else
				{
					ui->savebtn->setEnabled(true);
					saveState = 1;
				}
			
				});
			ui->tableWidget->setCellWidget(rowCount, 0, fpgaComboBox);
            const auto fpgaListView = new QListView();
			fpgaComboBox->setView(fpgaListView);
			fpgaComboBox->setStyleSheet(styleSheet);
            const auto hemisphereComboBox = new QComboBox(this);
            const auto lobeComboBox = new QComboBox(this);
            const auto areaComboBox = new QComboBox(this);
			hemisphereComboBox->setFixedWidth(120);
			lobeComboBox->setFixedWidth(150);
			areaComboBox->setFixedWidth(150);
			hemisphereComboBox->addItems(brainHemisphere);
			hemisphereComboBox->setCurrentText(getHemisphereTrName(plan));
			setLobe(hemisphereComboBox->currentIndex());
            const auto toponymListView = new QListView();
			hemisphereComboBox->setView(toponymListView);
			hemisphereComboBox->setStyleSheet(styleSheet);
			ui->tableWidget->setCellWidget(rowCount, 1, hemisphereComboBox);
			connect(hemisphereComboBox, &QComboBox::currentTextChanged, this, [=]() {
				setLobe(hemisphereComboBox->currentIndex());
				lobeComboBox->clear();
				lobeComboBox->addItems(lobeType);
				if (hemisphereComboBox->currentText() == getHemisphereTrName(plan))
				{
					ui->savebtn->setEnabled(false);
					saveState = 0;
				}
				else
				{
					ui->savebtn->setEnabled(true);
					saveState = 1;
				}
			
				});
			lobeComboBox->addItems(lobeType);
			lobeComboBox->setCurrentText(getLobeTrName(plan));
            const auto funZoneListView = new QListView();
			lobeComboBox->setView(funZoneListView);
			lobeComboBox->setStyleSheet(styleSheet);
			ui->tableWidget->setCellWidget(rowCount, 2, lobeComboBox);

			connect(lobeComboBox, &QComboBox::currentTextChanged, this, [=]() {
				setArea(hemisphereComboBox->currentIndex(), lobeComboBox->currentIndex() < 0 ? 0 : lobeComboBox->currentIndex() );
				areaComboBox->clear();
				areaComboBox->addItems(areaType);
				//areaComboBox->setCurrentText(getAreaTrName(plan));
				if (lobeComboBox->currentText()== getLobeTrName(plan))
				{
					ui->savebtn->setEnabled(false);
					saveState = 0;
				}
				else
				{
					ui->savebtn->setEnabled(true);
					saveState = 1;
				}	
			
				}
				);
		

			setArea(hemisphereComboBox->currentIndex(), lobeComboBox->currentIndex());
			connect(areaComboBox, &QComboBox::currentTextChanged, this, [=]() {
				if (areaComboBox->currentText() == getAreaTrName(plan))
				{
					ui->savebtn->setEnabled(false);
					saveState = 0;
				}
				else
				{
					ui->savebtn->setEnabled(true);
					saveState = 1;
				}
				});
			areaComboBox->addItems(areaType);
            const auto areaListView = new QListView();
			areaComboBox->setView(areaListView);
			areaComboBox->setStyleSheet(styleSheet);
			areaComboBox->setCurrentText(getAreaTrName(plan));
			ui->tableWidget->setCellWidget(rowCount, 3, areaComboBox);
            const auto baseTypeComboBox = new QComboBox(this);
			baseTypeComboBox->addItems(detectorType);
			connect(baseTypeComboBox, &QComboBox::currentTextChanged, this, [=]() {


				if (baseTypeComboBox->currentText() == QString::fromStdString(plan.DetectorType))
				{
					ui->savebtn->setEnabled(false);
					saveState = 0;
				}
				else
				{
					ui->savebtn->setEnabled(true);
					saveState = 1;
				}
				});
			
			baseTypeComboBox->setFixedWidth(80);
            const auto baseTypeListView = new QListView();
			baseTypeComboBox->setView(baseTypeListView);
			ui->tableWidget->setCellWidget(rowCount, 4, baseTypeComboBox);
			baseTypeComboBox->setStyleSheet(styleSheet);
			baseTypeComboBox->setCurrentText(QString::fromStdString(plan.DetectorType));
            const auto samplingRateComboBox = new QComboBox(this);
            const auto samplingRateListView = new QListView();
			samplingRateComboBox->setView(samplingRateListView);
			samplingRateComboBox->addItems(samplingRate);
			ui->tableWidget->setCellWidget(rowCount, 5, samplingRateComboBox);
			samplingRateComboBox->setStyleSheet(styleSheet);
			samplingRateComboBox->setFixedWidth(80);

			samplingRateComboBox->setCurrentText(QString::number(plan.SampleRate));
            const auto lowCutToolTip = new QLineEdit(QString::number(plan.LowCut));
			lowCutToolTip->setPlaceholderText(QString::fromLocal8Bit("0-%1").arg(QString::number((plan.SampleRate / 2)-1)));
			lowCutToolTip->setToolTip(QString::fromLocal8Bit("0-%1").arg(QString::number((plan.SampleRate / 2) - 1)));
			connect(lowCutToolTip, &QLineEdit::editingFinished, this, [=]() {
				if (lowCutToolTip->text() != QString::number(plan.LowCut))
				{
					ui->savebtn->setEnabled(true);
					saveState = 1;

				}
				else
				{
					ui->savebtn->setEnabled(false);
					saveState = 0;
				}});
			lowCutToolTip->setStyleSheet(lienStyleSheet);
			lowCutToolTip->setValidator(validator);
			ui->tableWidget->setCellWidget(rowCount, 6, lowCutToolTip);
            const auto highCutToolTip = new QLineEdit((QString::number(plan.HighCut)));
			highCutToolTip->setValidator(validator);
			highCutToolTip->setPlaceholderText(QString::fromLocal8Bit("0-%1").arg((QString::number((plan.SampleRate / 2)-1))));
			highCutToolTip->setToolTip(QString::fromLocal8Bit("0-%1").arg(QString::number((plan.SampleRate / 2) - 1)));
			highCutToolTip->setStyleSheet(lienStyleSheet);
			connect(highCutToolTip, &QLineEdit::editingFinished, this, [=]() {
				if (highCutToolTip->text() != QString::number(plan.HighCut))
				{
					ui->savebtn->setEnabled(true);
					saveState = 1;

				}
				else
				{
					ui->savebtn->setEnabled(false);
					saveState = 0;
				}
				});
			ui->tableWidget->setCellWidget(rowCount, 7, highCutToolTip);
            const auto notchComboBox = new QComboBox(this);
            const auto notchListView = new QListView();
			notchComboBox->setView(notchListView);
			notchComboBox->addItems(notch);
			ui->tableWidget->setCellWidget(rowCount, 8, notchComboBox);
			notchComboBox->setStyleSheet(styleSheet);
			notchComboBox->setCurrentText(QString::fromStdString(plan.Notch));
			notchComboBox->setFixedWidth(80);
			connect(notchComboBox, &QComboBox::currentTextChanged, this, [=]() {
				if (notchComboBox->currentText() == QString::fromStdString(plan.Notch))
				{
					ui->savebtn->setEnabled(false);
					saveState = 0;
				}
				else
				{
					ui->savebtn->setEnabled(true);
					saveState = 1;
				}
				});
            const auto signalTypeComboBox = new QComboBox(this);
            const auto signalTypeistView = new QListView();
			signalTypeComboBox->setView(signalTypeistView);
			signalTypeComboBox->addItems(signalType);
			signalTypeComboBox->setFixedWidth(80);
			ui->tableWidget->setCellWidget(rowCount, 9, signalTypeComboBox);
			signalTypeComboBox->setStyleSheet(styleSheet);
			signalTypeComboBox->setCurrentText(QString::fromStdString(plan.SignalType));

			connect(signalTypeComboBox, &QComboBox::currentTextChanged, this, [=]() {
				if (signalTypeComboBox->currentText() == QString::fromStdString(plan.SignalType))
				{
					ui->savebtn->setEnabled(false);
					saveState = 0;
				}
				else
				{
					ui->savebtn->setEnabled(true);
					saveState = 1;
				}
				});
            const auto channelCounts = new QTableWidgetItem(QString::number(plan.ChannelCounts));
			connect(baseTypeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeCurrentChanelCounts(int)));
			ui->tableWidget->setItem(rowCount, 10, channelCounts);
			connect(samplingRateComboBox, &QComboBox::currentTextChanged, this, [=]() {
                const auto lowItem = qobject_cast<QLineEdit*>(ui->tableWidget->cellWidget(ui->tableWidget->currentRow(), 6));
				lowItem->setPlaceholderText(QString::fromLocal8Bit("0.1-%1").arg(QString::number(samplingRateComboBox->currentText().toInt() / 2 - 5)));
				lowItem->setToolTip(QString::fromLocal8Bit("0.1-%1").arg(QString::number(samplingRateComboBox->currentText().toInt() / 2 - 5)));
				if (samplingRateComboBox->currentText() == (QString::number(plan.SampleRate)))
				{
					ui->savebtn->setEnabled(false);
					saveState = 0;
				}
				else
				{
					ui->savebtn->setEnabled(true);
					saveState = 1;
				}
                const auto highItem = qobject_cast<QLineEdit*>(ui->tableWidget->cellWidget(ui->tableWidget->currentRow(), 7));
				highItem->setPlaceholderText(QString::fromLocal8Bit("0.1-%1").arg(QString::number(samplingRateComboBox->currentText().toInt()  / 2 - 5)));
				highItem->setToolTip(QString::fromLocal8Bit("0.1-%1").arg(QString::number(samplingRateComboBox->currentText().toInt()  / 2 - 5)));


				});
		}

	}
	ui->tableWidget->setCurrentCell(0, 4);
}
void CChannelPlanWidget::insertRow()
{
	if (ui->tableWidget->rowCount()> fpgaList.size()-1)
	{
		return;
	}
	ui->savebtn->setEnabled(true);
	saveState = 1;
	currentFpgaPort.clear();
	for (int& i : fpgaList)
    {
        i = 0;
	}
    const int rowCounts = ui->tableWidget->rowCount();
	for (int i = 0; i < rowCounts; i++)
	{
        const auto currentFpgaCombobox = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(i, 0));
        const int index = fpgaPort.indexOf(currentFpgaCombobox->currentText());
		if (index != -1)
		{
			fpgaList.replace(index, 1);
		}
	}
	for (int j = 0; j < fpgaList.size(); j++)
	{
		if (fpgaList.at(j) == 0)
		{
			currentFpgaPort.append(fpgaPort.at(j));
		}
	}
    const auto fpgaComboBox = new QComboBox(this);
    const auto fpgaListView = new QListView();
	fpgaComboBox->setView(fpgaListView);
	fpgaComboBox->setFixedWidth(60);
    const QStringList thisRowFpgaPort = currentFpgaPort;
	fpgaComboBox->addItems(thisRowFpgaPort);
	connect(fpgaComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeCurrentfpgaPort(int)));
    const auto rowCount = ui->tableWidget->rowCount();
	ui->tableWidget->insertRow(rowCount);
	ui->tableWidget->setCellWidget(rowCount, 0, fpgaComboBox);
	fpgaComboBox->setStyleSheet(styleSheet);

    const auto hemisphereComboBox = new QComboBox(this);
	hemisphereComboBox->setFixedWidth(120);
    const auto hemisphereListView = new QListView();
	hemisphereComboBox->setView(hemisphereListView);
	hemisphereComboBox->setStyleSheet(styleSheet);
	hemisphereComboBox->setFixedWidth(150);
	ui->tableWidget->setCellWidget(rowCount, 1, hemisphereComboBox);
	LOG("hemisphereComboBox is inserted.");

	const auto lobeComboBox = new QComboBox(this);
	lobeComboBox->setFixedWidth(150);
	const auto lobeListView = new QListView();
	lobeComboBox->setView(lobeListView);
	lobeComboBox->setStyleSheet(styleSheet);
	ui->tableWidget->setCellWidget(rowCount, 2, lobeComboBox);
	LOG("lobeComboBox is inserted.");
	
    const auto areaComboBox = new QComboBox(this);
	areaComboBox->setFixedWidth(150);
	areaComboBox->setStyleSheet(styleSheet);
	const auto areaListView = new QListView();
	areaComboBox->setView(areaListView);
	ui->tableWidget->setCellWidget(rowCount, 3, areaComboBox);
	LOG("areaComboBox is inserted.");

	connect(hemisphereComboBox, SIGNAL(currentIndexChanged(int)), SLOT(slotHemisphereChanged(int)));
	hemisphereComboBox->addItems(brainHemisphere); // 添加项目，并触发currentIndexChanged事件

    const auto baseTypeComboBox = new QComboBox(this);
	baseTypeComboBox->addItems(detectorType);
	baseTypeComboBox->setFixedWidth(80);
    const auto baseTypeListView = new QListView();
	baseTypeComboBox->setView(baseTypeListView);
	ui->tableWidget->setCellWidget(rowCount, 4, baseTypeComboBox);
	baseTypeComboBox->setStyleSheet(styleSheet);
    const auto samplingRateComboBox = new QComboBox(this);
	samplingRateComboBox->addItems(samplingRate);
    const auto samplingRateListView = new QListView();
	samplingRateComboBox->setView(samplingRateListView);
	ui->tableWidget->setCellWidget(rowCount, 5, samplingRateComboBox);
	samplingRateComboBox->setStyleSheet(styleSheet);
	samplingRateComboBox->setFixedWidth(80);
    const auto lowCutToolTip = new QLineEdit(QString::number(0.5));
	lowCutToolTip->setPlaceholderText(QString::fromLocal8Bit("0.1-499"));
	lowCutToolTip->setStyleSheet(lienStyleSheet);
	lowCutToolTip->setValidator(validator);
	ui->tableWidget->setCellWidget(rowCount, 6, lowCutToolTip);
    const auto highCutToolTip = new QLineEdit((QString::number(499)));
	highCutToolTip->setValidator(validator);
	highCutToolTip->setPlaceholderText(QString::fromLocal8Bit("0.1-499"));
	ui->tableWidget->setCellWidget(rowCount, 7, highCutToolTip);
	highCutToolTip->setStyleSheet(lienStyleSheet);

	connect(samplingRateComboBox, &QComboBox::currentTextChanged, this, [=]() {
	    const auto lowItem = qobject_cast<QLineEdit*>(ui->tableWidget->cellWidget(ui->tableWidget->currentRow(), 6));
		lowItem->setPlaceholderText(QString::fromLocal8Bit("0.1-%1").arg(QString::number(samplingRateComboBox->currentText().toInt() / 2 - 5)));
		lowItem->setToolTip(QString::fromLocal8Bit("0.1-%1").arg(QString::number(samplingRateComboBox->currentText().toInt()/ 2 - 5)));

	    const auto highItem = qobject_cast<QLineEdit*>(ui->tableWidget->cellWidget(ui->tableWidget->currentRow(), 7));
		highItem->setPlaceholderText(QString::fromLocal8Bit("0.1-%1").arg(QString::number(samplingRateComboBox->currentText().toInt() / 2 - 5)));
		highItem->setToolTip(QString::fromLocal8Bit("0.1-%1").arg(QString::number(samplingRateComboBox->currentText().toInt()  / 2 - 5)));
		}
		);

    const auto notchComboBox = new QComboBox(this);
	notchComboBox->addItems(notch);
    const auto notchlistView = new QListView();
	notchComboBox->setView(notchlistView);
	ui->tableWidget->setCellWidget(rowCount, 8, notchComboBox);
	notchComboBox->setStyleSheet(styleSheet);
	notchComboBox->setFixedWidth(80);
    const auto signalTypeComboBox = new QComboBox(this);
    const auto signalTypelistView = new QListView();
	signalTypeComboBox->setView(signalTypelistView);
	signalTypeComboBox->addItems(signalType);
	ui->tableWidget->setCellWidget(rowCount, 9, signalTypeComboBox);
	signalTypeComboBox->setStyleSheet(styleSheet);
	signalTypeComboBox->setFixedWidth(80);

    const auto channelCounts = new QTableWidgetItem();
	channelCounts->setText(QString::number(allMapInfo.at(baseTypeComboBox->currentIndex()).second));
	connect(baseTypeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeCurrentChanelCounts(int)));
	// 将 QTableWidgetItem 添加到表格的指定位置
	ui->tableWidget->setItem(rowCount, 10, channelCounts);
	ui->tableWidget->setCurrentCell(rowCount, 0); // 选中指定行的第0列
}

void CChannelPlanWidget::slotHemisphereChanged(int index)
{
	const auto rowCount = ui->tableWidget->rowCount() - 1;
    const auto lobeComboBox = dynamic_cast<QComboBox*>(ui->tableWidget->cellWidget(rowCount, 2));
	if(!lobeComboBox)
	{
		LOG_ERR(std::format("Can not find lobe combobox, row count: {}", rowCount));
	}

	disconnect(lobeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotLobeChanged(int)));
	lobeComboBox->clear();
	connect(lobeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotLobeChanged(int)));

    setLobe(index);
	lobeComboBox->addItems(lobeType);
}

void CChannelPlanWidget::slotLobeChanged(int index)
{
	const auto rowCount = ui->tableWidget->rowCount() - 1;
    const auto hemisphereComboBox = dynamic_cast<QComboBox*>(ui->tableWidget->cellWidget(rowCount, 1));
    const auto lobeComboBox = dynamic_cast<QComboBox*>(ui->tableWidget->cellWidget(rowCount, 2));
	const auto areaComboBox = dynamic_cast<QComboBox*>(ui->tableWidget->cellWidget(rowCount, 3));

	disconnect(areaComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotAreaChanged(int)));
	areaComboBox->clear();
	connect(areaComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotAreaChanged(int)));
	
	setArea(hemisphereComboBox->currentIndex(), lobeComboBox->currentIndex());
	areaComboBox->addItems(areaType);
}

void CChannelPlanWidget::slotAreaChanged(int index)
{
	// TODO
}

void CChannelPlanWidget::slotUpdateDetectorTransform(QString planIndex, QVector3D translate, QVector3D rotate)
{
	for(auto plan : channelPlanVec)
	{
	    if(plan.Indexs == planIndex.toStdString())
	    {
			plan.DetectorTranslateX = translate.x();
			plan.DetectorTranslateY = translate.y();
			plan.DetectorTranslateZ = translate.z();

			plan.DetectorRotateX = rotate.x();
			plan.DetectorRotateY = rotate.y();
			plan.DetectorRotateZ = rotate.z();
	    }
	}
}

void CChannelPlanWidget::slotUpdateProbeTransform(QString planIndex, QVector3D translate, QVector3D rotate)
{
	std::vector<ChannelPlan> planToUpdate;
	for (auto plan : channelPlanVec)
	{
		if (plan.Indexs == planIndex.toStdString())
		{
			plan.ProbeTranslateX = translate.x();
			plan.ProbeTranslateY = translate.y();
			plan.ProbeTranslateZ = translate.z();

			plan.ProbeRotateX = rotate.x();
			plan.ProbeRotateY = rotate.y();
			plan.ProbeRotateZ = rotate.z();
			planToUpdate.push_back(plan);
			
		}
	}
	channelService->updateChannelPlan(planToUpdate);
}

QString CChannelPlanWidget::getNextUppercaseLetter(QString currentLetter)
{
	if (currentLetter >= "A" && currentLetter < "Z") {
		// 如果当前字母在 'A' 到 'Y' 之间，返回下一个字母
        const QChar nextLetter = currentLetter.at(0).toLatin1() + 1;
		return QString(nextLetter);
	}
	else if (currentLetter == "Z") {
		// 如果当前字母是 'Z'，则返回 'A'
		return "A";
	}
	else {
		// 如果输入不是大写英文字母，返回原始输入
		return currentLetter;
	}
}
void  CChannelPlanWidget::deleteChannelPlan()
{
    const auto row = ui->tableWidget->currentRow();
	if (row<0)
	{
		return;
	}
    const auto fpgaComboBox = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(row, 0));
    const auto plan = channelService->getChannelPlanByIndexs(currentPatientUid, fpgaComboBox->currentText());
	if (plan.UID.empty())
	{
		ui->tableWidget->removeRow(row);
		return;
	}

	CNxMessageBox msgBox(QString::fromLocal8Bit(""), QString::fromLocal8Bit("删除后不能恢复!"));
	msgBox.exec();
    const bool confirmed = msgBox.isConfirmed();
	if (confirmed)
	{
        const auto currentRow = ui->tableWidget->currentRow();
        const auto currentFpgaComboBox = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(currentRow, 0));
		if (currentFpgaComboBox->currentText() != "") {
            const QString data = currentFpgaComboBox->currentText();
			channelService->deleteChannelPlanByIndexs(data,currentPatientUid);
			brainArea->removeDetectorAndProbe(QString::fromStdString(plan.Indexs));
			ui->tableWidget->removeRow(row);
		}
	}
}

void CChannelPlanWidget::saveChannelPlan()
{
	channelPlanVec = channelService->getChannelPlanByPatient(currentPatientUid);
    const QDateTime currentDateTime = QDateTime::currentDateTime();
    const QString formattedDateTime = currentDateTime.toString("yyyy-MM-dd hh:mm:ss");
    const auto num = ui->tableWidget->rowCount();
	// 只有当行数大于1时才添加 
	if (num >= 1 && num < fpgaList.size() + 1)
	{
        std::vector<ChannelPlan> currentChannelVec;
        for (int i = 0; i < ui->tableWidget->rowCount(); i++)
		{
			ChannelPlan plan;
			if (i < channelPlanVec.size())
			{
				plan = channelPlanVec[i];
			}
            const auto fpgaComboBox = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(i, 0));
			plan.Indexs = fpgaComboBox->currentText().toStdString();

            plan.Position = getPositionByUI(i);
			plan.Area = getLobeByUI(i);
			plan.AreaNum = getAreaByUI(i);

            if (const auto baseTypeComboBox = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(i, 4))) {
				plan.DetectorType = baseTypeComboBox->currentText().toStdString();
			}

            const auto samplingRateComboBox = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(i, 5));
			if (samplingRateComboBox) {
				plan.SampleRate = samplingRateComboBox->currentText().toInt();
			}

			connect(samplingRateComboBox, &QComboBox::currentTextChanged, this, [=]() {
                        const auto lowItem = qobject_cast<QLineEdit*>(ui->tableWidget->cellWidget(ui->tableWidget->currentRow(), 6));
				lowItem->setPlaceholderText(QString::fromLocal8Bit("0.1-%1").arg(QString::number(samplingRateComboBox->currentText().toInt()  / 2 - 5)));
				lowItem->setToolTip(QString::fromLocal8Bit("0.1-%1").arg(QString::number(samplingRateComboBox->currentText().toInt() / 2 - 5)));
                        const auto highItem = qobject_cast<QLineEdit*>(ui->tableWidget->cellWidget(ui->tableWidget->currentRow(), 7));
				highItem->setPlaceholderText(QString::fromLocal8Bit("0.1-%1").arg(QString::number(samplingRateComboBox->currentText().toInt() / 2 - 5)));
				highItem->setToolTip(QString::fromLocal8Bit("0.1-%1").arg(QString::number(samplingRateComboBox->currentText().toInt() / 2 - 5)));
				}
			);
            const auto items = qobject_cast<QLineEdit*>(ui->tableWidget->cellWidget(i, 6));
			plan.LowCut = items->text().toFloat();
			if (plan.LowCut < 0.1 || plan.LowCut > plan.SampleRate / 2 - 5)
			{
			
				items->setStyleSheet("border: 0.5px solid red; color: red;background: transparent;QToolTip {  background-color: #172b73; color: white; border: 1px solid red; }");
				items->setPlaceholderText(QString::fromLocal8Bit("0.1-%1").arg(QString::number(plan.SampleRate / 2-1)));
				items->setValidator(validator);
				items->setToolTip(QString::fromLocal8Bit("0.1-%1").arg(QString::number(plan.SampleRate / 2 - 1)));
				ui->tableWidget->setCellWidget(i, 6, items);
				return;
			}
            const auto currentRowItem = qobject_cast<QLineEdit*>(ui->tableWidget->cellWidget(i, 7));
			plan.HighCut = currentRowItem->text().toFloat();
	
			if (plan.HighCut < 0.1 || plan.HighCut >= plan.SampleRate / 2 - 5)
			{
			
				currentRowItem->setStyleSheet("border: 0.5px solid red; color: red;background: transparent;QToolTip { background-color: #172b73; color: white; border: 1px solid red; }");
				currentRowItem->setPlaceholderText(QString::fromLocal8Bit("0.1-%1").arg(QString::number(plan.SampleRate / 2-1)));
				currentRowItem->setValidator(validator);
				currentRowItem->setToolTip(QString::fromLocal8Bit("0.1-%1").arg(QString::number(plan.SampleRate / 2 - 1)));
				ui->tableWidget->setCellWidget(i, 7, currentRowItem);
				return;
			}
			plan.ChannelCounts = ui->tableWidget->item(i, 10)->text().toInt();
            if (const auto notchComboBox = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(i, 8))) {
				plan.Notch = notchComboBox->currentText().toStdString();
			}
            if (const auto signalTypeComboBox = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(i, 9))) {
				plan.SignalType = signalTypeComboBox->currentText().toStdString();
			}

			currentChannelVec.push_back(plan);
		}

	    std::vector<ChannelPlan> updateChannel;
		if (!channelPlanVec.empty())
		{
			// 更新旧的plan
			for (auto& j : currentChannelVec)
            {
				for (auto& i : channelPlanVec)
                {
					
					if (j.UID == i.UID)
					{
		
						if (isUpdate(j, i))
						{
                            j.UID = i.UID;
                            j.CreateDateTime = i.CreateDateTime;
                            j.IsDeleted = 0;
                            j.PatientFK = i.PatientFK;
                            j.UpdateDateTime = formattedDateTime.toStdString();

							if(j.Position != i.Position || j.Area != i.Area || j.AreaNum != i.AreaNum ||j.DetectorType != i.DetectorType)
							{
								setDetectorTransformToPlan(j);
							}

							updateChannel.push_back(j);
							channelService->updateChannelPlan(updateChannel);
							updateChannelName(j, i);
						}
					}
				}
			}

			// 插入新的plan
			if (currentChannelVec.size() > channelPlanVec.size())
			{
				for (int k = static_cast<int>(channelPlanVec.size()); k < currentChannelVec.size(); k++)
				{
					auto cha = currentChannelVec.at(k);
					cha.UID = QUuid::createUuid().toString().remove("{").remove("}").remove("-").toStdString();
					cha.CreateDateTime = formattedDateTime.toStdString();
					cha.IsDeleted = 0;
					cha.PatientFK = currentPatientUid.toStdString();
					cha.UpdateDateTime = formattedDateTime.toStdString();

					setDetectorTransformToPlan(cha);
					
					updateChannel.push_back(cha);
					channelService->updateChannelPlan(updateChannel);
					updateChannelName(cha);
				}
			}
		}
		else
		{
			// 所有plan都为新增plan，直接插入
			for (auto cha : currentChannelVec)
            {
                cha.UID = QUuid::createUuid().toString().remove("{").remove("}").remove("-").toStdString();
				cha.CreateDateTime = formattedDateTime.toStdString();
				cha.IsDeleted = 0;
				cha.PatientFK = currentPatientUid.toStdString();
				cha.UpdateDateTime = formattedDateTime.toStdString();
				setDetectorTransformToPlan(cha);
				updateChannel.push_back(cha);
				channelService->updateChannelPlan(updateChannel);
				updateChannelName(cha);
			}
		}
	
	}
	ui->tableWidget->clearContents();
	ui->tableWidget->setRowCount(0);
	initTable();
	emit signalChannelPlanChanged();
}

bool CChannelPlanWidget::isUpdate(const ChannelPlan& currentChannel, const ChannelPlan& initChannel)
{
	if (currentChannel.Area == initChannel.Area && currentChannel.AreaNum == initChannel.AreaNum
		&& currentChannel.DetectorType == initChannel.DetectorType && currentChannel.HighCut == initChannel.HighCut
		&& currentChannel.LowCut == initChannel.LowCut && currentChannel.Notch == initChannel.Notch
		&& currentChannel.Position == initChannel.Position && currentChannel.SampleRate == initChannel.SampleRate
		&& currentChannel.SignalType == initChannel.SignalType && currentChannel.Indexs == initChannel.Indexs && currentChannel.ChannelCounts == initChannel.ChannelCounts)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void CChannelPlanWidget::changeCurrentFpgaPort(int index)
{

	currentFpgaPort.clear();
	// 将所有元素重置为0
	for (int i = 0; i < fpgaList.size(); ++i) {
		fpgaList[i] = 0;
	}
	QString styleSheet = "QComboBox{ background: transparent; border:none; font-size: 15px; color: #ffffff;font-family: 'PingFangSC-Regular', 'PingFang SC', sans-serif, 'PingFangSC-Regular', 'PingFang SC', sans-serif-400; }"
		"QComboBox::item:selected { background-color: rgba(54, 157, 251, 0.60); }"
		"QComboBox::down-arrow { image: url(:/Optimus/images/trainPage/down.svg); border: none;  width: 8px;height:6px;}"
		"QComboBox::drop-down {  subcontrol-position: center right; border:none;subcontrol-position: 50px; }"
		"QComboBox QAbstractItemView { background-color: #283c82; color: #ffffff; selection-background-color: rgba(54, 157, 251, 0.60); border:none; }"
		"QComboBox::item { background-color: transparent; color: #ffffff; font-size: 15px; font-normal: bold; border:none; }";
    const int rowCount = ui->tableWidget->rowCount();
	for (int i = 0; i < rowCount; i++)
	{
        const auto currentfpgaCombox = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(i, 0));
		;
        const int index = fpgaPort.indexOf(currentfpgaCombox->currentText());
		if (index != -1)
		{
			fpgaList.replace(index, 1);
		}
	}
	for (int j = 0; j < fpgaList.size(); j++)
	{
		if (fpgaList.at(j) == 0)
		{
			currentFpgaPort.append(fpgaPort.at(j));
		}
	}
}
void CChannelPlanWidget::findXML(QString path)
{
    const QDir currentDir(path);
	QStringList xmlFiles = currentDir.entryList(QStringList() << "*.xml", QDir::Files, QDir::Time);
	QString errorMessage;
	foreach(const QString & xmlFile, xmlFiles) {
		if (electrodeSettingInterface->loadFile(path + "//" + xmlFile, errorMessage))
		{
			QPair<QString, int> currentMapInfo;
			currentMapInfo.first = setting->pages[0].ports[0].name;
			currentMapInfo.second = setting->pages[0].ports[0].electrodeSites.size();
			electrodeSites.append(setting->pages[0].ports[0].electrodeSites);
			allMapInfo.append(currentMapInfo);
		}
	}
}
void CChannelPlanWidget::changeCurrentChanelCounts(int index)
{
    const auto currentrow = ui->tableWidget->currentRow();
        QTableWidgetItem* channelCounts = ui->tableWidget->item(currentrow, 10);
        channelCounts->setText(QString::number(allMapInfo.at(index).second));
}

void CChannelPlanWidget::slotBrainAreaInited()
{
    const auto plans = channelService->getChannelPlanByPatient(currentPatientUid);
	for (auto& plan : plans)
	{
        const auto atlasCfg = brainAtlasCfg.getAtlas();
		std::vector<BrainLobeInfo> lobeInfos;
		if (plan.Position == kHemisphereNames[0].toStdString())
		{
			lobeInfos = atlasCfg.LeftHemisphere;
		}
		else
		{
			lobeInfos = atlasCfg.RightHemisphere;
		}

		for (auto lobe : lobeInfos)
		{
			if (lobe.Name == plan.Area)
			{
				for (auto area : lobe.Areas)
				{
					if (area.Name == plan.AreaNum)
					{
						auto detectorPos = area;
						detectorPos.Center[0] = plan.DetectorTranslateX;
						detectorPos.Center[1] = plan.DetectorTranslateY;
						detectorPos.Center[2] = plan.DetectorTranslateZ;
						detectorPos.Rotate[0] = plan.DetectorRotateX;
						detectorPos.Rotate[1] = plan.DetectorRotateY;
						detectorPos.Rotate[2] = plan.DetectorRotateZ;

						auto probePos = area;
						probePos.Center[0] = plan.ProbeTranslateX;
						probePos.Center[1] = plan.ProbeTranslateY;
						probePos.Center[2] = plan.ProbeTranslateZ;
						probePos.Rotate[0] = plan.ProbeRotateX;
						probePos.Rotate[1] = plan.ProbeRotateY;
						probePos.Rotate[2] = plan.ProbeRotateZ;

						brainArea->addDetectorAndProbes(QString::fromStdString(plan.Indexs), detectorPos, probePos, QString::fromStdString(plan.DetectorType));
					}
				}
			}
		}
	}

	connect(brainArea, SIGNAL(signalUpdateDetectorTransform(QString, QVector3D, QVector3D)), this, SLOT(slotUpdateDetectorTransform(QString, QVector3D, QVector3D)));

	connect(brainArea, SIGNAL(signalUpdateProbeTransform(QString, QVector3D, QVector3D)), this, SLOT(slotUpdateProbeTransform(QString, QVector3D, QVector3D)));
}
void CChannelPlanWidget::updateChannelName(ChannelPlan currentChannel, ChannelPlan initChannel)
{
	if (currentChannel.Indexs != initChannel.Indexs || currentChannel.DetectorType != initChannel.DetectorType)
	{
		for (int i = 0; i < allMapInfo.size(); i++)
		{
		
			if (currentChannel.DetectorType== allMapInfo.at(i).first.toStdString())
			{
				channelNameService->deleteChannelBychannelFK(QString::fromStdString(currentChannel.UID));
				std::vector<ChannelName>channelnameVector;
				for (int j = 0; j < electrodeSites.at(i).size(); j++)
				{
					QDateTime currentDateTime = QDateTime::currentDateTime();
					QString formattedDateTime = currentDateTime.toString("yyyy-MM-dd hh:mm:ss");
					ChannelName channelName;
					channelName.UID = QUuid::createUuid().toString().remove("{").remove("}").remove("-").toStdString();
					channelName.CreateDateTime = formattedDateTime.toStdString();
					channelName.IsDeleted = 0;
					channelName.PatientFK = currentPatientUid.toStdString();
					channelName.UpdateDateTime = formattedDateTime.toStdString();
					channelName.ChannelPlanFK = currentChannel.UID;
					channelName.Indexs = currentChannel.Indexs;
					channelName.ChannelCoordX = electrodeSites.at(i).at(j).x;
					channelName.ChannelCoordY = electrodeSites.at(i).at(j).y;
					channelName.ChannelNumber = electrodeSites.at(i).at(j).channelNumber;
					channelName.CustomName = QString::fromStdString(currentChannel.Indexs).toStdString() + QString("-%1").arg(QString::fromStdString(electrodeSites.at(i).at(j).channelNumber).toInt(), 3, 10, QLatin1Char('0')).toStdString();
					
					channelnameVector.push_back(channelName);
				}
				channelNameService->updateChannelName(channelnameVector);
				break;
			}
		}
		
	}

}
void CChannelPlanWidget::updateChannelName(ChannelPlan currentChannel)
{
	for (int i = 0; i < allMapInfo.size(); i++)
	{
		if (currentChannel.DetectorType == allMapInfo.at(i).first.toStdString())
		{
			channelNameService->deleteChannelBychannelFK(QString::fromStdString(currentChannel.UID));
			std::vector<ChannelName>channelnameVector;
			for (int j = 0; j < electrodeSites.at(i).size(); j++)
			{
				ChannelName channelName;
				channelName.ChannelPlanFK = currentChannel.UID;
				channelName.Indexs = currentChannel.Indexs;
				QDateTime currentDateTime = QDateTime::currentDateTime();
				QString formattedDateTime = currentDateTime.toString("yyyy-MM-dd hh:mm:ss");
				channelName.UID = QUuid::createUuid().toString().remove("{").remove("}").remove("-").toStdString();
				channelName.CreateDateTime = formattedDateTime.toStdString();
				channelName.IsDeleted = 0;
				channelName.PatientFK = currentPatientUid.toStdString();
				channelName.UpdateDateTime = formattedDateTime.toStdString();
				channelName.ChannelCoordX = electrodeSites.at(i).at(j).x;
				channelName.ChannelCoordY = electrodeSites.at(i).at(j).y;
				channelName.ChannelNumber = electrodeSites.at(i).at(j).channelNumber;
				channelName.CustomName = QString::fromStdString(currentChannel.Indexs).toStdString() + QString("-%1").arg(QString::fromStdString(electrodeSites.at(i).at(j).channelNumber).toInt(), 3, 10, QLatin1Char('0')).toStdString();
				channelnameVector.push_back(channelName);
			
			}
			channelNameService->updateChannelName(channelnameVector);
			break;
		}
	}

}

QString CChannelPlanWidget::getHemisphereTrName(const ChannelPlan& plan)
{
	if (plan.Position == kHemisphereNames[0].toStdString())
	{
		if (systemConfig.GetSystemCfg().Common.Language == "zh-CN")
		{
			return kHemisphereNames_CN[0];
		}
		else
		{
			return kHemisphereNames[0];
		}
	}
	else
	{
		if (systemConfig.GetSystemCfg().Common.Language == "zh-CN")
		{
			return kHemisphereNames_CN[1];
		}
		else
		{
			return kHemisphereNames[1];
		}
	}
}

QString CChannelPlanWidget::getLobeTrName(const ChannelPlan& plan)
{
	auto hemisphere = brainAtlasCfg.getAtlas().LeftHemisphere;
	if(plan.Position == kHemisphereNames[1].toStdString())
	{
		hemisphere = brainAtlasCfg.getAtlas().RightHemisphere;
	}

	for(auto lobe : hemisphere)
	{
		if(lobe.Name == plan.Area)
		{
			if (systemConfig.GetSystemCfg().Common.Language == "zh-CN")
			{
				return QString::fromStdString(lobe.ChineseName);
			}
			else
			{
				return QString::fromStdString(lobe.Name);
			}
		}
	}
	return {};
}

QString CChannelPlanWidget::getAreaTrName(const ChannelPlan& plan)
{
	auto hemisphere = brainAtlasCfg.getAtlas().LeftHemisphere;
	if (plan.Position == kHemisphereNames[1].toStdString())
	{
		hemisphere = brainAtlasCfg.getAtlas().RightHemisphere;
	}

	for (auto lobe : hemisphere)
	{
		if (lobe.Name == plan.Area)
		{
			for(auto area : lobe.Areas)
			{
			    if(area.Name == plan.AreaNum)
			    {
					if (systemConfig.GetSystemCfg().Common.Language == "zh-CN")
					{
						return QString::fromStdString(area.ChineseName);
					}
					else
					{
						return QString::fromStdString(area.Name);
					}
			    }
			}
			
		}
	}
	return {};
}

std::string CChannelPlanWidget::getPositionByUI(const int row) const
{
	const auto hemisphereComboBox = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(row, 1));
	if(!hemisphereComboBox)
	{
		LOG_ERR("Can not find hemisphere combobox");
	}
	return kHemisphereNames[hemisphereComboBox->currentIndex()].toStdString();
}

std::string CChannelPlanWidget::getLobeByUI(const int row) const
{
	const auto hemisphereComboBox = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(row, 1));
	if (!hemisphereComboBox)
	{
		LOG_ERR("Can not find hemisphere combobox");
	}
	const auto lobeComboBox = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(row, 2));
	if (!lobeComboBox)
	{
		LOG_ERR("Can not find lobe combo box");
	}

	auto hemisphere = brainAtlasCfg.getAtlas().LeftHemisphere;
	if(hemisphereComboBox->currentIndex() == 0)
	{
		hemisphere = brainAtlasCfg.getAtlas().LeftHemisphere;
	}
	else
	{
		hemisphere = brainAtlasCfg.getAtlas().RightHemisphere;
	}

	return hemisphere[lobeComboBox->currentIndex()].Name;
}

std::string CChannelPlanWidget::getAreaByUI(const int row) const
{
	const auto hemisphereComboBox = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(row, 1));
	if (!hemisphereComboBox)
	{
		LOG_ERR("Can not find hemisphere combobox");
	}
	const auto lobeComboBox = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(row, 2));
	if (!lobeComboBox)
	{
		LOG_ERR("Can not find lobe combo box");
	}

	const auto areaComboBox = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(row, 3));
	if (!areaComboBox)
	{
		LOG_ERR("Can not find area combo box");
	}

	auto hemisphere = brainAtlasCfg.getAtlas().LeftHemisphere;
	if (hemisphereComboBox->currentIndex() == 0)
	{
		hemisphere = brainAtlasCfg.getAtlas().LeftHemisphere;
	}
	else
	{
		hemisphere = brainAtlasCfg.getAtlas().RightHemisphere;
	}

	return hemisphere[lobeComboBox->currentIndex()].Areas[areaComboBox->currentIndex()].Name;
}

void CChannelPlanWidget::setDetectorTransformToPlan(ChannelPlan& plan)
{
	plan.AtlasMapType = systemConfig.GetSystemCfg().Common.AtlasType;

    const auto atlasCfg = brainAtlasCfg.getAtlas();
	auto hemisphere = atlasCfg.LeftHemisphere;
	if (plan.Position != "Left Hemisphere")
	{
		hemisphere = atlasCfg.RightHemisphere;
	}
	for (const auto lobe : hemisphere)
	{
		if (lobe.Name == plan.Area)
		{
			for (const auto area : lobe.Areas)
			{
				if(area.Name == plan.AreaNum)
				{
					plan.DetectorTranslateX = area.Center[0];
					plan.DetectorTranslateY = area.Center[1];
					plan.DetectorTranslateZ = area.Center[2];
					plan.DetectorRotateX = area.Rotate[0];
					plan.DetectorRotateY = area.Rotate[1];
					plan.DetectorRotateZ = area.Rotate[2];
					plan.ProbeTranslateX = area.Center[0];
					plan.ProbeTranslateY = area.Center[1];
					plan.ProbeTranslateZ = area.Center[2];
					plan.ProbeRotateX = area.Rotate[0];
					plan.ProbeRotateY = area.Rotate[1];
					plan.ProbeRotateZ = area.Rotate[2];
					brainArea->removeDetectorAndProbe(QString::fromStdString(plan.Indexs));
					brainArea->addDetectorAndProbes(QString::fromStdString(plan.Indexs), area, area, QString::fromStdString(plan.DetectorType));
				}
			}
		}
	}
}

END_NX_NAMESPACE

