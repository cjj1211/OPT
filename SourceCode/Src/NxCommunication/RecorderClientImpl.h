/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __RECORDER_CLIENT_IMPL_H
#define __RECORDER_CLIENT_IMPL_H
#include "nxcommunication_global.h"
#include "RecorderClient.h"

#include "Recorder.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <memory>
#include <vector>
#include <string>
#include <QString>
#include <QFuture>

class QThreadPool;
class QSharedMemory;
using grpc::ClientContext;
using grpc::Status;
using grpc::ClientReader;

using NX::Recorder;
using NX::EmptyMsg;
using NX::BoolMsg;
using NX::SampleMsg;
using NX::InitBoardMsg;
using NX::BufferStatusResponse;
using NX::WaveFormType;
BEGIN_NX_NAMESPACE

class RecorderClient::RecorderClientImpl
{
public:
	explicit RecorderClientImpl( std::shared_ptr<grpc::Channel> channel);

	~RecorderClientImpl();

	void setMasterId(const std::string processId) const;

	bool isControllerConnected() const;

	bool isServerAvailable() ;

	bool initBoard(const InitInfo& initInfo, QMap<QString, QString>& channelInfos) const;

	void resetSampleRate(const int sampleRate) const;

	void closeController() const;

	void setStoragePath(const QString& rhdFilePath) const;

	void setImpedanceFilePath(const QString& impedanceFilePath) const;

	std::string getPID() const;

	std::string getVID() const;

	std::string getDeviceID() const;

	void stopLoadWaveformStream();

	void changeOsMode(const OscillographMode& mode);

	void setSelectedWaveName(const QString& nativeWaveName) const;

	void loadSelectedAmpData();

	void loadWaveformData();

	void asyncSendSetCommand(const QString& parameter, const QString& value) const;

	void syncSendSetCommand(const QString& parameter, const QString& value) const;

	void asyncSendExecuteCommand(const QString& action) const;

	void syncSendExecuteCommand(const QString& action) const;

	std::vector<std::string> getDisplayListAmplifiers(const std::string& groupName) const;

	void getImpedanceInfos(const QStringList& enabledPorts, QVector<ImpedanceInfo>& impedanceInfos) const;

	void getImpedanceMeasureStatus(bool& isCompleted, int& progress);

	void getHeatScales(QMap<QString, float>& heatScales) const;

	void enableMeasureHeatScale(const bool enable) const;

	void setHeatScaleChannels(const QStringList& channelNames) const;

	void getBufferStatus(double& hdBufferPercent, double& swBufferPercent, double& cpuLoadPercent) const;

	void updateFilterParams(const QString& portName, const bool enableLff,const bool enableHff, const bool enableDSP, const double dspCutoff,const double hwLowCut, const double hwHighCut, const double lowBandwidth, const double highBandwidth, const std::string& notchFilter,
		const double Order, const std::string& Filter, const int32_t sampleRate, const int multiNotchCount) const;

	void updateDisplayParam(const double timespan, const double dispWidth, const double refreshCount, const bool isRoll) const;

	void jumpToPosition(const uint64_t targetTime) const;

	void startRecord() const;

	void restartRecord(const int timestampOffset) const;

	void stopRecord() const;

	void setVisibleWaveNamesHandle(const VisibleWaveNamesHandle& handle) { getVisibleWaveNamesFunc = handle; }

	void setLoadWaveformCallback(const LoadWaveformCallback& callback) { loadwaveformCallback = callback; }

	void setNotch(const QString& portName, const NotchType type);

	void setLowFilter(const QString& portName, const double filterValue, const int order, const FilterType type);

	void setHighFilter(const QString& portName, const double filterValue, const int order, const FilterType type);

	void setSignalInfo(WaveFormType info);

	void setSignalType(std::vector<int>signalType);

	void setLoadSelectedAmpDataCallback(const LoadSelectedAmpDataCallback& callback) { loadSelectedAmpDataCallback = callback; }

	void setTimeScale(int time);

	void setRestore(QVector<QString>  restorePath);

	bool exportData(const QString& sourcePath_, const QString& destFilePath_, const bool isSrcCompressed_, const QString& exportFileType_) const;

	void stopExport() const;

	bool isExportRunning() const;

	int getExportProgress() const;

private:
	std::unique_ptr<Recorder::Stub>							stub;
	int														watcherTimeoutCount{0};
	int														loadWaveformTimeoutCount{ 0 };

	QFuture<void>											waveformDataReaderThread;
	VisibleWaveNamesHandle									getVisibleWaveNamesFunc;
	LoadWaveformCallback									loadwaveformCallback;
	OscillographMode										oscillographMode;
	bool													isLoading;

	LoadSelectedAmpDataCallback								loadSelectedAmpDataCallback;
	QFuture<void>											spectrogramDataReaderThread;
	bool													isSpectrogramDataLoading;
};

END_NX_NAMESPACE

#endif
