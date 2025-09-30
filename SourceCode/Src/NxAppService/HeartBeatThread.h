#ifndef __CHeartBeatThread_H
#define __CHeartBeatThread_H
/******************************************************************************
*  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
*  作者 : <崔俊杰><junjie.cui@neuroxess.com>
*  修改记录:
*  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#include <QThread>
#include<QTimer>
class HeartBeatThread: public QThread
{
	Q_OBJECT
public:
	explicit   HeartBeatThread(int msec ,QObject* parent = nullptr);
	~HeartBeatThread();
signals:
	void SendHeart();
private:
	void run() override;
	int  Mscoend;
	QTimer* HeartBeatTimer;
};
#endif // __HeartBeatThread_H
