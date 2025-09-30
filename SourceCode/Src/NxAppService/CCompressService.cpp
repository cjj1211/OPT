#include "CCompressService.h"
#include "CCompressThread.h"





BEGIN_NX_NAMESPACE

CCompressService::CCompressService(QObject* parent)
	: IAppService(parent)
	//, compressThread(nullptr)
{
}

CCompressService::~CCompressService()
{
	
}



void CCompressService::exportToDir(const QString& outDir,const QString& inputDir)
{
	
	//if (nullptr != compressThread)
	//{
	//	compressThread->wait();
	//	delete compressThread;
	//	compressThread = nullptr;
	//}
	//
	//compressThread = new CCompressThread(CCompressThread::compress_Dir, outDir, inputDir,false, this);
	//connect(compressThread, SIGNAL(signalCompressProgress(QString, int, int)), SIGNAL(signalCompressProgress(QString, int, int)));
	//compressThread->start();
}

void CCompressService::importToDir(const QString& inputDir, const QString& outDir)
{
	//if (nullptr != compressThread)
	//{
	//	compressThread->wait();
	//	delete compressThread;
	//	compressThread = nullptr;
	//}
	//compressThread = new CCompressThread(CCompressThread::extract_Dir, outDir, inputDir, false, this);
	//connect(compressThread, SIGNAL(signalExtractProgress(QString, int, int)), SIGNAL(signalExtractProgress(QString, int, int)));
	//compressThread->start();
}


END_NX_NAMESPACE


