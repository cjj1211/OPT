/******************************************************************************
 *  版权所有（C）2022-2024，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *  创建日期:   3月 2024
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/

#pragma once
#include "nxappservice_global.h"
#include "../NxMessage/TransInformation.h"
#include <QMap>
#include <QObject>
BEGIN_NX_NAMESPACE

class    CWebSocketClient;
class NXAPPSERVICE_EXPORT CInformationProcessing  : public QObject
{
	Q_OBJECT

public:
	CInformationProcessing(QObject *parent);
	~CInformationProcessing()=default ;
	 void  receiveMessage(const QString& receiveMessage);
	 void  sendMessage(const QString& sendMessage);
	 bool  isConnect();
	 void  initClient();
	 void  continueGame();

signals:
	void receiveCommandMessage(ActionCommand commandType);
	void receiveDeviceMessage(DeviceStatus deviceStatus);
	void receiveAnalysisMessage(QMap<QString, QString>data);
	void initGame(TransInformation initInfo);

private:
	CWebSocketClient* webSocketClient;
	bool isClientConnect=false;
	TransInformation receiveTransInformation;
};
END_NX_NAMESPACE
