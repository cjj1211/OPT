#pragma once

#include <G3_Protocol.h>
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

class G3Controller;
class IController;
class G3_ECU;
struct SystemCfg
{

    uint64_t address;
};
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
    void slotUpadteICU_BLE();
    void slotUpadteICU_STM32();
    void slotUpadteICU_FPGA();
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

private:
    Ui::G3_BleTestClass ui;
    G3_ECU* dev=nullptr;
    G3Controller* controller;
    CSelectChannel* mySelectChannel;
    QTimer* timer;
    float progress;
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
    QThread* loadingScreenThread;
    FrameHandler ecuFrameHandlers[FRAME_HANDLER_NUM] = { NULL };
    void connectSlot();
    void readJSon();
    SystemCfg systemCfg;
};
