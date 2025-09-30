#include "RecorderClient.h"
#include "RecorderClientImpl.h"
#include <QString>

BEGIN_NX_NAMESPACE

RecorderClient::RecorderClient(const std::string& address)
	:impl(new RecorderClientImpl(grpc::CreateChannel(
		address, grpc::InsecureChannelCredentials())))
{
}

RecorderClient::~RecorderClient()
{
	delete impl;
}

void RecorderClient::setMasterId(const std::string processId) const
{
	impl->setMasterId(processId);
}

bool RecorderClient::isControllerConnected() const
{
	return impl->isControllerConnected();
}

bool RecorderClient::initBoard(const InitInfo& initInfo, QMap<QString, QString>& channelInfos) const
{
	return impl->initBoard(initInfo, channelInfos);
}

void RecorderClient::resetSampleRate(const int sampleRate) const
{
    impl->resetSampleRate(sampleRate);
}

void RecorderClient::closeController() const
{
    impl->closeController();
}

void RecorderClient::setStoragePath(const QString& rhdFilePath) const
{
	impl->setStoragePath(rhdFilePath);
}

void RecorderClient::setImpedanceFilePath(const QString& impedanceFilePath) const
{
	impl->setImpedanceFilePath(impedanceFilePath);
}

std::string RecorderClient::getPID() const
{
	return impl->getPID();
}

std::string RecorderClient::getVID() const
{
	return impl->getVID();
}

std::string RecorderClient::getDeviceID() const
{
	return impl->getDeviceID();
}

void RecorderClient::stopLoadWaveformStream()
{
	impl->stopLoadWaveformStream();
}

void RecorderClient::loadWaveformData()
{
	impl->loadWaveformData();
	impl->loadSelectedAmpData();
}

bool RecorderClient::isServerAvailable() const
{
	return impl->isServerAvailable();
}

void RecorderClient::asyncSetCommand(QString parameter, QString value) const
{
	impl->asyncSendSetCommand(parameter, value);
}

void RecorderClient::syncSendSetCommand(const QString& parameter,const QString& value) const
{
	impl->syncSendSetCommand(parameter, value);
}

void RecorderClient::asyncSendExecuteCommand(QString action) const
{
	impl->asyncSendExecuteCommand(action);
}

void RecorderClient::syncSendExecuteCommand(const QString& action) const
{
	impl->syncSendExecuteCommand(action);
}

std::vector<std::string> RecorderClient::getDisplayListAmplifiers(const std::string& groupName) const
{
	return impl->getDisplayListAmplifiers(groupName);
}

void RecorderClient::getImpedanceInfos(const QStringList& enabledPorts, QVector<ImpedanceInfo>& impedanceInfos) const
{
	impl->getImpedanceInfos(enabledPorts,impedanceInfos);
}

void RecorderClient::getImpedanceMeasureStatus(bool& isCompleted, int& progress)
{
	impl->getImpedanceMeasureStatus(isCompleted, progress);
}

void RecorderClient::getHeatScales(QMap<QString, float>& heatScales) const
{
	return impl->getHeatScales(heatScales);
}

void RecorderClient::enableMeasureHeatScale(const bool enable) const
{
	impl->enableMeasureHeatScale(enable);
}

void RecorderClient::setHeatScaleChannels(const QStringList& channelNames) const
{
	impl->setHeatScaleChannels(channelNames);
}


void RecorderClient::getBufferStatus(double& hdBufferPercent, double& swBufferPercent, double& cpuLoadPercent) const
{
	impl->getBufferStatus(hdBufferPercent, swBufferPercent, cpuLoadPercent);
}

void RecorderClient::updateFilterParams(const QString& portName, const bool enableLff, const bool enableHff, const bool enableDSP, const double dspCutoff, const double hwLowCut, const double hwHighCut, const double lowBandwidth, const double highBandwidth,
	const std::string& notchFilter, const double Order, const std::string& Filter, const int32_t sampleRate, const int multiNotchCount) const
{
	impl->updateFilterParams(portName, enableLff, enableHff, enableDSP, dspCutoff, hwLowCut, hwHighCut, lowBandwidth, highBandwidth, notchFilter, Order, Filter, sampleRate, multiNotchCount);
}

void RecorderClient::updateDisplayParam(const double timespan, const double dispWidth, const double refreshCount, const bool isRoll)
{
	impl->updateDisplayParam(timespan, dispWidth, refreshCount, isRoll);
}

void RecorderClient::jumpToPosition(const uint64_t targetTime) const
{
	impl->jumpToPosition(targetTime);
}

void RecorderClient::startRecord() const
{
	impl->startRecord();
}

void RecorderClient::restartRecord(const int timestampOffset) const
{
	impl->restartRecord(timestampOffset);
}

void RecorderClient::stopRecord() const
{
	impl->stopRecord();
}

void RecorderClient::changeOsMode(const OscillographMode& mode)
{
	impl->changeOsMode(mode);
}

void RecorderClient::setSelectedWaveName(const QString& nativeWavename)
{
	impl->setSelectedWaveName(nativeWavename);
}

void RecorderClient::setVisibleWaveNamesHandle(const VisibleWaveNamesHandle& handle)
{
	impl->setVisibleWaveNamesHandle(handle);
}

void RecorderClient::setLoadWaveformCallback(const LoadWaveformCallback& callback)
{
	impl->setLoadWaveformCallback(callback);
}

void  RecorderClient::setSignalInfo(WaveFormType info)
{
	impl->setSignalInfo(info);
}
void RecorderClient::setSignalType(std::vector<int> signalType)
{
	impl->setSignalType(signalType);
}

void RecorderClient::setNotch(const QString& portName, const NotchType type)
{
	impl->setNotch(portName, type);
}

void RecorderClient::setLowFilter(const QString& portName, const double filterValue, const int order, const FilterType type)
{
	impl->setLowFilter(portName, filterValue, order, type);
}

void RecorderClient::setHighFilter(const QString& portName, const double filterValue, const int order, const FilterType type)
{
	impl->setHighFilter(portName, filterValue, order, type);
}

void RecorderClient::setTimeScale(int time)
{
	impl->setTimeScale(time);
}

void RecorderClient::setRestore(QVector<QString> restorePath)
{
	impl->setRestore(restorePath);
}

void RecorderClient::setLoadSelectedAmpDataCallback(const LoadSelectedAmpDataCallback& callback) const
{
	impl->setLoadSelectedAmpDataCallback(callback);
}

bool RecorderClient::exportData(const QString& sourcePath_, const QString& destFilePath_, const bool isSrcCompressed_,
    const QString& exportFileType_) const
{
	return impl->exportData(sourcePath_, destFilePath_, isSrcCompressed_, exportFileType_);
}

void RecorderClient::stopExport() const
{
	impl->stopExport();
}

bool RecorderClient::isExportRunning() const
{
	return impl->isExportRunning();
}

int RecorderClient::getExportProgress() const
{
	return impl->getExportProgress();
}


END_NX_NAMESPACE
