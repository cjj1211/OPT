#ifndef __CWebSocketClient_H
#define __CWebSocketClient_H
/******************************************************************************
*  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
*  作者 : <崔俊杰><junjie.cui@neuroxess.com>
*  修改记录:
*  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#include "CWebSocketClientGlobal.h"
#include<QObject>


class  QWebSocket;
class  QTimer;
class  CLogger;
BEGIN_NX_NAMESPACE
class   CWebSocketClient :public QObject
{
    Q_OBJECT
public:
    /**

        @fn     CWebSocketClient
        @brief  CWebSocketClient object constructor
        @param  ClientAddress             - 连接地址
        @param  HeartBeatTime             - 心跳间隔（毫秒）
        @param  MissedHeartbeatsThreshold - 错过心跳阈值
        @param  parent                    - 
        @author CuiJunJie
        @date   7.08.2023

    **/
    explicit  CWebSocketClient(QString ClientAddress, int HeartBeatTime, int MissedHeartbeatsThreshold, QObject* parent = nullptr);

    ~CWebSocketClient();
    /**

        @fn     IsConnected
        @brief  判断是否连接
        @retval  -
        @author CuiJunJie
        @date   2.08.2023

    **/
    bool IsConnected();
    /**

        @fn     Reconnect
        @brief  判断是否重连成功
        @retval  -
        @author CuiJunJie
        @date   2.08.2023

    **/
    bool Reconnect();
    /**

        @fn     DisConnected
        @brief  断开连接
        @author CuiJunJie
        @date   2.08.2023

    **/
    void DisConnected();

    /**

        @fn     ReceiveTextMessage
        @brief  客户端接收信息
        @param  message - 接收信息
        @retval         -
        @author CuiJunJie
        @date   2.08.2023

    **/
    void  ReceiveTextMessage(const QString& message);
    /**

        @fn     init
        @brief  初始化webscokect
        @author CuiJunJie
        @date   4.08.2023

    **/
    void InitClient();
    /**

    @fn     SendTextMessage
    @brief  客户端发送信息
    @param  message -  发送的信息
    @author CuiJunJie
    @date   2.08.2023

**/
    void SendTextMessage(const QString& message);
    void initGame();
public:
    QString initMessage = "";
signals:
    //触发收到信息信号用于外部接收
    QString ReceiveMessage(QString);
    //用于触发发送信息
    void SendTextMessageSignal(const QString& message);
private slots:
    ;
    /**

        @fn     ClientReceiveHeartBeat
        @brief  客户端收到心跳进行处理
        @param  message -
        @author CuiJunJie
        @date   4.08.2023

    **/
    void ClientReceiveHeartBeat(const QString& message);

    /**

        @fn     HeartBeatCheck
        @brief  心跳检测
        @author CuiJunJie
        @date   4.08.2023

    **/
    void HeartBeatCheck();
 


private:
    /**
 * 连接地址
 */
    QString connectAddress;
    /**
   * 心跳时间
   */
    int  clientHeartBeatTime;

    QWebSocket* webSocket;
    /**
    * 心跳定时器
    */
    QTimer* heartbeatCheckTimer;
    /**
      * 添加一个计数器用于统计错失心跳次数
   */
    int missedHeartbeats = 0;
    /**
      * 错失心跳次数预设的阈值进行重连
   */
    const int missedHeartbeatsThreshold;  // 预设的阈值
};

#endif // __CWebSocketClientThread_H
END_NX_NAMESPACE