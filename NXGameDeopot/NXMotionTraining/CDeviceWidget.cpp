#include "CDeviceWidget.h"
#include "../NxMessage/TransInformation.h"
#include "../NxSystemCfg/CSystemCfg.h"
#include  "../NxAppService/CDeviceStatus.h"
#include <QPixmap>
BEGIN_NX_NAMESPACE
CDeviceWidget::CDeviceWidget(CInformationProcessing* informationProcessing,QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	deviceStatus = new CDeviceStatus(informationProcessing, this);
	connect(deviceStatus, &CDeviceStatus::setDeviceStatus, this, &CDeviceWidget::setDeviceStatus);
}

CDeviceWidget::~CDeviceWidget()
{


}
void CDeviceWidget::setDeviceStatus(DeviceStatus deviceStatus)
{
	if (deviceStatus.Master>systemConfig.GetSystemCfg().Common.Master)
	{
		QPixmap pixmapFull(":/NXMotionTraining/images/trainPage/signal-full.svg"); // 
		ui.masterSignal->setPixmap(pixmapFull);

	}
	else
	{
		QPixmap pixmap(":/NXMotionTraining/images/trainPage/signal-2.svg"); // 
		ui.masterSignal->setPixmap(pixmap);
	}

	if (deviceStatus.Outboard > systemConfig.GetSystemCfg().Common.Outboard)
	{
		QPixmap pixmapFull(":/NXMotionTraining/images/trainPage/signal-full.svg"); // 
		ui.outBoardSignal->setPixmap(pixmapFull);
	}
	else
	{
		QPixmap pixmap(":/NXMotionTraining/images/trainPage/signal-2.svg"); // 
		ui.outBoardSignal->setPixmap(pixmap);
	}
}
END_NX_NAMESPACE
