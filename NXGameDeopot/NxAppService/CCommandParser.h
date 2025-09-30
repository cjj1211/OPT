#pragma once
#include "nxappservice_global.h"
#include <QObject>
#include "../NxMessage/TransInformation.h"
BEGIN_NX_NAMESPACE
class CInformationProcessing;
class NXAPPSERVICE_EXPORT CCommandParser  : public QObject
{
	Q_OBJECT

public:
	CCommandParser(CInformationProcessing* informationProcessing,QObject *parent);
	~CCommandParser();
	void  receiveMessage(ActionCommand command);
	signals:
	void sendActionCommand(ActionCommand command);


};
END_NX_NAMESPACE
