#ifndef __RECORDER_CLIENT_H
#define __RECORDER_CLIENT_H
#include "nxcommunication_global.h"
#include <memory>
#include <string>
#include <functional>
#include <QString>
#include <QMap>

BEGIN_NX_NAMESPACE

struct InitInfo {
	int SampleRate;
	int AcquisitionMode;
	QString SaveFilePath;
	QString SaveFileType;
	bool UseGpu;
	bool CompressData;
};

class DisplayDto;
enum OscillographMode;

struct DisplayWaveformParam
{
    int numRefreshZones;

    // Calculated display data parameters
    int zoneWidthInPixels;
    int samplesPerZone;
    bool useVerticalLines;
    int length;
    int zoneLength;
};

struct ImpedanceInfo
{
	double magnitude;
	double phase;
	bool isValid;
	std::string nativeChannelName;
};

enum  WaveFormType
{
	WAVEFORM=0,
	SNR,
    RMS,
	SPECTRUM

};

enum NotchType {
	NT_NONE,
	NT_50Hz,
	NT_60Hz
};

enum FilterType {
	Bessel,
	Butterworth
};

typedef std::function<QSet<QString>(void)> VisibleWaveNamesHandle;

typedef std::function<QString(void)> SelectedWaveNameHandle;

typedef std::function<void(
	bool isSweepFirstTime,
	bool isUseVerticalLines,
	int validDataIndex,
	uint64_t startTimestamp,
	uint64_t endTimestamp,
	bool isRunning,
	QVector<QMap<std::string, std::string>>)> LoadWaveformCallback;

typedef std::function<void(const QString& nativeName, int numSamples, const QVector<float>& data,
	const QVector<uint64_t>& timestamps)> LoadSelectedAmpDataCallback;

class NXCOMMUNICATION_EXPORT RecorderClient
{
public:
	RecorderClient(const std::string& address);
	~RecorderClient();

	void setMasterId(const std::string processId) const;

	bool isControllerConnected() const;

	bool initBoard(const InitInfo& initInfo, QMap<QString, QString>& channelInfos) const;

	void resetSampleRate(const int sampleRate) const;

	void closeController() const;

	void setStoragePath(const QString& rhdFilePath) const;

	void setImpedanceFilePath(const QString& impedanceFilePath) const;

	std::string getPID() const;

	std::string getVID() const;

	std::string getDeviceID() const;

	void stopLoadWaveformStream();

	void loadWaveformData();

	bool isServerAvailable() const;

	void asyncSetCommand(QString parameter, QString value) const;

	void syncSendSetCommand(const QString& parameter, const QString& value) const;

	void asyncSendExecuteCommand(QString action) const;

	void syncSendExecuteCommand(const QString& action) const;

	std::vector<std::string> getDisplayListAmplifiers(const std::string& groupName) const;

	void getImpedanceInfos(const QStringList& enabledPorts, QVector<ImpedanceInfo>& impedanceInfos) const;

	void getImpedanceMeasureStatus(bool& isCompleted, int& progress);

	void getHeatScales(QMap<QString, float>& heatScales) const;

	void enableMeasureHeatScale(const bool enable) const;

	void setHeatScaleChannels(const QStringList& channelNames) const;

	void getBufferStatus(double& hdBufferPercent, double& swBufferPercent, double& cpuLoadPercent) const;

	void updateFilterParams(const QString& portName, const bool enableLff, const bool enableHff, const bool enableDSP, const double dspCutoff, const double hwLowCut, const double hwHighCut, const double lowBandwidth, const double highBandwidth,
		const std::string& notchFilter  ,const double Order, const std::string& Filter, const int32_t sampleRate, const int multiNotchCount) const;

	void updateDisplayParam(const double timespan, const double dispWidth, const double refreshCount, const bool isRoll);

	void jumpToPosition(const uint64_t targetTime) const;

	void startRecord() const;

	void restartRecord(const int timestampOffset) const;

	void stopRecord() const;

	void changeOsMode(const OscillographMode& mode);

	void setSelectedWaveName(const QString& nativeWavename);

	void setVisibleWaveNamesHandle(const VisibleWaveNamesHandle& handle);

	void setLoadWaveformCallback(const LoadWaveformCallback& callback);

	void setLoadSelectedAmpDataCallback(const LoadSelectedAmpDataCallback& callback) const;

	void setSignalInfo(WaveFormType info);

	void setSignalType(std::vector<int>signalType);

	void setNotch(const QString& portName, const NotchType type);

	void setLowFilter(const QString& portName, const double filterValue, const int order, const FilterType type);

	void setHighFilter(const QString& portName, const double filterValue, const int order, const FilterType type);

	void setTimeScale(int time);

	void setRestore(QVector<QString>  restorePath);

	bool exportData(const QString& sourcePath_, const QString& destFilePath_, const bool isSrcCompressed_, const QString& exportFileType_) const;

	void stopExport() const;

	bool isExportRunning() const;

	int getExportProgress() const;

private:
	class RecorderClientImpl;
	RecorderClientImpl*			 impl;
};

END_NX_NAMESPACE
#endif