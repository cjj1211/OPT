#include "CWebSocketServer.h"
#include <QThread>
#include <QWebSocketServer>
#include <QWebSocket>
#include<QTimer>
#include<QUrl>

#include <CLogger.h>

BEGIN_NX_NAMESPACE
using namespace NX;
CWebSocketServer::CWebSocketServer(int port, int HeartBeatTime, int MissHeartBreat, QObject* parent) :
    QObject(parent),
    serverPort(port),
    heartbeatTime(HeartBeatTime),
    missHeartbeat(MissHeartBreat)
{
    QThread* webSocketThread = new QThread();
    moveToThread(webSocketThread);
    QObject::connect(webSocketThread, &QThread::finished, this, &QObject::deleteLater);
    QObject::connect(webSocketThread, &QThread::started, [=]() {
        // Start the WebSocket connection
        InitServer();
    });

    webSocketServer = new QWebSocketServer(QStringLiteral("Echo Server"), QWebSocketServer::NonSecureMode);
    heartbeatTimer = new QTimer();

    if (webSocketServer->listen(QHostAddress::Any, serverPort))
    {
        connect(webSocketServer, &QWebSocketServer::newConnection, this, &CWebSocketServer::NewConnection);
    }
    else {
        LOG("error connect");
    }

    webSocketThread->start();
    /* heartbeatTimer->setInterval(heartbeatTime);
     connect(heartbeatTimer, &QTimer::timeout, this, &CWebSocketServer::SendHeartBeat);
     heartbeatTimer->start();*/

}

CWebSocketServer::~CWebSocketServer()
{
    for (QWebSocket* client : clients) {
        if (client->isValid()) {
            client->close();
        }
        client->deleteLater();
    }
    clients.clear();

    if (webSocketServer->isListening()) {
        webSocketServer->close();
    }

    delete webSocketServer;
    delete heartbeatTimer;
}

bool CWebSocketServer::IsOpen()
{
    return   (webSocketServer && webSocketServer->isListening());
}

void CWebSocketServer::SendTextMessage(const QString& message)
{

    for (QWebSocket* client : clients) {
        if (client && client->isValid()) {
            client->sendTextMessage(message);
        }
    }
}



QString CWebSocketServer::ReceiveTextMessage(const QString& message)
{
    if (message == "NXHeartbeatIsJump") {
        lastHeartbeatReceived = 0;
    }
  
    else {
        emit ReceiveMessage(message);
    }
    return message;
}
void CWebSocketServer::SendHeartBeat()
{

    lastHeartbeatReceived++;
    QString heartbeatMessage = "NXHeartbeatIsJump";
    SendTextMessage(heartbeatMessage);
    ReceiveHeartBeat();
}

void CWebSocketServer::ReceiveHeartBeat()
{
    if (lastHeartbeatReceived >= missHeartbeat)
    {
        LOG_INFO("Receive Heartbeat failed time: " + QString::number(missHeartbeat).toStdString());
    }
}

void CWebSocketServer::InitServer()
{
    NewConnection();
}
void CWebSocketServer::NewConnection()
{
    QWebSocket* socket = webSocketServer->nextPendingConnection();
    if (socket) {

        LOG_INFO("New client connected: ");
        LOG_INFO(" - IP Address: " + socket->peerAddress().toString().toStdString());
        LOG_INFO(" - Port: " + QString::number(socket->peerPort()).toStdString());
        LOG_INFO(" - Hostname: " + socket->peerName().toStdString());
        clients << socket;
    }
    else {
        LOG_ERR(" Handle error condition here, such as logging an error message");
    }
    for (QWebSocket* client : clients) {
        if (client && client->isValid()) {
            connect(client, &QWebSocket::textMessageReceived, this, &CWebSocketServer::ReceiveTextMessage);
            connect(client, &QWebSocket::disconnected, this, &CWebSocketServer::ClientDisconnected);
        }
    }

}
void CWebSocketServer::ClientDisconnected()
{
    QWebSocket* client = qobject_cast<QWebSocket*>(sender());
    if (client) {
        clients.removeOne(client);
        client->deleteLater();
    }
}
END_NX_NAMESPACE