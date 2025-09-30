#include "CDeviceStatus.h"
#include "CInformationProcessing.h"

BEGIN_NX_NAMESPACE
CDeviceStatus::CDeviceStatus(CInformationProcessing* informationProcessing,QObject *parent)
	: QObject(parent)
{

	connect(informationProcessing, &CInformationProcessing::receiveDeviceMessage, this, &CDeviceStatus::receiveDeviceMessage);
}

CDeviceStatus::~CDeviceStatus()
{}
void CDeviceStatus::receiveDeviceMessage(DeviceStatus deviceStatus)
{
	emit setDeviceStatus( deviceStatus);
}
END_NX_NAMESPACE