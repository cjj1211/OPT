#include "CElectrodePreview.h"
#include"CElectrodeMapDataStructure.h"
#include "CElectrodeSettingInterface.h"
#include "ui_CElectrodePreview.h"
#include <QTabWidget>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <qsettings.h>
BEGIN_NX_NAMESPACE
CElectrodePreview::CElectrodePreview(QWidget* parent)
	:QWidget(parent)
	, ui(new Ui_CElectrodePreview())
	, electrodeMapSetting(new ElectrodeMapSettings())
	, currerntElectrodeMapSetting(new ElectrodeMapSettings())
	, pageView(nullptr)
{
	ui->setupUi(this);
	pageTabWidget = new QTabWidget(this);
	pageTabWidget->setTabPosition(QTabWidget::North);
	ui->mainLayout->addWidget(pageTabWidget);
	pageTabWidget->setStyleSheet(
		"QTabWidget::pane { \
        margin: 0px; \
        border: none; \
    } \
    \
    QTabBar::tab { \
        width: 94px; \
        height: 32px; \
        background: rgba(24, 144, 255, 0.0); \
        border: 1px solid #4b68b3; \
        border-radius: 4px; \
        font-size: 16px; \
        font-family: 'PingFangSC-Regular', 'PingFang SC', sans-serif, 'PingFangSC-Regular', 'PingFang SC', sans-serif-400; \
        font-weight: normal; \
        color: rgba(255, 255, 255, 0.85); \
        margin-left: 20px; \
    } \
    \
    QTabBar::tab:selected { \
        background: #1890ff; \
        border-radius: 4px; \
        font-size: 16px; \
        font-family: 'PingFangSC-Regular', 'PingFang SC', sans-serif, 'PingFangSC-Regular', 'PingFang SC', sans-serif-400; \
        font-weight: normal; \
        color: #ffffff; \
        margin-left: 20px; \
    } \
    \
    QTabWidget { \
        background: transparent; \
    }"
	);
	probeMapSettingsInterface = new CElectrodeSettingInterface(electrodeMapSetting);
	setStyleSheet("background: transparent;");
	setWindowTitle(QString::fromLocal8Bit("ChannelMap"));
}
CElectrodePreview::~CElectrodePreview() {
	delete probeMapSettingsInterface;
	delete currerntElectrodeMapSetting;
}
void CElectrodePreview::slotLoad(const QString& filename)
{    // If user clicked 'cancel', then abort.
	if (filename.isEmpty()) return;
	QString errorMessage;
	const bool loadSuccess = probeMapSettingsInterface->loadFile(filename, errorMessage); // Specify that this XML parsing is probe-map specific.
	if (!loadSuccess) {
		return;
	}
	disconnect(pageTabWidget, SIGNAL(currentChanged(int)), this, SLOT(slotPageChanged(int)));
	currentPage = -1;
	connect(pageTabWidget, SIGNAL(currentChanged(int)), this, SLOT(slotPageChanged(int)));
	slotPageChanged(0);
	//update();
}
void CElectrodePreview::setCurrentTab(int currentTab)
{
	pageTabWidget->setCurrentIndex(currentTab);
}
void CElectrodePreview::slotPageChanged(int index)
{
	currentPage = index;
	auto thisPage = dynamic_cast<PageView*>(pageTabWidget->widget(currentPage));
	if (thisPage==nullptr)
	{
		return;
	}
	connect(thisPage, &PageView::signalShowCurrentChannelName, this, [this](int currentButtonX, int currentButtonY, QString channelNumber) {
		emit signalShowCurrentChannelName(currentButtonX, currentButtonY, channelNumber, QString::number(currentPage));
	
		});

	connect(thisPage, &PageView::signalUpdateCurrentChannelName, this, [this](int currentButtonX, int currentButtonY, QString channelNumber) {
		emit signalUpdateCurrentChannelName(currentButtonX, currentButtonY, QString::number(currentPage), channelNumber.toInt());
		auto Page = dynamic_cast<PageView*>(pageTabWidget->widget(currentPage));
		Page->currentButtonCoustName = currentButtonName;
		});
	if (thisPage && thisPage->num > 0)
	{
		thisPage->buttonClick();
		thisPage->buttonStyle = allButtonStyle;
		thisPage->setbuttonStyle(allButtonStyle);
		pageTabWidget->setCurrentIndex(currentPage);
		thisPage->impedanceValue = impedanceValue;
	}
}
void CElectrodePreview::populateTabWidget(const  QPair <QString, QString>& channelMap)
{
	
	xmlFilePath = R"(../config/ElectrodeMapConfig/%1.xml)";
	LOG_INFO(" lod   xmlFilePath...................");
	slotLoad(xmlFilePath.arg(channelMap.second));
	LOG_INFO(" end   xmlFilePath...................");
	constexpr ViewMode viewMode = DefaultView;
	LOG_INFO(" new   PageView...................");
	auto singlePage = new PageView(electrodeMapSetting, 0, viewMode, this);
	LOG_INFO(" end new   PageView...................");
	Group currentpage = singlePage->page->ports[0];
	allChannel.append(currentpage);
	LOG_INFO("    pageTabWidget   addTab...................");
	pageTabWidget->addTab(singlePage, channelMap.first + "(" + electrodeMapSetting->pages[0].ports[0].name + ")");
	LOG_INFO("    pageTabWidget   end addTab...................");
}
void CElectrodePreview::clearTabWidget() const
{
	for (int page = 0; page < pageTabWidget->count(); page++) {
		delete pageTabWidget->widget(page);
	}
	pageTabWidget->clear();
}
bool CElectrodePreview::searchChannelMap(const QString& channnelIndex, QPair<int, int>position)
{
	int tabIndex = 0;
	for (int i = 0; i < pageTabWidget->count(); ++i) {
		QString tabText = pageTabWidget->tabText(i);
		if (tabText.contains(channnelIndex)) {
			tabIndex = i;
			break;
		}
	}
	pageTabWidget->setCurrentIndex(tabIndex);
	currentPage = tabIndex;
	const auto thisPage = dynamic_cast<PageView*>(pageTabWidget->widget(currentPage));
	if (thisPage->searchChannelMap(position))
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool CElectrodePreview::searchChannelMap(const QString& channnelIndex, const QString& customname)
{
	int tabIndex = 0;
	for (int i = 0; i < pageTabWidget->count(); ++i) {
		QString tabText = pageTabWidget->tabText(i);
		if (tabText.contains(channnelIndex)) {
			tabIndex = i;
			break;
		}
	}
	pageTabWidget->setCurrentIndex(tabIndex);
	currentPage = tabIndex;
	const auto thisPage = dynamic_cast<PageView*>(pageTabWidget->widget(currentPage));
	if (thisPage->searchChannelMap(customname))
	{
		return true;
	}
	else
	{
		return false;
	}
}
void CElectrodePreview::setButtonStyle(QMap<QString, QColor> buttonStyle)
{
	const auto thisPage = dynamic_cast<PageView*>(pageTabWidget->widget(currentPage));
	thisPage->currentPageName = pageTabWidget->tabText(currentPage);
	thisPage->changeView(ViewMode::LocationView);
	allButtonStyle = buttonStyle;
	thisPage->buttonStyle = allButtonStyle;
	thisPage->setbuttonStyle(allButtonStyle);
}
int CElectrodePreview::getCurrentTabIndex()
{
	return currentPage;
}

void CElectrodePreview::setImpedanceValue(QMap<QString, QString> ImpedanceValue)
{
	const auto thisPage = dynamic_cast<PageView*>(pageTabWidget->widget(currentPage));
	thisPage->currentPageName = pageTabWidget->tabText(currentPage);
	thisPage->changeView(ViewMode::LocationView);
	impedanceValue = ImpedanceValue;
	thisPage->impedanceValue = impedanceValue;
}
END_NX_NAMESPACE
