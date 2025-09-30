#ifndef __CWebSocketServer_H
#define __CWebSocketServer_H
/******************************************************************************
*  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
*  作者 : <崔俊杰><junjie.cui@neuroxess.com>
*  修改记录:
*  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/

#include "IAppService.h"
#include <QObject>
class QWebSocketServer;
class QTimer;
class CLogger;
class QWebSocket;
BEGIN_NX_NAMESPACE
class    CWebSocketServer : public QObject
{
    Q_OBJECT
public:
    /**

        @fn     CWebSocketServer
        @brief  CWebSocketServer object constructor
        @param  port           - 端口号
        @param  HeartBeatTime  - 心跳间隔（毫秒）
        @param  MissHeartBreat - 错失心跳次数
        @param  parent         -
        @author CuiJunJie
        @date   7.08.2023

    **/
    explicit  CWebSocketServer(int port, int HeartBeatTime, int MissHeartBreat, QObject* parent = nullptr);
    /**
        @fn     ~WebSocketServerThread
        @brief  析构函数
        @author CuiJunJie
        @date   2.08.2023
    **/
    ~CWebSocketServer();
    /**

        @fn     IsOpen
        @brief  判断服务器是否打开
        @retval  -
        @author CuiJunJie
        @date   2.08.2023

    **/
    bool IsOpen();
    /**

        @fn     SendTextMessage
        @brief  服务端发送信息
        @param  message - 发送信息
        @retval         -
        @author CuiJunJie
        @date   2.08.2023

    **/
    void SendTextMessage(const QString& message);
    /**

        @fn     ReceiveTextMessage
        @brief  返回接收到的信息推送到前端
        @param  message - 接收到的信息
        @retval         - 返回接收到的信息推送到前端
        @author CuiJunJie
        @date   2.08.2023

    **/
    QString  ReceiveTextMessage(const QString& message);
    /**

        @fn     InitServer
        @brief  初始化Server
        @author CuiJunJie
        @date   7.08.2023

    **/
    void InitServer();
signals:
    // 接收到信息反馈接收信息
    QString ReceiveMessage(QString);

private slots:

    void NewConnection();
    /**

        @fn     SendHeartBeat
        @brief  发送心跳
        @author CuiJunJie
        @date   7.08.2023

    **/
    void SendHeartBeat();
    /**

        @fn     ReceiveHeartBeat
        @brief  当错失心跳大于某次数 进行记录
        @author CuiJunJie
        @date   7.08.2023

    **/
    void ReceiveHeartBeat();
    /**

        @fn     ClientDisconnected
        @brief  客户端断开连接服务端进行处理
        @author CuiJunJie
        @date   7.08.2023

    **/
    void ClientDisconnected();


private:
    //服务端端口号
    int  serverPort;
    // 心跳发送间隔
    int  heartbeatTime;
    QWebSocketServer* webSocketServer;
    QList<QWebSocket*> clients;
    //用于检测心跳
    QTimer* heartbeatTimer;

    int lastHeartbeatReceived = 0;
    //错失心跳次数
    int missHeartbeat;
};
END_NX_NAMESPACE
#endif // __CWebSocketServer_H