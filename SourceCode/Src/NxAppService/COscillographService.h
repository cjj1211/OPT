/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_OSCILLOGRAPH_SERVICE_H
#define __C_OSCILLOGRAPH_SERVICE_H
#include "nxappservice_global.h"
#include "IAppService.h"
#include "NxCommunication/RecorderClient.h"
#include <QObject>
#include <QTime>
#include <QMap>
#include <QFuture>
#include <memory>
#include <functional>
#include <vector>

BEGIN_NX_NAMESPACE

class RecorderClient;
class DiscreteList;
class CCompressThread;
class CChannelPlanRepository;
class CPatientRepository;
class CChannelNameRepository;
class CTreatmentRecordRepository;
class CMasterServer;
struct ChannelPlan;
struct ChannelName;
struct TreatmentRecord;
struct Patient;

enum RunStatus {
    RS_RUNNING,
    RS_RECORDING,
    RS_IDLE
};

enum RecorderType
{
    RT_RECORDER,
    RT_PLAYBACK
};

enum AcquisitionMode {
    LiveMode,
    SyntheticMode,
    PlaybackMode,
    IcuMode
};

enum OscillographMode;

typedef std::function<QString(void)> SelectedWaveNameHandle;

typedef std::function<QSet<QString>(void)> VisibleWaveNamesHandle;

typedef std::function<void(
    bool isSweepFirstTime,
    bool isUseVerticalLines,
    int validZoneIndex,
    uint64_t startTimestamp,
    uint64_t endTimestamp,
    bool isRunning,
    QVector<QMap<std::string, std::string>>)> LoadWaveformCallback;

typedef std::function<void(const QString& nativeName, int numSamples, const QVector<float>& data,
    const QVector<uint64_t>& timestamps)> LoadSelectedAmpDataCallback;

class NX_APP_SERVICE_EXPORT COscillographService: public IAppService
{
    Q_OBJECT
public:
    COscillographService(QString patientUid, QObject* parent = nullptr);
    virtual ~COscillographService() override;
    void setRestore(QVector<QString>  restorePath);
    // 实时检测相关操作
    bool initBoard(const int sampleRate);
    void startRun();
    void stopRun();
    void startRecord();
    void stopRecord();
    void updateDisplayParam(const double timespan, const double dispWidth, const double refreshCount, const bool isRoll) const;
    
    // 回放相关操作
    bool isPlayback() const { return recorderType == RT_PLAYBACK; }
    void initPlayback(const int sampleRate);
    void setPlaybackStopFunc(const std::function<void()>& func) const;
    bool getRecordInfo(TreatmentRecord& recordInfo) const;
    void jumpToSecs(const uint64_t time) const;
    void exportToDir(const QString& outDir);

    RunStatus getRunStatus() const { return runStatus; }
    QDateTime getRecordStartDateTime() const { return recordStartDatetime; }
    uint64_t getRuntimeInMSecs() const { return runtimeInMSecs; }
    uint64_t getRuntimeInSecs() const { return runtimeInMSecs / 1000; }
    void setRuntime(const uint64_t mS) { runtimeInMSecs = mS; }

    void changeOsMode(const OscillographMode& mode) const;
    void setVisibleWaveNamesHandle(const VisibleWaveNamesHandle& handle) const;
    void setSelectedWaveName(const QString& nativeWaveName) const;
    void setLoadSelectedAmpDataCallback(const LoadSelectedAmpDataCallback& callback) const;

    // 数据库操作
    ChannelPlan getChannelPlanByUid(const QString& uid) const;
    virtual std::vector<ChannelPlan> getChannelPlans() const;
    std::vector<ChannelName> getChannelNamesByPlan(const QString& plansUid) const;
    void updateChannelPlan(const ChannelPlan& plan) const;
    QPoint getChannelPosition(const QString& channelPlanUid, const QString& customName) const;
    ChannelName getChannelByChannelPosition(const QString& channelPlanUid, const int x, const int y, const QString& index) const;
    
    void getCurrentPatient(Patient& patient) const;
    void updatePatient(const Patient& patient) const;
    QString getPatientDataPath() const { return patientDataPath; }
    QString getImpedanceDataPath() const { return patientDataPath + "/Impedance"; }

    void setSignalInfo(const WaveFormType& info) const;
    void setSignalType(const std::vector<int>& signalType) const;
    void setTimeScale(int time) const;
    void setNotch(const QString& portName, const QString& type) const;
    void setLowFilter(const QString& portName, const double filterValue) const;
    void setHighFilter(const QString& portName, const double filterValue) const;
    void updateFilterParam(const ChannelPlan& channelPlan) const;
    QString getEventSavePath();

    void getImpedanceInfos(const QStringList& enabledPorts, QVector<ImpedanceInfo>& impedanceInfos) const;
    void getImpedanceMeasureStatus(bool& isCompleted, int& progress) const;

    void getHeatScales(QMap<QString, float>& heatScales) const;
    void enableMeasureHeatScale(const bool enable) const;
    void setHeatScaleChannels(const QStringList& channelNames) const;

    // windows服务管理
    static bool restartServiceByName(const std::string& strDestServiceName);

    // 进程管理
    void createProcess();

    static bool findProcessById(const DWORD processId);
    bool killProcess(const DWORD processId);
    void shutdownProcess();
    void recorderProcessWatcher();

    // Export data
    bool exportData(const QString& destFilePath_, const QString& exportFileType_) const;

    void stopExport() const;

    bool isExportRunning() const;

    int getExportProgress() const;

signals:
    void signalBoardInited(bool res, QMap<QString, QString> infos);
    void signalUpdateSelectedAmpData(int numSamples,  QVector<float> data,  QVector<uint64_t> timestamps);
    void signalUpdateWaveform(
        bool isSweepFirstTime,
        bool isUseVerticalLines,
        int validDataIndex, 
        uint64_t startTimestamp, 
        uint64_t endTimestamp, 
        bool isRunning,
        QVector<QMap<std::string, std::string>> waveformData);
    void signalRecorderProcessExited();
    void signalCompressProgress(QString fileName, int currentIndex, int allIndex);
    //void signalExtractProgress(QString fileName, int currentIndex, int allIndex);

private:
    void syncSendSetCommand(const QString& parameter, const QString& value) const;
    void asyncSendSetCommand(const QString& parameter, const QString& value) const;
    void insertTrainRecord() const;

protected:
    RunStatus runStatus;
    RecorderType recordType;
    CCompressThread* compressThread;
    std::unique_ptr<RecorderClient> recorderClient;
    std::unique_ptr<CMasterServer> masterServer;
    std::unique_ptr<CChannelPlanRepository> channelPlanRepository;
    std::unique_ptr<CPatientRepository> patientRepository;
    std::unique_ptr<CChannelNameRepository> channelNameRepository;
    std::unique_ptr<CTreatmentRecordRepository> treatmentRecordRepository;
    QDateTime recordStartDatetime;
    QString currentPatientUid;
    QString patientDataPath;
    QString trainDataPath;
    QString currentPatientEventPath="";
    
    uint64_t runtimeInMSecs;
    QFuture<void> watcherThread;
    volatile bool processWatcherRunning;
    volatile bool measuringImpedance{ false };
    RecorderType   recorderType;
    DWORD   recorderProcessId{ 0 };

    // FOR PLAYBACK
    QString recordUid;
};

END_NX_NAMESPACE

#endif