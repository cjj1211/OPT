#pragma once
#include "nxappservice_global.h"
#include "../NxMessage/TransInformation.h"

#include <QObject>

BEGIN_NX_NAMESPACE
class CInformationProcessing;
class NXAPPSERVICE_EXPORT CDeviceStatus  : public QObject
{
	Q_OBJECT

public:
	CDeviceStatus(CInformationProcessing* informationProcessing,QObject *parent);
	~CDeviceStatus();
	void receiveDeviceMessage(DeviceStatus deviceStatus);
signals:
	void setDeviceStatus(DeviceStatus deviceStatus);
};
END_NX_NAMESPACE