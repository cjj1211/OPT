#include "CWebSocketClient.h"
#include <QThread>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QTimer>
#include <QUrl>
#include <QAbstractSocket>
#include <CLogger.h>
BEGIN_NX_NAMESPACE
using namespace NX;
CWebSocketClient::CWebSocketClient(QString ClientAddress, int HeartBeatTime, int MissedHeartbeatsThreshold, QObject* parent) :
	QObject(parent),
	connectAddress(ClientAddress),
	clientHeartBeatTime(HeartBeatTime),
	missedHeartbeatsThreshold(MissedHeartbeatsThreshold),
	heartbeatCheckTimer(new QTimer(this))
{
	QThread* webSocketThread = new QThread();
	moveToThread(webSocketThread);
	QObject::connect(webSocketThread, &QThread::finished, this, &QObject::deleteLater);
	webSocketThread->start();
	QMetaObject::invokeMethod(this, &CWebSocketClient::InitClient, Qt::QueuedConnection);
	connect(heartbeatCheckTimer, &QTimer::timeout, this, &CWebSocketClient::HeartBeatCheck);
}

CWebSocketClient::~CWebSocketClient()
{

	heartbeatCheckTimer->stop();
	delete heartbeatCheckTimer;

	if (webSocket) {
		webSocket->close();
		webSocket->deleteLater();
		webSocket = nullptr;
	}
}

bool CWebSocketClient::IsConnected()
{
	if (webSocket->state() == QAbstractSocket::ConnectedState)
	{

		return true;
	}
	else {

		qDebug() << "WebSocket error occurred: " << webSocket->errorString();
		LOG_INFO("Client connect failed!");
		return false;
	}
}

bool CWebSocketClient::Reconnect()
{
	//if (IsConnected()) {
	//	webSocket->close();
	//}
	//InitClient();
	//return IsConnected();
	return true;
}

void CWebSocketClient::DisConnected()
{
	if (IsConnected()) {
		webSocket->close();
	}

}

void CWebSocketClient::SendTextMessage(const QString& message)
{
	if (IsConnected()) {
	
		if (webSocket && webSocket->isValid()) {
			webSocket->sendTextMessage(message);
		}
		
		//heartbeatCheckTimer->start(clientHeartBeatTime);
	}
	else {
		LOG_INFO(" Client message failed!");
	}
}

void CWebSocketClient::initGame()
{
	emit SendTextMessageSignal ("InitGame");
}

void CWebSocketClient::HeartBeatCheck()
{
	//missedHeartbeats++;
	//if (missedHeartbeats >= missedHeartbeatsThreshold) {
	//	if (Reconnect())
	//	{
	//		missedHeartbeats = 0;  // reset the counter after reconnecting
	//	}
	//	else
	//	{
	//		LOG_INFO("Reconnect failed!");
	//	}

	//}
}



void CWebSocketClient::ReceiveTextMessage(const QString& message)
{
	ClientReceiveHeartBeat(message);

}

void CWebSocketClient::InitClient()
{
	webSocket = new QWebSocket();
	connect(webSocket, &QWebSocket::disconnected, this, &CWebSocketClient::DisConnected);
	connect(webSocket, &QWebSocket::textMessageReceived, this, &CWebSocketClient::ReceiveTextMessage);
	connect(this, &CWebSocketClient::SendTextMessageSignal, this, &CWebSocketClient::SendTextMessage);

	QUrl url("ws://"+connectAddress);
	if (!url.isValid()) {
		LOG_WARN("URL is not valid ");
	}
	webSocket->open(url);
	heartbeatCheckTimer->start(clientHeartBeatTime);

}

void CWebSocketClient::ClientReceiveHeartBeat(const QString& message)
{
	//判断是否是心跳数据
	if (message == (QStringLiteral("NXHeartbeatIsJump")))
	{
		missedHeartbeats = 0;  // reset the counter
		heartbeatCheckTimer->start(clientHeartBeatTime);  // reset the timer
		SendTextMessage("NXHeartbeatIsJump");
	}
	else
	{
		
		emit ReceiveMessage(message);
		
	
	}
}
END_NX_NAMESPACE