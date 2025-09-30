#pragma once

#include <QFile>
#include <QMutex>
#include <QtWidgets/QMainWindow>
#include "ui_G3_BleTest.h"
#include "../NxDataStructure/RhxGlobals.h"
#include <QTextBrowser>
#include <QTextStream>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QRandomGenerator>
#include <QtMath>
#include "SelectChannel.h"
#include "CLoadingScreen.h"
#include "CShowCurrentChannelWave.h"
#include"../NxG3ECU/EcuProtocol/G3_Protocol.h"

#include "SerialAssistant.h"
class G3Controller;
class IController;
class G3_ECU;
class G3_BleTest : public QMainWindow
{
    Q_OBJECT

public:
    G3_BleTest(QWidget *parent = nullptr);
    ~G3_BleTest();
    G3Controller* getRhxController(ControllerType type, AmplifierSampleRate sampleRate, AcquisitionMode mode);
    void startRun();
    void stopRun();


private  slots:
    void slotConnectBleConnect();
    void slotSetBleMessage(QString message);
    void slotSetBlTemperature(QString message);
    void slotSetBlePower(QString message);
    void slotSetBleRssi(QString message);
    void slotBleIsConnect(bool  isConnect);
    void slotUpdateProgress();
    void slotPrintReport();
    void slotGetBlTemperature();
    void slotGetBlPower();
    void slotGetBlRssi();
    void slotGetDischargeChargeCurrent();
    void slotGetBoardTemp();
    void slotGetProbeCurrent();
    void slotGetPtxVddcVoltage();
    void slotGetIcuBatteryVoltage();
    void slotUpdateICU_BLE();
    void slotUpdateICU_STM32();
    void slotUpdateICU_FPGA();
    void slotOpenSTM32File();
    void slotOpenFPGAFile();
    void slotOpenBLEFile();
    void slotGetBleVersion();
    void slotGetSTM32Version();
    void slotGetFpgaVersion();
    void slotSetDeviceInfo();
    void slotGetDeviceInfo();
    void slotSetTimeOutTime();
    void slotSetPowerOff();
    void slotStartimpedance();
    void slotReadRegValue();
    void slotWriteRegValue();
    void slotGetSensorData();
    void slotSetSensorData();
    void slotGetLog();
    void slotGetCRLog();
    void slotGetLogSize();
    void slotClearLog();
	void slotRestIcu();
    void slotSetCurrentTime();
    void slotGetCurrentTime();
	void slotGetSOH();
	void slotUpdateSuccess(bool);
    void slotCalibrateGauge();
    void slotOpenLogFile();
	void slotAnsysGetLog();
signals:
    void showChannel(int x,double y);
    void clearWave();
    void setCurrentWindowsTitle(QString title);
    void closeShowChannel();
private:
    void clearReport();
    void reportView();
    void showDelayVector(QVector<int>delay);
    void currentIndex();
    void slotSetChannel(int channelNumber, bool isCheck);
	bool checkSensorData(SensorData  &data);

private:
    Ui::G3_BleTestClass ui;
    G3_ECU* dev;
    G3Controller* controller;
    CSelectChannel* mySelectChannel;
    QTimer* timer;
    float progress;
	int connectFalseCount  = 0;
    int totalSampleTime = 0;
    double packetLoss = 0.0;
    double errorRate = 0.0;
    int  averageSpeed = 0;
    int sampleRate = 0;
    QString strHtml;
    bool isAppMode;
    std::string STM32FileName;
    std::string FPGAFileName; 
    std::string BLEFileName;
    bool isSample = false;
    uint8_t frameData;
    int startRecord = 0;
	bool isConnect = false;
    uint64_t channels = 0;  // 初始化所有 64 个通道为 0（全部关闭）
    QTimer* drawTimer;
    void updatePlot(double data);//QVector<double>data );
    void updateChart();
    QString setStatus(int num);
    double simulatedData;
    int oldNum = 0;
    QMutex mutex_;
	int currentChannel = 0;
    CLoadingScreen* loadingScreen;
    CShowCurrentChannelWave* showCurrentChannelWave;
    // 设置某个通道状态为 1（开启）
    void setChannel(uint64_t* channels, int index);
    void clearChannel(uint64_t* channels, int index);
    int isChannelEnabled(uint64_t channels, int index);
    void burnProgram();
    QThread* loadingScreenThread;

    void parseJsonConfig(const QString& filePath);
    int testMode;
    QString fpgaPath;
    QString stm32Path;
    bool fpgaBurn;
    bool stm32Burn;
    int intervalTime;
    QTimer* Burn;
    bool isBurning = false;
    std::string BLE_UUID;
    std::string ICU_NAME;
    std::string userName;
    bool useUpdate;
	SerialAssistant* serialAssistant = nullptr;
	QString logFilePath;
    int ChannelNum = 0;
	int range_x = 5000;
	int range_y = 2000;
    QThread* workerThread;
};
