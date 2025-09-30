#include "HeartBeatThread.h"

HeartBeatThread::HeartBeatThread(int msec, QObject* parent)
	: QThread(parent)
	,Mscoend(msec)
	, HeartBeatTimer(new QTimer())

{
	HeartBeatTimer->setInterval(msec);
}

HeartBeatThread::~HeartBeatThread()
{
	delete HeartBeatTimer;
}

void HeartBeatThread::run()
{
	connect(HeartBeatTimer,&QTimer::timeout, [=]() {
		emit SendHeart();
		});
	HeartBeatTimer->start();
}
