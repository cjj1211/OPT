#include "COscillographService.h"
#include "CCompressThread.h"

#include "NxSystemCfg/CSystemCfg.h"
#include "NxCommunication/DiscreteList.h"
#include "NxCommunication/CMasterServer.h"
#include "NxDBManager/CChannelPlanRepository.h"
#include "NxDBManager/CPatientRepository.h"
#include "NxDBManager/CChannelNameRepository.h"
#include "NxDBManager/CTreatmentRecordRepository.h"
#include "NxEntity/ChannelPlan.h"
#include "NxEntity/ChannelName.h"
#include "NxEntity/TreatmentRecord.h"

#include <QCoreApplication>
#include <QtConcurrent/QtConcurrent>
#include <QDir>
#include <QDateTime>
#include <QVector>
#include <QUUid>
#include <QProcessEnvironment>
#include <Windows.h>
#include <TlHelp32.h>
#include <tchar.h>

#pragma comment(lib, "advapi32.lib")

BEGIN_NX_NAMESPACE

const char g_kServiceName[] = "Optimus";
const char g_kDateTimeFormat[] = "yyyy-MM-dd hh-mm-ss";
const char g_kDateTimeFormatInDB[] = "yyyy-MM-dd hh:mm:ss";

COscillographService::COscillographService(QString patientUid, QObject* parent)
	: IAppService(parent)
	, runStatus(RS_IDLE)
	, recordType(RT_RECORDER)
	, compressThread(nullptr)
	, channelPlanRepository(new CChannelPlanRepository())
	, patientRepository(new CPatientRepository())
	, channelNameRepository(new CChannelNameRepository())
	, treatmentRecordRepository(new CTreatmentRecordRepository())
	, currentPatientUid(std::move(patientUid))
	, runtimeInMSecs(0)
{
	restartServiceByName(g_kServiceName);
	auto commonCfg = systemConfig.GetSystemCfg().Common;
	patientDataPath = QDir(QString::fromStdString(commonCfg.StoragePath)).absolutePath() + "/" + currentPatientUid;
	recorderType = RT_RECORDER;

	recorderClient = std::make_unique<RecorderClient>(commonCfg.RecorderAddress);
	recorderClient->closeController();
	masterServer = std::make_unique<CMasterServer>(commonCfg.MasterAddress);
}

COscillographService::~COscillographService()
{
	if (RS_RUNNING == runStatus)
	{
		stopRun();
	}

	if (RS_RECORDING == runStatus)
	{
		stopRecord();
	}

	processWatcherRunning = false;
	watcherThread.waitForFinished();
	shutdownProcess();
	recorderClient->closeController();
}

void COscillographService::setRestore(QVector<QString> restorePath)
{
	recorderClient->setRestore(restorePath);
}

bool COscillographService::initBoard(const int sampleRate)
{
    LOG_INFO("initBoard..................start.");
	recorderClient->closeController();
    if(!systemConfig.GetSystemCfg().Hardware.SyntheticMode && !recorderClient->isControllerConnected())
	{
		emit signalBoardInited(false, QMap<QString, QString>());
		return false;
	}
	
	auto channelPlans = getChannelPlans();
	InitInfo initInfo;
	if(systemConfig.GetSystemCfg().Hardware.CommunicationMode==0)
	{
		initInfo.AcquisitionMode = systemConfig.GetSystemCfg().Hardware.SyntheticMode ? SyntheticMode : LiveMode;
	}
	else
	{
		initInfo.AcquisitionMode = IcuMode;
	}
	
	initInfo.UseGpu = systemConfig.GetSystemCfg().Hardware.EnableGPU;
	initInfo.CompressData = systemConfig.GetSystemCfg().Hardware.EnableCompress;
	initInfo.SampleRate = sampleRate;
	initInfo.SaveFileType = QString::fromStdString(systemConfig.GetSystemCfg().Hardware.SaveFileType);
	
	auto initBoardFinishFunc = [this](const InitInfo& info, const QString& masterId) {
		QMap<QString, QString> channelInfos;
        const auto initRes = recorderClient->initBoard(info, channelInfos);
		emit signalBoardInited(initRes, channelInfos);

		recorderClient->setMasterId(masterId.toStdString());
	};
	QtConcurrent::run(initBoardFinishFunc, initInfo, QString::number(qApp->applicationPid()));

	recorderClient->setLoadWaveformCallback([=](const bool isSweepFirstTime,
                                                const bool isUseVerticalLines,
                                                const int validDataIndex,
                                                const uint64_t startTimestamp,
                                                const uint64_t endTimestamp,
                                                const bool isRunning,
                                                const QVector<QMap<std::string, std::string>>& waveforms) {
		emit signalUpdateWaveform(isSweepFirstTime, isUseVerticalLines, validDataIndex, startTimestamp, endTimestamp, isRunning, waveforms);
		});

	recorderProcessWatcher();

    LOG_INFO("initBoard..................finished.");
	return true;
}

void COscillographService::initPlayback(const int sampleRate)
{
	LOG_INFO("initPlayback..................start.");

	TreatmentRecord record;
	treatmentRecordRepository->GetByUID(recordUid, record);
    const auto dateTimeStr = QString::fromStdString(record.StartDatetime);
	recordStartDatetime = QDateTime::fromString(dateTimeStr, g_kDateTimeFormatInDB);
	currentPatientEventPath = QString::fromStdString(record.TrainDataPath) + "/" + "EventRecord.json";
    const auto recordDir = QDir(QString::fromStdString(record.TrainDataPath));
	auto list = recordDir.entryList(QStringList() << "*.edf" << "*.rhd" , QDir::Files, QDir::Time);
	if (list.count() == 0)
	{
        const QMap<QString, QString> channelInfos;
		emit signalBoardInited(false, channelInfos);
		return;
	}

	auto channelPlans = getChannelPlans();
	InitInfo initInfo;
	initInfo.AcquisitionMode = PlaybackMode ;
	initInfo.SaveFilePath = recordDir.absolutePath() + "/" + list[list.count() - 1];
	initInfo.SaveFileType = QString::fromStdString(record.SaveFileType);
	initInfo.UseGpu = systemConfig.GetSystemCfg().Hardware.EnableGPU;
	initInfo.CompressData = systemConfig.GetSystemCfg().Hardware.EnableCompress;
	initInfo.SampleRate = sampleRate; 

	auto initBoardFinishFunc = [=](const InitInfo& initializeInfo, const QString& masterId) {
		QMap<QString, QString> channelInfos;
        const auto initRes = recorderClient->initBoard(initializeInfo, channelInfos);
		emit signalBoardInited(initRes, channelInfos);

		recorderClient->setMasterId(masterId.toStdString());
	};
	QtConcurrent::run(initBoardFinishFunc, initInfo, QString::number(qApp->applicationPid()));

	recorderClient->setLoadWaveformCallback([=](bool isSweepFirstTime,
		bool isUseVerticalLines,
		int validDataIndex, 
		uint64_t startTimestamp,
		uint64_t endTimestamp,
		bool isRunning,
		QVector<QMap<std::string, std::string>> waveforms) {
			emit signalUpdateWaveform(isSweepFirstTime, isUseVerticalLines, validDataIndex, startTimestamp, endTimestamp,isRunning, waveforms);
			qApp->processEvents();
		});

	LOG_INFO("initPlayback..................finished.");
}

void COscillographService::setPlaybackStopFunc(const std::function<void()>& func) const
{
	masterServer->setPlaybackStopedFunc(func);
}

bool COscillographService::getRecordInfo(TreatmentRecord& recordInfo) const
{
	return treatmentRecordRepository->GetByUID(recordUid, recordInfo);
}

void COscillographService::jumpToSecs(const uint64_t timeInSecs) const
{
	recorderClient->jumpToPosition(timeInSecs);
}

void COscillographService::exportToDir(const QString& outDir)
{
	TreatmentRecord recordInfo;
	getRecordInfo(recordInfo);
	if (nullptr != compressThread)
	{
		compressThread->wait();
		delete compressThread;
		compressThread = nullptr;
	}
    const QDir dir(outDir);
    const QString recordUid = QString::fromStdString(recordInfo.UID).replace("{", "").replace("}", "");
    const QString zipFile = dir.absolutePath() + "/" + recordUid + ".zip";
	compressThread = new CCompressThread(CCompressThread::compress_Dir, zipFile, QString::fromStdString(recordInfo.TrainDataPath),false, this);
	connect(compressThread, SIGNAL(signalCompressProgress(QString, int, int)), SIGNAL(signalCompressProgress(QString, int, int)));
	compressThread->start();
}

QString COscillographService::getEventSavePath()
{

	return currentPatientEventPath;
}

void COscillographService::startRun()
{
	if (RS_RECORDING == runStatus)
	{
		stopRecord();
	}

	if (RS_RUNNING == runStatus)
	{
		return;
	}
	else
	{
		runtimeInMSecs = 0;
		asyncSendSetCommand("RunMode", "Run");
		if (recorderType == RT_RECORDER)
		{
			recordStartDatetime = QDateTime::currentDateTime();
		}
		recorderClient->loadWaveformData();
		runStatus = RS_RUNNING;
	}
	
}

void COscillographService::stopRun()
{
	if (RS_RUNNING != runStatus ) // 非采集状态，不操作，直接返回
	{
		return;
	}

	syncSendSetCommand("RunMode", "Stop");
	recorderClient->stopLoadWaveformStream();
	runStatus = RS_IDLE;
}

void COscillographService::startRecord()
{
	if (RS_RUNNING == runStatus)  // 正在采集，非录制，先停止采集
	{
		stopRun();
	}

	if (RS_RECORDING == runStatus) // 已经在录制，不需要操作，直接返回
	{
		return;
	}
	else
	{
		runtimeInMSecs = 0;
        const auto datetimeStr = QDateTime::currentDateTime().toString(g_kDateTimeFormat);
		trainDataPath = patientDataPath + "/EEG/" + datetimeStr;
		const QDir dir(trainDataPath);
		currentPatientEventPath = dir.absolutePath() + "/EventRecord.json";
		if (!dir.exists())
		{
			auto res = dir.mkpath(dir.absolutePath());//创建多级目录
		}
        const auto rhdFilePath = dir.absolutePath()  + "/" + currentPatientUid + ".rhd";
		recorderClient->setStoragePath(rhdFilePath);
		
		asyncSendSetCommand("RunMode", "Record");
		recordStartDatetime = QDateTime::currentDateTime();
		recorderClient->loadWaveformData();
		runStatus = RS_RECORDING;
	}
}

void COscillographService::stopRecord()
{
	if (RS_RECORDING != runStatus) // 没有在录制，不操作，直接返回
	{
		return;
	}
	syncSendSetCommand("RunMode", "Stop");
	insertTrainRecord();
	recorderClient->stopLoadWaveformStream();
	runStatus = RS_IDLE;
}

void COscillographService::updateDisplayParam(const double timespan, const double dispWidth, const double refreshCount, const bool isRoll) const
{
	recorderClient->updateDisplayParam(timespan, dispWidth, refreshCount, isRoll);
}

void COscillographService::changeOsMode(const OscillographMode& mode) const
{
	recorderClient->changeOsMode(mode);
}

void COscillographService::setVisibleWaveNamesHandle(const VisibleWaveNamesHandle& handle) const
{
	recorderClient->setVisibleWaveNamesHandle(handle);
}

void COscillographService::setSelectedWaveName(const QString& nativeWaveName) const
{
	recorderClient->setSelectedWaveName(nativeWaveName);
}

void COscillographService::setLoadSelectedAmpDataCallback(const LoadSelectedAmpDataCallback& callback) const
{
	recorderClient->setLoadSelectedAmpDataCallback(callback);
}

ChannelPlan COscillographService::getChannelPlanByUid(const QString& uid) const
{
	return channelPlanRepository->getChannelPlanByUid(uid);
}

std::vector<ChannelPlan> COscillographService::getChannelPlans() const
{
	return channelPlanRepository->getChannelPlanByPatient(currentPatientUid);
}

void COscillographService::syncSendSetCommand(const QString& parameter, const QString& value) const
{
	recorderClient->syncSendSetCommand(parameter, value);
}

void COscillographService::asyncSendSetCommand(const QString& parameter, const QString& value) const
{
	recorderClient->asyncSetCommand(parameter, value);
}

void COscillographService::insertTrainRecord() const
{
	TreatmentRecord record;
	record.UID  = QUuid::createUuid().toString().replace("{", "").replace("}", "").toStdString();
	record.PatientFK = currentPatientUid.toStdString();
	record.Score = 80;
	record.TrainDataPath = trainDataPath.toStdString();
	record.StartDatetime = recordStartDatetime.toString(g_kDateTimeFormat).toStdString();
	record.TrainTimeLen = getRuntimeInSecs() + 1;
	record.TrainType = TrainTypeEnum::Click;
	record.IsSuccess = true;
	record.IsCompress = systemConfig.GetSystemCfg().Hardware.EnableCompress;
	record.FailTimes = 0;
	record.SuccessTimes = 10;
	record.Difficult = 5;
	record.CreateDateTime = QDateTime::currentDateTime().toString(g_kDateTimeFormat).toStdString();
	record.UpdateDateTime = QDateTime::currentDateTime().toString(g_kDateTimeFormat).toStdString();
	record.IsDeleted = false;
	record.SaveFileType = systemConfig.GetSystemCfg().Hardware.SaveFileType;
	treatmentRecordRepository->Insert(record);

	auto channelPlans = channelPlanRepository->getChannelPlanByPatient(currentPatientUid);
	for (auto& plan : channelPlans) {
		auto channelNames = channelNameRepository->getChannelNameByChannelPlanFK(QString::fromStdString(plan.UID));

		plan.UID = QUuid::createUuid().toString().replace("{", "").replace("}", "").toStdString();
		plan.CreateDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString();
		plan.UpdateDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString();
		plan.IsDeleted = false;
		plan.RecordUidFK = record.UID;
		channelPlanRepository->Insert(plan);
		for (auto& name : channelNames) {
			name.UID = QUuid::createUuid().toString().replace("{", "").replace("}", "").toStdString();
			name.CreateDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString();
			name.UpdateDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString();
			name.ChannelPlanFK = plan.UID;
			channelNameRepository->Insert(name);
		}
	}
}

std::vector<ChannelName> COscillographService::getChannelNamesByPlan(const QString& plansUid) const
{
	auto channelNames = channelNameRepository->getChannelNameByChannelPlanFK(plansUid);
	ChannelPlan plan;
	channelPlanRepository->GetByUID(plansUid, plan);
	std::vector<ChannelName> enabledChannelNames;
	std::ranges::copy_if(channelNames, std::back_inserter(enabledChannelNames) , [&](const ChannelName& val) {
		return val.Indexs == plan.Indexs;
		});
	return enabledChannelNames;
}

void COscillographService::updateChannelPlan(const ChannelPlan& plan) const
{
	channelPlanRepository->Update(plan);
}
 
QPoint COscillographService::getChannelPosition(const QString& channelPlanUid, const QString& customName) const
{
	return channelNameRepository->getChannelPositionByUid(channelPlanUid, customName);
}

void COscillographService::getCurrentPatient(Patient& patient) const
{
	patientRepository->GetByUID(currentPatientUid, patient);
}

void COscillographService::updatePatient(const Patient& patient) const
{
	patientRepository->Update(patient);
}

ChannelName COscillographService::getChannelByChannelPosition(const QString& channelPlanUid, const int x, const int y, const QString& index) const
{
	return channelNameRepository->getChannelBychannelPosition(channelPlanUid, x, y, index);
}

void COscillographService::setSignalInfo(const WaveFormType& info) const
{

	recorderClient->setSignalInfo(info);
}
void COscillographService::setSignalType(const std::vector<int>& signalType) const
{
	recorderClient->setSignalType(signalType);
}

void COscillographService::setTimeScale(int time) const
{
	recorderClient->setTimeScale(time);
}

void COscillographService::setNotch(const QString& portName, const QString& type) const
{
	NotchType notchType;
	if (type.toLower() == "50") {
		notchType = NT_50Hz;
	}
	else if (type.toLower() == "60") {
		notchType = NT_60Hz;
	}
	else {
		notchType = NT_NONE;
	}
	recorderClient->setNotch(portName, notchType);
}

void COscillographService::setLowFilter(const QString& portName, const double filterValue) const
{
	recorderClient->setLowFilter(portName, filterValue, 8, Butterworth);
}

void COscillographService::setHighFilter(const QString& portName, const double filterValue) const
{
	recorderClient->setHighFilter(portName, filterValue, 8, Butterworth);
}

void COscillographService::updateFilterParam(const ChannelPlan& channelPlan) const
{
	const std::vector<std::string> notchFilterList = { "None", "50 Hz", "60 Hz" };
	const auto filter = "";
	auto notchFilterStr = "None";
	if (channelPlan.Notch == "50") {
		notchFilterStr = "50 Hz";
	}
	else if (channelPlan.Notch == "60") {
		notchFilterStr = "60 Hz";
	}

    const auto firstChannelPlan = getChannelPlans()[0];
	
	recorderClient->updateFilterParams(
		QString::fromStdString(channelPlan.Indexs),
		channelPlan.EnableLowCut,
		channelPlan.EnableHighCut, 
		firstChannelPlan.EnableDSP,
		firstChannelPlan.DspCutoff,
		firstChannelPlan.HwLowCut,
		firstChannelPlan.HwHighCut,
		channelPlan.LowCut, 
		channelPlan.HighCut, notchFilterStr, 8, "butterworth", 
		channelPlan.SampleRate,
		systemConfig.GetSystemCfg().Hardware.MultiNotchCount);
}

void COscillographService::getImpedanceInfos(const QStringList& enabledPorts, QVector<ImpedanceInfo>& impedanceInfos) const
{
	const QDir dir(patientDataPath + "/Impedance/");
	if (!dir.exists())
	{
		auto res = dir.mkpath(dir.absolutePath());//创建多级目录
	}
    const auto datetimeStr = QDateTime::currentDateTime().toString(g_kDateTimeFormat);
    const auto impedanceFile = dir.absolutePath() + "/" + datetimeStr + ".csv";
	// TODO 需要设置 阻抗测试频率
	recorderClient->setImpedanceFilePath(impedanceFile);
	recorderClient->getImpedanceInfos(enabledPorts,impedanceInfos);
}

void COscillographService::getImpedanceMeasureStatus(bool& isCompleted, int& progress) const
{
	recorderClient->getImpedanceMeasureStatus(isCompleted, progress);
}

void COscillographService::getHeatScales( QMap<QString, float>& heatScales) const
{
	recorderClient->getHeatScales(heatScales);
}

void COscillographService::enableMeasureHeatScale(const bool enable) const
{
	recorderClient->enableMeasureHeatScale(enable);
}

void COscillographService::setHeatScaleChannels(const QStringList& channelNames) const
{
	recorderClient->setHeatScaleChannels(channelNames);
}

bool COscillographService::restartServiceByName(const std::string& strDestServiceName)
{
	return true;
	LOG_INFO("restartServiceByName.................start");
	bool bServiceStatus = FALSE;
    DWORD dwBytesNeeded = 0;
	SERVICE_STATUS_PROCESS ssStatus;
	char szSvcName[MAX_PATH] = { 0 };
	memcpy_s(szSvcName, MAX_PATH, strDestServiceName.c_str(), strDestServiceName.length());
	//! 获取一个服务控制管理器数据库的句柄
    const SC_HANDLE schSCManager = OpenSCManager(
        nullptr, // local computer
        nullptr, // ServicesActive database
        SC_MANAGER_ALL_ACCESS);             // full access rights

	if (schSCManager == nullptr) {
		LOG_ERR("OpenSCManager fail");
		return bServiceStatus;
	}

	//! 获取该服务在服务控制管理器数据库中的句柄
    const SC_HANDLE schService = OpenServiceA(
        schSCManager, // SCM database
        szSvcName, // name of service
        SERVICE_ALL_ACCESS/* | DELETE*/);   // full access

	if (schService == nullptr) {
		CloseServiceHandle(schSCManager);
		LOG_ERR("Get a handle to the service fail");
		return bServiceStatus;
	}

	//! 查询该服务的当前状态
	if (!QueryServiceStatusEx(
		schService,                         // handle to service
		SC_STATUS_PROCESS_INFO,             // information level
		reinterpret_cast<LPBYTE>(&ssStatus),                 // address of structure
		sizeof(SERVICE_STATUS_PROCESS),    // size of structure
		&dwBytesNeeded))                  // size needed if buffer is too small
	{
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		LOG_ERR("QueryServiceStatusEx fail");
		return bServiceStatus;
	}
	else
	{
		switch (ssStatus.dwCurrentState)
		{
		case  SERVICE_STOPPED:
		case  SERVICE_STOP_PENDING:
			LOG_INFO(std::string(szSvcName) + " Service status is Stop");
			break;
		case  SERVICE_PAUSED:
		case  SERVICE_PAUSE_PENDING:
			LOG_INFO(std::string(szSvcName) + " Service status is Pause");
			break;
		case  SERVICE_CONTINUE_PENDING:
		case  SERVICE_RUNNING:
		case  SERVICE_START_PENDING:
			LOG_INFO(std::string(szSvcName) + " Service status is Running");
			bServiceStatus = TRUE;
			break;
		}
	}

	//! 服务正在运行，返回
	if (bServiceStatus == TRUE) {
		return true;
	}

	//! 启动该服务
	StartService(schService, 0, nullptr);
	Sleep(500);

	//! 获得服务的当前状态
	QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, reinterpret_cast<LPBYTE>(&ssStatus), sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded);
	if (SERVICE_RUNNING == ssStatus.dwCurrentState)
	{
		bServiceStatus = TRUE;
	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
	LOG_INFO("restartServiceByName.................finish");
	return bServiceStatus;
}

void COscillographService::createProcess()
{

	LOG_INFO("createProcess ..................start");
	QString serverPath = QProcessEnvironment::systemEnvironment().value("OptimusServerPath");
	serverPath.replace(QRegExp("\\\\"), "/");

    const auto serverDir = QDir(serverPath);
	if(!serverDir.exists())
	{ 
		serverDir.mkdir(serverPath);
	}

	QProcess::execute("taskkill /im "+ serverDir.absolutePath() + "/OptimusServer.exe /f ");

	// 参考 https://blog.csdn.net/junxuezheng/article/details/127605154
	if (findProcessById(recorderProcessId)) // 杀掉之前的进程
	{
		LOG_INFO(std::format("Process is running, process id:{}", recorderProcessId));
		killProcess(recorderProcessId);
		while (findProcessById(recorderProcessId))
		{
			LOG_INFO(std::format("Process name has already exist, process id:{}", recorderProcessId));
			QThread::msleep(1000);
		}
		recorderProcessId = 0;
		LOG_INFO("Process has been terminated.");
	}
	else
	{
		LOG_INFO(std::format("Process has already stoped, processId:{}", recorderProcessId));
	}

	std::string cmd = serverDir.absolutePath().toStdString() + "/OptimusServer.exe ";

	cmd = cmd + systemConfig.GetSystemCfg().Common.RecorderAddress + " ";
	cmd = cmd + systemConfig.GetSystemCfg().Common.MasterAddress;

	STARTUPINFOA startInfo;
	startInfo = { 0 };
	startInfo.cb = sizeof(startInfo);

	PROCESS_INFORMATION processInfo;
	processInfo = { nullptr };
    const auto stdServerPath = serverPath.toStdString();
    const LPCSTR workingDir = stdServerPath.c_str();

    constexpr auto createFlags = false ? CREATE_NEW_CONSOLE : CREATE_NO_WINDOW;

	if (!CreateProcessA(nullptr, const_cast<char*>(cmd.c_str()), nullptr, nullptr,
		FALSE, createFlags, nullptr, workingDir, &startInfo, &processInfo))
	{
		LOG_ERR("createProcess FAILED");
	}
	else
	{
		recorderProcessId = processInfo.dwProcessId;
		LOG_INFO(std::format("Process has started, process id:{}", recorderProcessId));
	}

	LOG_INFO("createProcess ..................finished");
}


bool COscillographService::findProcessById(const DWORD processId)
{
	if (processId == 0)
	{
		return false;
	}

	PROCESSENTRY32 pe32;
	const HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return(false);
	}
	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(hProcessSnap, &pe32))
	{
		CloseHandle(hProcessSnap);          // clean the snapshot object
		return(false);
	}

	bool processExist = false;
	do
	{
		if (pe32.th32ProcessID == static_cast<DWORD>(processId))//进程名称
		{
			processExist = true;
			break;
		}
	} while (Process32Next(hProcessSnap, &pe32));
	CloseHandle(hProcessSnap);

	if (!processExist)
	{
		LOG("Process Not Exist.");
	}

	return processExist;
}

void COscillographService::shutdownProcess()
{
	killProcess(recorderProcessId);
}

bool COscillographService::killProcess(const DWORD processId)
{
	if (processId == 0)
	{
		return true;
	}
	const HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	PROCESSENTRY32 pe;
	pe.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hSnapShot, &pe))
	{
		return false;
	}

	bool hasKilled = false;
	while (Process32Next(hSnapShot, &pe))
	{
		if (static_cast<DWORD>(processId) == pe.th32ProcessID)
		{
			const HANDLE hProcess = ::OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
			TerminateProcess(hProcess, 0);
			CloseHandle(hProcess);
			hasKilled = true;
		}
	}

	return hasKilled;
}

void COscillographService::recorderProcessWatcher()
{
	LOG_INFO("recorderProcessWatcher..................start.");
	processWatcherRunning = true;
	watcherThread = QtConcurrent::run([this]()
		{
			while (processWatcherRunning && recorderClient != nullptr)
			{
				if (!measuringImpedance && (!recorderClient->isServerAvailable() || !findProcessById(recorderProcessId)))// 阻抗测量过程中不允许watcher
				{
					LOG_WARN("Record server disconnected.");
					processWatcherRunning = false;
					emit signalRecorderProcessExited();
					break;
				}
				QThread::msleep(100);
			}
			processWatcherRunning = false;
			LOG_INFO("recorderProcessWatcher..................exit.");
		});
	LOG_INFO("recorderProcessWatcher..................finished.");
}

bool COscillographService::exportData(const QString& destFilePath_,  const QString& exportFileType_) const
{
	TreatmentRecord record;
	Patient patient;
	treatmentRecordRepository->GetByUID(recordUid, record);
	patientRepository->GetByUID(QString::fromStdString(record.PatientFK), patient);
	const auto recordDir = QDir(QString::fromStdString(record.TrainDataPath));
	auto list = recordDir.entryList(QStringList() << "*.edf" << "*.rhd", QDir::Files, QDir::Time);
	if (list.count() == 0)
	{
		return false;
	}

    const auto sourceFilePath = recordDir.absolutePath() + "/" + list[list.count() - 1];
	const auto timeStr = QDateTime::fromString(QString::fromStdString(record.StartDatetime), g_kDateTimeFormat).toString("yyyymmdd_HHMM");
	const auto exportFileName = patient.PatientName + "_" + patient.CaseID + "_" + exportFileType_.toStdString() + "_" + timeStr.toStdString() + ".zip";
	const auto exportFilePath = destFilePath_ + "/" + QString::fromStdString(exportFileName);
	return recorderClient->exportData(sourceFilePath, exportFilePath, record.IsCompress, exportFileType_);
}

void COscillographService::stopExport() const
{
	recorderClient->stopExport();
}

bool COscillographService::isExportRunning() const
{
	return recorderClient->isExportRunning();
}

int COscillographService::getExportProgress() const
{
	return recorderClient->getExportProgress();
}

END_NX_NAMESPACE


