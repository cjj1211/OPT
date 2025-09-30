#include "G3_BleTest.h"
#include<QPrinterInfo>
#include "../NxG3Controller/CG3Controller.h"
#include "../NxControllerInterface/IController.h"
#include "../NxG3ECU/G3_ECU.h"
#include <QFileDialog>
#include <QFile>
#include <QCryptographicHash>
#include <QDebug>
#include <QTextBrowser>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <CLogger.h>
#include<QMessageBox>
using namespace NX;
G3_BleTest::G3_BleTest(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	serialAssistant = new SerialAssistant();
	setWindowTitle(QString::fromLocal8Bit("Optimus升级测试工具"));
	connect(ui.start, &QPushButton::clicked, this, &G3_BleTest::startRun);
	connect(ui.stop, &QPushButton::clicked, this, &G3_BleTest::stopRun);
	connect(ui.bleConnect, &QPushButton::clicked, this, &G3_BleTest::slotConnectBleConnect);
	connect(ui.temperature, &QPushButton::clicked, this, &G3_BleTest::slotGetBlTemperature);
	connect(ui.rssi, &QPushButton::clicked, this, &G3_BleTest::slotGetBlRssi);
	connect(ui.icuPower, &QPushButton::clicked, this, &G3_BleTest::slotGetBlPower);
	connect(ui.print, &QPushButton::clicked, this, &G3_BleTest::slotPrintReport);
	connect(ui.discharge_charge_ma, &QPushButton::clicked, this, &G3_BleTest::slotGetDischargeChargeCurrent);
	connect(ui.probe_current, &QPushButton::clicked, this, &G3_BleTest::slotGetProbeCurrent);
	connect(ui.icu_battery_voltage, &QPushButton::clicked, this, &G3_BleTest::slotGetIcuBatteryVoltage);
	connect(ui.ptx_vddc_voltage, &QPushButton::clicked, this, &G3_BleTest::slotGetPtxVddcVoltage);
	connect(ui.updateSTM32, &QPushButton::clicked, this, &G3_BleTest::slotUpdateICU_STM32);
	connect(ui.update_BLE, &QPushButton::clicked, this, &G3_BleTest::slotUpdateICU_BLE);
	connect(ui.update_FPGA, &QPushButton::clicked, this, &G3_BleTest::slotUpdateICU_FPGA);
	connect(ui.chooseSTM32, &QPushButton::clicked, this, &G3_BleTest::slotOpenSTM32File);
	connect(ui.chosseLog, &QPushButton::clicked, this, &G3_BleTest::slotOpenLogFile);
	connect(ui.logPlot, &QPushButton::clicked, this, &G3_BleTest::slotAnsysGetLog);
	connect(ui.choose_FPGA, &QPushButton::clicked, this, &G3_BleTest::slotOpenFPGAFile);
	connect(ui.selectComboBox, &QComboBox::currentTextChanged, this, &G3_BleTest::currentIndex);
	connect(ui.STM32Version, &QPushButton::clicked, this, &G3_BleTest::slotGetSTM32Version);
	connect(ui.FPGAversion, &QPushButton::clicked, this, &G3_BleTest::slotGetFpgaVersion);
	connect(ui.BLEversion, &QPushButton::clicked, this, &G3_BleTest::slotGetBleVersion);
	connect(ui.setDeviceInfo, &QPushButton::clicked, this, &G3_BleTest::slotSetDeviceInfo);
	connect(ui.getDeviceInfo, &QPushButton::clicked, this, &G3_BleTest::slotGetDeviceInfo);
	connect(ui.setSenorData, &QPushButton::clicked, this, &G3_BleTest::slotSetSensorData);
	connect(ui.getSenorData, &QPushButton::clicked, this, &G3_BleTest::slotGetSensorData);
	connect(ui.sendCurrentTime, &QPushButton::clicked, this, &G3_BleTest::slotSetCurrentTime);
	connect(ui.getFWtime, &QPushButton::clicked, this, &G3_BleTest::slotGetCurrentTime);
	connect(ui.getSOH, &QPushButton::clicked, this, &G3_BleTest::slotGetSOH);
	ui.progressBar->setVisible(false);
	showMaximized();
	parseJsonConfig("./system.json");
	timer = new QTimer();
	//connect(timer, &QTimer::timeout, this, &G3_BleTest::slotUpdateProgress);
	ui.temperature->setEnabled(true);
	ui.rssi->setEnabled(true);
	ui.icuPower->setEnabled(true);
	ui.start->setEnabled(true);
	ui.discharge_charge_ma->setEnabled(true);
	ui.probe_current->setEnabled(true);
	ui.icu_battery_voltage->setEnabled(true);
	ui.ptx_vddc_voltage->setEnabled(true);
	ui.stop->setEnabled(true);
	ui.selectComboBox->setVisible(false);;
	// 创建定时器，设置5秒更新一次
	drawTimer = new QTimer(this);
	connect(drawTimer, &QTimer::timeout, this, &G3_BleTest::updateChart);
	mySelectChannel = new CSelectChannel();
	connect(mySelectChannel, &CSelectChannel::signalSelectChannel, this, &G3_BleTest::slotSetChannel);
	connect(ui.selectChannel, &QPushButton::clicked, [this]
	{
		mySelectChannel->show();
	}
	);
	loadingScreen = new CLoadingScreen();
	// 创建线程和工作对象
	workerThread = new QThread(this);
	showCurrentChannelWave = new CShowCurrentChannelWave();
	showCurrentChannelWave->moveToThread(workerThread);
	showCurrentChannelWave->rangeMaxY = range_y;
	showCurrentChannelWave->rangeMinX = range_x;	
	connect(this, &G3_BleTest::showChannel, showCurrentChannelWave, &CShowCurrentChannelWave::setData);
	connect(this, &G3_BleTest::clearWave, showCurrentChannelWave, &CShowCurrentChannelWave::clearWave);
	connect(this, &G3_BleTest::setCurrentWindowsTitle, showCurrentChannelWave, &CShowCurrentChannelWave::setCurrentWindowsTitle);
	connect(this, &G3_BleTest::closeShowChannel, showCurrentChannelWave, &CShowCurrentChannelWave::closeShowChannel);
	connect(ui.sleepMode, &QPushButton::clicked, this, &G3_BleTest::slotSetPowerOff);
	connect(ui.timeOut, &QPushButton::clicked, this, &G3_BleTest::slotSetTimeOutTime);
	connect(ui.impedance, &QPushButton::clicked, this, &G3_BleTest::slotStartimpedance);
	connect(ui.write, &QPushButton::clicked, this, &G3_BleTest::slotWriteRegValue);
	connect(ui.read, &QPushButton::clicked, this, &G3_BleTest::slotReadRegValue);
	connect(ui.getLog, &QPushButton::clicked, this, &G3_BleTest::slotGetLog);
	connect(ui.getCrLog, &QPushButton::clicked, this, &G3_BleTest::slotGetCRLog);
	connect(ui.getLogAccout, &QPushButton::clicked, this, &G3_BleTest::slotGetLogSize);
	connect(ui.clearLog, &QPushButton::clicked, this, &G3_BleTest::slotClearLog);
	connect(ui.resetIcu, &QPushButton::clicked, this, &G3_BleTest::slotRestIcu);
	connect(ui.calibrateGauge, &QPushButton::clicked, this, &G3_BleTest::slotCalibrateGauge);
	ui.ptx_vddc_voltage->setVisible(false);
	ui.ptx_vddc_voltageMssage->setVisible(false);
	Burn = new QTimer();
	connect(Burn, &QTimer::timeout, this, &G3_BleTest::burnProgram);
	
	if (testMode)
	{
		Burn->start(1000);
	}

	if (userName != "YJYFB" && useUpdate != true)
	{
		ui.temperature->setEnabled(false);
		ui.rssi->setEnabled(false);
		ui.icuPower->setEnabled(false);
		ui.start->setEnabled(false);
		ui.discharge_charge_ma->setEnabled(false);
		ui.probe_current->setEnabled(false);
		ui.icu_battery_voltage->setEnabled(false);
		ui.ptx_vddc_voltage->setEnabled(false);
		ui.stop->setEnabled(false);
		ui.tabWidget->removeTab(1);
		ui.tabWidget->removeTab(2);
		ui.tabWidget->removeTab(3);
		ui.selectComboBox->setVisible(false);
		ui.update_FPGA->setVisible(false);
		ui.updateSTM32->setVisible(false);
		ui.STM32Version->setEnabled(false);
		ui.FPGAversion->setEnabled(false);
		ui.BLEversion->setEnabled(false);
		ui.setSenorData->setEnabled(false);
		ui.getLog->setEnabled(false);
		ui.sleepMode->setEnabled(false);
		ui.getCrLog->setEnabled(false);
		ui.getLogAccout->setEnabled(false);
		ui.resetIcu->setEnabled(false);
		ui.clearLog->setEnabled(false);
		ui.chosseLog->setEnabled(false);
		ui.logPlot->setEnabled(false);
		ui.calibrateGauge->setEnabled(false);
		ui.getSenorData->setEnabled(false);
		ui.setSenorData->setEnabled(false);
		ui.boardTemp->setEnabled(false);
	}
	if (userName != "YJYFB" && useUpdate == true)
	{
		ui.temperature->setEnabled(false);
		ui.rssi->setEnabled(false);
		ui.icuPower->setEnabled(false);
		ui.start->setEnabled(false);
		ui.discharge_charge_ma->setEnabled(false);
		ui.probe_current->setEnabled(false);
		ui.icu_battery_voltage->setEnabled(false);
		ui.ptx_vddc_voltage->setEnabled(false);
		ui.stop->setEnabled(false);
		ui.tabWidget->removeTab(1);
		ui.tabWidget->removeTab(2);
		ui.tabWidget->removeTab(3);
		ui.selectComboBox->setVisible(false);
		//ui.STM32Version->setEnabled(false);
		//ui.FPGAversion->setEnabled(false);
		//ui.BLEversion->setEnabled(false);
		//ui.setSenorData->setEnabled(false);
		ui.getLog->setEnabled(false);
		ui.sleepMode->setEnabled(false);
		ui.getCrLog->setEnabled(false);
		ui.getLogAccout->setEnabled(false);
		ui.resetIcu->setEnabled(false);
		ui.clearLog->setEnabled(false);
		ui.chosseLog->setEnabled(false);
		ui.logPlot->setEnabled(false);
		ui.calibrateGauge->setEnabled(false);
		ui.getSenorData->setEnabled(false);
		ui.setSenorData->setEnabled(false);
		ui.boardTemp->setEnabled(false);

	}
	if(userName == "YJYFB")
	{
		ui.tabWidget->removeTab(1);
		ui.tabWidget->removeTab(2);
		ui.verticalLayout_2->addWidget(serialAssistant);
		serialAssistant->show();
	}
	ui.boardTemp->setEnabled(true);
	connect(ui.boardTemp, &QPushButton::clicked, this, &G3_BleTest::slotGetBoardTemp);
	connect(workerThread, &QThread::started, showCurrentChannelWave, &CShowCurrentChannelWave::show);
}


G3_BleTest::~G3_BleTest()
{

}
G3Controller* G3_BleTest::getRhxController(ControllerType type, AmplifierSampleRate sampleRate,
	AcquisitionMode mode)
{
	LOG_INFO("G3_ECU .");
	dev = new G3_ECU(NX_G3_BLE, ICU_NAME, BLE_UUID);
	LOG_INFO("G3_ECU initialized.");
	QThread* thread = new QThread;
	thread->moveToThread(thread);
	connect(thread, &QThread::started, dev, [=]() {
		dev->openBySerial("com4");
	});
	connect(thread, &QThread::finished, dev, &QObject::deleteLater);
	connect(thread, &QThread::finished, thread, &QThread::deleteLater);
	thread->start();
	return   new G3Controller(type, sampleRate, dev);
}

/**
 * \开始采集
 */
void G3_BleTest::startRun()
{
	
	
	QStringList items;
	for (int i = 0; i < 64; i++)
	{
		if( isChannelEnabled( channels, i))
		{
			items << QString::number(i+1);
			ChannelNum++;
		}
	}
	controller->setTestMode(testMode);
	controller->selectChannel(channels);
	QThread::msleep(1000);
	ui.selectComboBox->clear();
	ui.selectComboBox->addItems(items);
	ui.selectComboBox->setVisible(true);
	
	controller->samplingControl(true);
	workerThread->start();

	qDebug() << "start sample";
}

void G3_BleTest::stopRun()
{
	timer->stop();
	workerThread->quit();
	controller->samplingControl(false);
	isSample = false;
     emit  closeShowChannel();
	channels = 0;
	qDebug() << "stop sample";

#ifdef DEBUG_MODE
	QFile file("opt.csv");  // 旧文件名
	if (file.exists()) {
		// 尝试重命名文件
		auto currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");  // 修改为合法的文件名格式
		QString fileNameReport = currentTime + ".csv";

		bool success = file.rename(fileNameReport);  // 新文件名
		if (success) {
			qDebug() << QString::fromLocal8Bit("文件重命名成功！");
		}
		else {
			qDebug() << QString::fromLocal8Bit("文件重命名失败：") << file.errorString();
		}
	}
	else {
		qDebug() << QString::fromLocal8Bit("文件不存在！");
	}
#endif

}


void G3_BleTest::slotConnectBleConnect()
{
	if (!isConnect)
	{
		controller = getRhxController(ControllerRecordUSB3, SampleRate500Hz, IcuMode);
		QObject::connect(controller, &G3Controller::singnalSetBleMessage, this, &G3_BleTest::slotSetBleMessage);
		QObject::connect(controller, &G3Controller::signalShowChannel, this, &G3_BleTest::slotUpdateProgress);
		QObject::connect(controller, &G3Controller::signalisisUpdateSuccess, this, &G3_BleTest::slotUpdateSuccess);
		QObject::connect(controller, &G3Controller::signalisConnect, this, &G3_BleTest::slotBleIsConnect);
	}
	else
	{
		controller = nullptr;
		ui.bleMessage->clear();
		ui.bleConnect->setText(QString::fromLocal8Bit("连接蓝牙"));
		ui.bleConnect->setStyleSheet("background-color: rgb(0, 255, 0);");
		isConnect = false;
	}

}

void G3_BleTest::slotSetBleMessage(QString message)
{
	ui.bleMessage->append(message);
}

void G3_BleTest::slotSetBlTemperature(QString message)
{
	ui.temperatureMessage->append(message);
}

void G3_BleTest::slotSetBlePower(QString message)
{
	ui.icuPowerMessage->append(message);
}

void G3_BleTest::slotSetBleRssi(QString message)
{
	ui.rssiMessage->append(message);
}

void G3_BleTest::slotBleIsConnect(bool isConnect)
{
	if (isConnect)
	{
		ui.bleConnect->setText(QString::fromLocal8Bit("断开连接"));
		ui.bleConnect->setStyleSheet("background-color: rgb(255, 0, 0);");
		this->isConnect = true;
		DeviceInfo deviceInfo;
		auto  bleName = controller->getDeviceInfo(deviceInfo,true);
		ui.bleName_Com->setText(QString::fromStdString(bleName));
	}
	else
	{
		ui.bleConnect->setText(QString::fromLocal8Bit("连接蓝牙"));
		ui.bleConnect->setStyleSheet("background-color: rgb(0, 255, 0);");
		this->isConnect = false;
		connectFalseCount++;
		if (connectFalseCount<=5)
		{
			delete controller ;
			slotConnectBleConnect();
		}
       

	}
}

void G3_BleTest::slotUpdateProgress()
{


#ifdef DEBUG_MODE

	QFile  file("opt.csv");
	static  int count = 0;
	/*if (count == 0)
	{
		if (file.exists())
		{
			file.remove();
		}

	}*/
	if (!file.open(QIODevice::Append | QIODevice::Text))
	{
		return;
	}
	QTextStream out(&file);  // 绑定 QDataStream 到 QFile
	controller->readRecordBlock(&frameData);
	constexpr uint64_t header = 0xd7a22aaa38132a53UL;
	constexpr uint64_t noframeheader = 0xd7a22aaa38132a57UL;
	uint64_t x1 = (&frameData)[0];
	uint64_t x2 = (&frameData)[1];
	uint64_t x3 = (&frameData)[2];
	uint64_t x4 = (&frameData)[3];
	uint64_t x5 = (&frameData)[4];
	uint64_t x6 = (&frameData)[5];
	uint64_t x7 = (&frameData)[6];
	uint64_t x8 = (&frameData)[7];
	uint64_t usbHeader = (x8 << 56) + (x7 << 48) + (x6 << 40) + (x5 << 32) + (x4 << 24) + (x3 << 16) + (x2 << 8) + (x1 << 0);
	if (usbHeader == header)
	{
		
		
		if(count!=0)
		{
			out << "\n";
		
		}
		count++;
		 //创建 QTextStream 对象来操作文件流

		   //假设 frameData 是一个数组或结构体，需要转换成 uint8_t* 进行字节访问
		uint8_t* dataPtr = (uint8_t*)&frameData;

		uint32_t temp = *(uint32_t*)(dataPtr + 8);  // 正确的字节偏移
			//qDebug() << "temp is  " << temp;

		uint16_t read_channel = *(uint16_t*)(dataPtr + 12);  // 计算偏移后读取通道号
		//qDebug() << "read_channel is " << read_channel;
		int channel = 0;
		for (int i = 14; i <=read_channel * 2 * 2 + 12; i += 2) 
		{
			uint8_t highByte = *(dataPtr + i);         // 高字节
			uint8_t lowByte = *(dataPtr + i + 1);      // 低字节
			uint16_t combined = (highByte << 8) | lowByte;
		
			// 判断是否是32通道的位置，i == 32 * 2 即第32个通道结束
			int transNubmer = read_channel*2+12;
			//qDebug() << QString::number(i);
			if ((i == transNubmer))
			{
			
				out << (combined); // 换行的输出
			//	out << (combined - 32768) * 0.195; // 换行的输出
				out << "\n";
			
				//updatePlot(simulatedData);
				channel=0;
			}
			else {
				
				
			//	out << (combined - 32768) * 0.195 << ","; // 其他情况输出并加上逗号
				out << (combined) << ",";
				channel++;
			}
		}

		file.close();

	}
	else
	{
		if (usbHeader == noframeheader)
		{

		//	qDebug() << "nofra  ";
		}

	}

# else
	{

		if (testMode == 0)
		{
			return;
		}
		if(!controller) return;
		static 	int sampleCount = 0;
		if(sampleCount>range_x)
		{
			emit clearWave();
			sampleCount = 0;
		}
	
		uint8_t* dataPtr=controller->readRecordBlock();
		for (int i = 0; i <= ChannelNum*2; i += 2)
		{
			int8_t highByte = *(dataPtr + i+1);         // 高字节
			uint8_t lowByte = *(dataPtr+i);      // 低字节
			int16_t combined = (highByte << 8) | lowByte;
			int transNubmer = 0;
			auto index = ui.selectComboBox->currentIndex();
			if (i == index|| i == index*2)
			{
				double data = combined;
				if (showCurrentChannelWave)
				{
					emit showChannel(sampleCount, data);
				}

				sampleCount += 1;
			}

		}



	/*	constexpr uint64_t header = 0xd7a22aaa38132a53UL;
		constexpr uint64_t noframeheader = 0xd7a22aaa38132a57UL;
		uint64_t x1 = (&frameData)[0];
		uint64_t x2 = (&frameData)[1];
		uint64_t x3 = (&frameData)[2];
		uint64_t x4 = (&frameData)[3];
		uint64_t x5 = (&frameData)[4];
		uint64_t x6 = (&frameData)[5];
		uint64_t x7 = (&frameData)[6];
		uint64_t x8 = (&frameData)[7];
		uint64_t usbHeader = (x8 << 56) + (x7 << 48) + (x6 << 40) + (x5 << 32) + (x4 << 24) + (x3 << 16) + (x2 << 8) + (x1 << 0);
		if (usbHeader == header)
		{
		
		
		}*/
		
	}
#endif
}

void G3_BleTest::slotPrintReport()
{
	QString content;
	QString newContent;
	// 获取可执行文件所在目录
	QString exePath = QCoreApplication::applicationDirPath();
	// 相对路径，指向exe目录的上一级目录中的config目录
	QString fileName = "../config/report.html";
	// 转换为绝对路径（基于exe所在目录）
	QString absoluteFilePath = QDir(exePath).absoluteFilePath(fileName);
	// 打开文件并读取内容
	QFile file(absoluteFilePath);
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QTextStream stream(&file);
		stream.setCodec("UTF-8");
		QString chart1Total;
		while (!stream.atEnd()) {
			content = file.readLine();
			if (content.indexOf("testTime") > -1)
			{
				content = content.replace(QStringLiteral("testTime"), ui.testTime->text());
			}

			else if (content.indexOf("testPersonnel") > -1)
			{
				content = content.replace(QStringLiteral("testPersonnel"), ui.testPersonnel->text());
			}
			else if (content.indexOf("testLong") > -1)
			{
				content = content.replace(QStringLiteral("testLong"), ui.testLong->text());
			}
			else if (content.indexOf("bleName") > -1)
			{
				content = content.replace(QStringLiteral("bleName"), ui.bleName->text());
			}
			else if (content.indexOf("bleID") > -1)
			{
				content = content.replace(QStringLiteral("bleID"), ui.bleID->text());
			}

			else if (content.indexOf("sample") > -1)
			{
				content = content.replace(QStringLiteral("sample "), ui.sample->text());
			}

			else if (content.indexOf("packetLoss") > -1)
			{
				content = content.replace(QStringLiteral("packetLoss"), ui.packetLoss->text());
			}
			else if (content.indexOf("errorRate") > -1)
			{
				content = content.replace(QStringLiteral("errorRate"), ui.errorRate->text());
			}
			else if (content.indexOf("averageSpeed") > -1)
			{
				content = content.replace(QStringLiteral("averageSpeed"), ui.averageSpeed->text());
			}
			else if (content.indexOf("textEdit") > -1)
			{
				content = content.replace(QStringLiteral("textEdit"), ui.textEdit->toPlainText());
			}

			newContent += (content);
		}
	}
	file.close();
	QDir dir(QDir(exePath).absoluteFilePath("../report"));
	if (!dir.exists()) {
		dir.mkpath(QDir(exePath).absoluteFilePath("../report"));
	}

	auto currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");  // 修改为合法的文件名格式
	QString fileNameReport = QString::fromStdString("../report") + "/" + currentTime + ".html";
	QString absoluteFilePathReport = QDir(exePath).absoluteFilePath(fileNameReport);

	QFile newFile(absoluteFilePathReport);
	QTextStream stream(&newFile);
	stream.setCodec("UTF-8");
	if (newFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
		// 文件写入操作
		stream << (newContent);
		newFile.close();
	}
	strHtml = newContent;
	QStringList strlist = QPrinterInfo::availablePrinterNames();
	//打印设备
	QPrinter Printer;// （该文档以下所有Printer都为类QPrinter 定义）
	//获取默认打印机
	Printer.setPrinterName(Printer.printerName());//设置打印机设备名称
	Printer.setPageSize(QPrinter::A4);//设置纸张大小
	Printer.setNumCopies(1);//设置打印机打印份数
	Printer.setPageMargins(6, 6, 6, 6, QPrinter::Millimeter);//设置页边距
	Printer.setOrientation(QPrinter::Portrait);//纵向：Portrait 横向：Landscape
	Printer.setPageOrder(QPrinter::FirstPageFirst);//从第一页开始：FirstPageFirst  从最后一页开始：LastPageFirst

}

void G3_BleTest::slotGetBlTemperature()
{
	auto bleTemp=controller->getBleTemperature();
	ui.temperatureMessage->append(QString::number(bleTemp) + QString::fromLocal8Bit("℃"));
}

void G3_BleTest::slotGetBlPower()
{
	auto blePower=controller->getBlePower();
	ui.icuPowerMessage->append(QString::number(blePower) + QString::fromLocal8Bit("%"));
}

void G3_BleTest::slotGetBlRssi()
{
	auto rssi = controller->getBleRssi();
	ui.rssiMessage->append(QString::number(rssi));
}

void G3_BleTest::slotGetDischargeChargeCurrent()
{
	auto rssi = controller->getDischargeChargeCurrent();
	ui.discharge_charge_ma_Mssage->append(QString::number(rssi) + QString::fromLocal8Bit("ma"));

}

void G3_BleTest::slotGetBoardTemp()
{
	auto rssi = controller->getBoardTemp();
	ui.boardTempMssage->append(QString::number(rssi) + QString::fromLocal8Bit("℃"));
}

void G3_BleTest::slotGetProbeCurrent()
{
	auto rssi = controller->getProbeCurrent();
	ui.probe_currentMssage->append(QString::number(rssi) + "ma");
}

void G3_BleTest::slotGetPtxVddcVoltage()
{
	auto rssi = controller->getPtxVddcVoltage();
	ui.ptx_vddc_voltageMssage->append(QString::number(rssi) + "V");
}

void G3_BleTest::slotGetIcuBatteryVoltage()
{
	auto rssi = controller->getIcuBatteryVoltage();
	ui.icu_battery_voltagetMssage->append(QString::number(rssi) + "V");
}

void G3_BleTest::slotUpdateICU_BLE()
{
	

// 打开文件对话框，选择本地的 HTML 文件
	QString fileName = QCoreApplication::applicationDirPath() +"/BLE_OTA/index.html";
	if (!fileName.isEmpty()) {
		// 使用 QUrl::fromLocalFile() 来加载本地 HTML 文件
		QProcess::startDetached("cmd", QStringList() << "/c" << "start" << fileName.replace("/", "\\"));
	}
	else {
		qDebug() << "No file selected";
	}
}

void G3_BleTest::slotUpdateICU_STM32()
{
	//loadingScreen->show();
	//drawTimer->start(1000);
	QString::fromLocal8Bit("文件传输中！");
	controller->updateIcuMcu(STM32FileName);

}

void G3_BleTest::slotUpdateICU_FPGA()
{
	
	//loadingScreen->show();
	qDebug() <<	QString::fromLocal8Bit("FPGA 升级中！");
	//drawTimer->start(1000);

	controller->updateFPGA(FPGAFileName);

}


void G3_BleTest::slotOpenSTM32File()
{
	// 打开文件选择对话框，过滤文件类型为 .bin
	QString filePath = QFileDialog::getOpenFileName(
		nullptr,                             // 父窗口
		QString::fromLocal8Bit("选择一个BIN文件"),                     // 对话框标题
		"",                                  // 默认路径
		"Binary Files (*.bin);;All Files (*)" // 文件过滤器
	);

	if (!filePath.isEmpty()) {
		// 获取文件名
		QFileInfo fileInfo(filePath);
		QString fileName = fileInfo.absoluteFilePath();
		ui.STM32_LE->setText(fileName);
		STM32FileName = fileName.toStdString();
	}
}

void G3_BleTest::slotOpenFPGAFile()
{
	// 打开文件选择对话框，过滤文件类型为 .bin
	QString filePath = QFileDialog::getOpenFileName(
		nullptr,                             // 父窗口
		QString::fromLocal8Bit("选择一个BIN文件"),                   // 对话框标题
		"",                                  // 默认路径
		"Binary Files (*.bin);;All Files (*)" // 文件过滤器
	);

	if (!filePath.isEmpty()) {
		// 获取文件名
		QFileInfo fileInfo(filePath);
		QString fileName = fileInfo.absoluteFilePath();
		ui.FPGA_LE->setText(fileName);
		FPGAFileName = fileName.toStdString();
	}
}

void G3_BleTest::slotOpenBLEFile()
{
	
}

void G3_BleTest::slotGetBleVersion()
{
	std::string bleVersion = controller->getIcuVersion();
	//qDebug() << QString::fromStdString(bleVersion);
	ui.BLE_VERSION->append(QString::fromStdString(bleVersion));
}

void G3_BleTest::slotGetSTM32Version()
{
	std::string stm32Version = controller->getIcuMcuVersion();
	ui.STM32_VERSION->append(QString::fromStdString(stm32Version));
}

void G3_BleTest::slotGetFpgaVersion()
{
	std::string fpgaVersion = controller->getFPGAVersion();
	ui.FPGA_VERSION->append(QString::fromStdString(fpgaVersion));
}

void G3_BleTest::slotSetDeviceInfo()
{
	DeviceInfo de;
	auto info = ui.setDeviceInfoMessage->toPlainText();
	if (QString(info[0]) + QString(info[1])!="NW")
	{
		QMessageBox messageBox;
		messageBox.setWindowTitle(QString::fromLocal8Bit("失败"));
		messageBox.setText(QString::fromLocal8Bit("成品代码设置错误"));
		messageBox.setIcon(QMessageBox::Information);
		messageBox.setStyleSheet(" QMessageBox { color: white; }");
		messageBox.exec(); 
		return;
	}
	de.deviceName = QString(info[0]) + QString(info[1]);
	de.deviceType = QString(info[2]);
	de.year = QString(info[3]) ;
	de.month = QString(info[4]) ;
	if (QString(info[5])!="A")
	{
		QMessageBox messageBox;
		messageBox.setWindowTitle(QString::fromLocal8Bit("失败"));
		messageBox.setText(QString::fromLocal8Bit("型号类型设置错误"));
		messageBox.setIcon(QMessageBox::Information);
		messageBox.setStyleSheet(" QMessageBox { color: white; }");
		messageBox.exec();
		return;
	}
	de.deviceID = QString(info[5]) + QString(info[6])+ QString(info[7])+ QString(info[8]);
	 controller->getDeviceInfo(de,false);
	 QMessageBox messageBox;
	 messageBox.setWindowTitle(QString::fromLocal8Bit("成功"));
	 messageBox.setText(QString::fromLocal8Bit("设置成功"));
	 messageBox.setIcon(QMessageBox::Information);
	 messageBox.setStyleSheet(" QMessageBox { color: white; }");
	 messageBox.exec();

}

void G3_BleTest::slotGetDeviceInfo()
{
	DeviceInfo de;
	std::string res=controller->getDeviceInfo(de, true);
	ui.getDeviceInfoMessage->append(QString::fromStdString(res));
	
}



void G3_BleTest::slotGetSensorData()
{
	controller->getSensorData();
}

void G3_BleTest::slotSetSensorData()
{
	SensorData data;
	data.ChongFang = ui.chargeLimit->text().toInt();
	data.TanTou = ui.INTANLimit->text().toInt();
	data.IcuTemp = ui.icuTemp->text().toDouble()*10;
	data.OverTemp = ui.overTemp->text().toInt();
	data.OverTempMax=ui.OverTempMax->text().toDouble() * 10;
	data.SampleTime= ui.SampleTime->text().toInt();
	if (checkSensorData(data))
	{
		//Sleep(50);
		controller->setSensorData(data);
	}
	
}

void G3_BleTest::slotGetLog()
{
	controller->getLog();
}

void G3_BleTest::slotGetCRLog()
{
	controller->getCRLog();
}

void G3_BleTest::slotGetLogSize()
{
	controller->getLogSize();
}

void G3_BleTest::slotClearLog()
{
	controller->clearLog();
}

void G3_BleTest::slotRestIcu()
{
	controller->resetIcu();

}

void G3_BleTest::slotSetCurrentTime()
{
  uint32_t now = QDateTime::currentDateTime().toSecsSinceEpoch();
  controller->setCurrentTime(now);

}

void G3_BleTest::slotGetCurrentTime()
{
	QString time=controller->getCurrentTime();
	ui.lineEdit->setText(time);
}

void G3_BleTest::slotGetSOH()
{
	int soh = controller->getPowerSOH();
	ui.sohLineEdit->setText(QString::number(soh) + "%");
}

void G3_BleTest::slotUpdateSuccess(bool val)
{
	if(val)
	{

		controller->setPowerOff();
		QMessageBox messageBox;
		messageBox.setWindowTitle(QString::fromLocal8Bit("成功"));
		messageBox.setText(QString::fromLocal8Bit("STM32 升级成功 请重新唤醒ICU 然后重启软件 ！"));
		messageBox.setIcon(QMessageBox::Information);
		messageBox.setStyleSheet(" QMessageBox { color: white; }");
		messageBox.exec();    
	}
	
}

void G3_BleTest::slotCalibrateGauge()
{
	controller->calibrateGauge();
}

void G3_BleTest::slotOpenLogFile()
{
	QString fileName = QFileDialog::getOpenFileName(
		this, QString::fromLocal8Bit("选择日志文件"),
		QCoreApplication::applicationDirPath(),
		QString::fromLocal8Bit("日志文件 (*.log);;所有文件 (*)")
	);
	if (!fileName.isEmpty()) 
	{
		logFilePath = fileName;
		ui.logPath->setText(logFilePath);
		
	}
		else 
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("错误"), QString::fromLocal8Bit("无法打开文件"));
	}
	
}

void G3_BleTest::slotAnsysGetLog()
{
	QString pythonExe = "python";  // 如果有 venv 或 Python 路径，填绝对路径
	QString scriptPath = "./plot-optimus-log.py"; // 你的 py 脚本路径
	QProcess* process = new QProcess(this);
	process->setProgram(pythonExe);
	process->setArguments({ scriptPath, logFilePath });
	// 直接启动 Python 进程
	process->startDetached();
}

void G3_BleTest::clearReport()
{
	ui.testTime->clear();
	ui.testPersonnel->clear();
	ui.testLong->clear();
	ui.bleName->clear();
	ui.bleID->clear();
	ui.sample->clear();
	ui.packetLoss->clear();
	ui.errorRate->clear();
	ui.averageSpeed->clear();
	ui.textEdit->clear();

}

void G3_BleTest::reportView()
{
	ui.testTime->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
	if (totalSampleTime == 30)
	{
		ui.testLong->setText(QString::fromLocal8Bit("30秒"));
	}
	else
	{
		ui.testLong->setText(QString::number(totalSampleTime / 60) + QString::fromLocal8Bit("分钟"));
	}
	
	ui.bleID->setText(ui.bleId_Com->currentText());
	ui.sample->setText(QString::number(sampleRate) + "HZ");
	ui.packetLoss->setText(QString::number(packetLoss) + "%");
	ui.errorRate->setText(QString::number(errorRate) + "%");
	ui.averageSpeed->setText(QString::number(averageSpeed) + "kbps/s");

}

void G3_BleTest::showDelayVector(QVector<int> delay)
{
	if (delay.isEmpty()) return;
	QString exePath = QCoreApplication::applicationDirPath();
	auto currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");  // 修改为合法的文件名格式
	QString fileNameReport = QString::fromStdString("../report") + "/" + currentTime + ".csv";
	QString absoluteFilePathReport = QDir(exePath).absoluteFilePath(fileNameReport);
	QFile file(absoluteFilePathReport);
	if (!file.open(QIODevice::WriteOnly)) {
		qWarning() << "Failed to open file for writing:" << file.errorString();
		return;
	}
	QTextStream out(&file);
	// 遍历数据并写入 CSV 文件
	for (const int& value : delay) {
		out << value << "\n";
	}

	file.close();


}

void G3_BleTest::currentIndex()
{ 
	auto index= ui.selectComboBox->currentIndex();
	currentChannel = index;

	emit setCurrentWindowsTitle(QString::fromLocal8Bit("第%1通道数据").arg(ui.selectComboBox->currentText()));
	emit clearWave();

}

void G3_BleTest::slotSetChannel(int channelNumber, bool isCheck)
{
	if(isCheck)
	{
		setChannel(&channels, channelNumber);
	}
	else
	{
		clearChannel(&channels, channelNumber);
	}
}
bool G3_BleTest::checkSensorData(SensorData &data)
{

	if (ui.enablechargeLimit->isChecked())
	{
		data.ChongFang = 0xFFFF;
	}
	else
	{
		if (data.ChongFang < 100 || data.ChongFang >200)
		{
			QMessageBox messageBox;
			messageBox.setWindowTitle(QString::fromLocal8Bit("错误"));
			messageBox.setText(QString::fromLocal8Bit("充放电限制设置错误！"));
			messageBox.setIcon(QMessageBox::Warning);
			messageBox.setStyleSheet(" QMessageBox { color: white; }");
			messageBox.exec();
			return  0;
		}
	}
	if (ui.enableINTANLimit->isChecked())
	{
		data.TanTou = 0xFFFF;
	}
	else
	{
		if (data.TanTou < 16 || data.TanTou >50)
		{
			QMessageBox messageBox;
			messageBox.setWindowTitle(QString::fromLocal8Bit("错误"));
			messageBox.setText(QString::fromLocal8Bit("探头限制设置错误！"));
			messageBox.setIcon(QMessageBox::Warning);
			messageBox.setStyleSheet(" QMessageBox { color: white; }");
			messageBox.exec();
			return  0;
		}
	}
	if (ui.enableicuTemp->isChecked())
	{
		data.IcuTemp = 0xFFFF;
	}
	else
	{
		if (data.IcuTemp/10 < 37 || data.IcuTemp/10 >41)
		{
			QMessageBox messageBox;
			messageBox.setWindowTitle(QString::fromLocal8Bit("错误"));
			messageBox.setText(QString::fromLocal8Bit("ICU温度设置错误！"));
			messageBox.setIcon(QMessageBox::Warning);
			messageBox.setStyleSheet(" QMessageBox { color: white; }");
			messageBox.exec();
			return  0;
		}
	}
	if (ui.enableoverTemp->isChecked())
	{
		data.OverTemp = 0XFFFF;
	}
	else
	{
		if (data.OverTemp < 0 || data.OverTemp >60)
		{
			QMessageBox messageBox;
			messageBox.setWindowTitle(QString::fromLocal8Bit("错误"));
			messageBox.setText(QString::fromLocal8Bit("过温持续时间设置错误！"));
			messageBox.setIcon(QMessageBox::Warning);
			messageBox.setStyleSheet(" QMessageBox { color: white; }");
			messageBox.exec();
			return  0;
		}
	}
	if (ui.enableOverTempMax->isChecked())
	{
		data.OverTempMax = 0xFFFF;
	}
	else
	{
		if (data.OverTempMax/10 < 39 || data.OverTempMax/10 >43)
		{
			QMessageBox messageBox;
			messageBox.setWindowTitle(QString::fromLocal8Bit("错误"));
			messageBox.setText(QString::fromLocal8Bit("过温极限设置错误！"));
			messageBox.setIcon(QMessageBox::Warning);
			messageBox.setStyleSheet(" QMessageBox { color: white; }");
			messageBox.exec();
			return  0;
		}
	}

	if (ui.enableSampleTime->isChecked())
	{
		data.SampleTime = 0xFFFF;
	}
	else
	{
		if (data.SampleTime < 1)
		{
			QMessageBox messageBox;
			messageBox.setWindowTitle(QString::fromLocal8Bit("错误"));
			messageBox.setText(QString::fromLocal8Bit("采样时间设置错误！"));
			messageBox.setIcon(QMessageBox::Warning);
			messageBox.setStyleSheet(" QMessageBox { color: white; }");
			messageBox.exec();
			return  0;
		}
	}
	return 1;
	
}
void G3_BleTest::slotSetTimeOutTime()
{
	controller->setTimeOut(2);
}

void G3_BleTest::slotSetPowerOff()
{
	controller->setPowerOff();
}

void G3_BleTest::slotStartimpedance()
{
	controller->startImpedance();
}

void G3_BleTest::slotReadRegValue()
{
	bool ok;
	QString addrStr = ui.read_line->text().trimmed();

	// 允许 0x 前缀，统一去掉
	if (addrStr.startsWith("0x", Qt::CaseInsensitive)) {
		addrStr = addrStr.mid(2);
	}

	// 如果输入只有 0-9 则按十进制，否则按十六进制
	int base = addrStr.contains(QRegExp("[A-Fa-f]")) ? 16 : 10;
	uint8_t address = static_cast<uint8_t>(addrStr.toUInt(&ok, base));

	if (!ok) {
		ui.read_value_line->setText("输入错误");
		return;
	}

	// 读取寄存器
	uint8_t val = controller->getFPGAReg(address);

	// 输出十六进制（两位补零，大写）
	ui.read_value_line->setText(QString("%1").arg(val, 2, 16, QChar('0')).toUpper());

}

void G3_BleTest::slotWriteRegValue()
{
	uint8_t address = ui.adress->text().toInt();
	uint32_t value = ui.value->text().toInt();
	controller->setFPGAReg(address, value);
}


void G3_BleTest::updatePlot(double channelData)
{
}

void G3_BleTest::updateChart()
{
	/*int status=controller->isUpdateSuccess();
	if(oldNum!= status)
	{
		QString text = setStatus(status);
		qDebug() << text;
		loadingScreen->setStatus(text);
		oldNum = status;
	}
	if (status==2|| status==3||status==4)
	{
		QString text = setStatus(status);
		qDebug() << text;
		drawTimer->stop();
	}*/
}

QString G3_BleTest::setStatus(int num)
{
	//switch (num)
	//{
	//case 0:
	//	
	//	return  QString::fromLocal8Bit("文件传输中！");

	//case 1:
	//	return  QString::fromLocal8Bit("文件传输成功！");
	//case 2:
	//	return  QString::fromLocal8Bit("文件传输失败！");
	//case 3:
	//	return  QString::fromLocal8Bit("升级失败！");
	//case 4:
	//	return  QString::fromLocal8Bit("升级成功！");
	//case 5:
	//	return  QString::fromLocal8Bit("升级中！");
	//default: return  QString::fromLocal8Bit("文件传输开始！");
	//}
	return   QString();
}
// 设置某个通道状态为 1（开启）
void G3_BleTest::setChannel(uint64_t* channels, int index) {
	*channels |= (1ULL << index);
}

// 清除某个通道状态为 0（关闭）
void G3_BleTest::clearChannel(uint64_t* channels, int index) {
	*channels &= ~(1ULL << index);
}

// 检查某个通道是否为 1
int G3_BleTest::isChannelEnabled(uint64_t channels, int index) {
	return (channels & (1ULL << index)) ? 1 : 0;
}

void G3_BleTest::burnProgram()
{
	if(!isBurning)
	{
		if (controller)
		{

			if (fpgaBurn)
			{
				isBurning = true;
				QDir dir(fpgaPath);
				if (!dir.exists()) {
					qDebug() << "Folder does not exist!";
					return;
				}

				QStringList filters;
				filters << "*.bin";  // 只筛选 .bin 文件
				QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files | QDir::NoDotAndDotDot);
				for (const QFileInfo& file : fileList) {
					qDebug() << file.absoluteFilePath();
					controller->updateFPGA(file.absoluteFilePath().toStdString());
					QThread::sleep(intervalTime * 60);

				}
			}
			if (stm32Burn)
			{
				isBurning = true;
				
					QDir dir(stm32Path);
					if (!dir.exists()) {
						qDebug() << "Folder does not exist!";
						return;
					}

					QStringList filters;
					filters << "*.bin";  // 只筛选 .bin 文件
					QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files | QDir::NoDotAndDotDot);
					for (const QFileInfo& file : fileList) {
						qDebug() << file.absoluteFilePath();
						controller->updateFPGA(file.absoluteFilePath().toStdString());
						QThread::sleep(intervalTime * 60);

					}
				}
			

		}
	}
	isBurning = false;
}

void G3_BleTest::parseJsonConfig(const QString& filePath)
{
	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qDebug() << "Failed to open file:" << filePath;
		return;
	}

	QByteArray jsonData = file.readAll();
	file.close();

	QJsonDocument doc = QJsonDocument::fromJson(jsonData);
	if (doc.isNull() || !doc.isObject()) {
		qDebug() << "Invalid JSON format";
		return;
	}

	 QJsonObject obj = doc.object();
	 testMode = obj.value("test_mode").toInt();
	 fpgaPath = obj.value("fpga_file_path").toString();
	 stm32Path = obj.value("stm32_file_path").toString();
	 fpgaBurn = obj.value("fpga_burn").toBool();
	 stm32Burn = obj.value("stm32_burn").toBool();
	 intervalTime = obj.value("interval_time").toInt();
	 BLE_UUID= obj.value("ble_uuid").toString().toStdString();
	 ICU_NAME= obj.value("icu_name").toString().toStdString();
	 userName= obj.value("user_name").toString().toStdString();
	 useUpdate= obj.value("use_update").toBool();
	 range_x = obj.value("range_X").toInt();
	 range_y = obj.value("range_Y").toInt();
	/*qDebug() << "Test Mode:" << testMode;
	qDebug() << "FPGA File Path:" << fpgaPath;
	qDebug() << "STM32 File Path:" << stm32Path;
	qDebug() << "FPGA Burn:" << fpgaBurn;
	qDebug() << "STM32 Burn:" << stm32Burn;
	qDebug() << "Interval Time:" << intervalTime << "min";
	qDebug() << "ble_uuid:" << QString::fromStdString(BLE_UUID);
	qDebug() << "icu_name:" << QString::fromStdString(ICU_NAME);
	qDebug() << "user_name:" << QString::fromStdString(userName);
	qDebug() << "useUpdate:" << useUpdate;*/
}
