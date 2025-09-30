#include "G3_BleTest.h"
#include<QPrinterInfo>
#include "../NxG3Controller/CG3Controller.h"
//#include "../NxControllerInterface/IController.h"
//#include "../NxG3ECU/G3_ECU.h"
#include <QFileDialog>
#include <QFile>
#include <QDebug>
#include <QTextBrowser>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
//#define  DEBUG_MODE
#include <qprocess.h>
#include "../NxG3ECU/EcuProtocol/HardwareDefine.h"
#include  <QTimer>
#include <string>
#include <QJsonDocument>
#include <QJsonObject>



G3_BleTest::G3_BleTest(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	setWindowTitle(QString::fromLocal8Bit("Optimus"));
	
	ui.progressBar->setVisible(false);
	drawTimer = new QTimer(this);
	timer = new QTimer();
	loadingScreen = new CLoadingScreen();
	
	ui.temperature->setEnabled(true);
	ui.rssi->setEnabled(true);
	ui.icuPower->setEnabled(true);
	ui.start->setEnabled(true);
	ui.discharge_charge_ma->setEnabled(true);
	ui.probe_current->setEnabled(true);
	ui.icu_battery_voltage->setEnabled(true);
	ui.boardTemp->setEnabled(true);
	ui.ptx_vddc_voltage->setEnabled(true);
	ui.stop->setEnabled(true);
	ui.tabWidget->removeTab(1);
	ui.tabWidget->removeTab(2);
	ui.selectComboBox->setVisible(false);;
	// 创建定时器，设置5秒更新一次
	mySelectChannel = new CSelectChannel();
	showCurrentChannelWave = new CShowCurrentChannelWave();
	
	const std::string BLE_UUID("6e400001-b5a3-f393-e0a9-e50e24dcca9e");
	std::string ICU_NAME = "Optimus_ICU";
	BleDeviceInfo  connectDev;
	connectDev.charServiceUUID = L"{6e400001-b5a3-f393-e0a9-e50e24dcca9e}";
	//connectDev.mac_address = 0xE3FE123F07CF;//
	//connectDev.mac_address = 0xd98d8b92e1a4;//
	connectDev.mac_address = 0xC8D118D750EEA;//
	connectDev.charReadUUID = L"{6e400003-b5a3-f393-e0a9-e50e24dcca9e}";
	connectDev.charWriteUUID = L"{6e400002-b5a3-f393-e0a9-e50e24dcca9e}";
	readJSon();
	connectSlot();
	showMaximized();
	ui.updateSTM32->setVisible(false);
	ui.update_BLE->setVisible(false);
	ui.update_FPGA->setVisible(false);
	ui.chooseSTM32->setVisible(false);
	ui.choose_FPGA->setVisible(false);
	ui.label_3->setVisible(false);
	ui.label_4->setVisible(false);
	ui.label_8->setVisible(false);
	ui.STM32_LE->setVisible(false);
	ui.FPGA_LE->setVisible(false);
}
G3_BleTest::~G3_BleTest()
{
}
G3Controller* G3_BleTest::getRhxController(ControllerType type, AmplifierSampleRate sampleRate,
	AcquisitionMode mode)
{
	auto a = new G3Controller();
	//a->setBleMacAdd(0xC2CD19375C47);
	//a->setBleMacAdd(0xE3FE123F07CF);
	//a->setBleMacAdd(0xd98d8b92e1a4);
	//a->setBleMacAdd(0Xeb998955413d);
		//a->setBleMacAdd(0xe75a0ddb14a5);
	a->setBleMacAdd(systemCfg.address);
	//a->setBleMacAdd(0xd441e1263260);

	return  a;
}

/**
 * \开始采集
 */
void G3_BleTest::startRun()
{
	int num = 0;
	QStringList items;
	for (int i = 0; i < 64; i++)
	{
		if( isChannelEnabled( channels, i))
		{
			items << QString::number(i+1);
			num++;
		}
	}
	controller->selectChannel(channels);
	ui.selectComboBox->clear();
	ui.selectComboBox->addItems(items);
	ui.selectComboBox->setVisible(true);
	showCurrentChannelWave->show();
	controller->startRecord();
	//timer->start(2);
	//controller->setFrameCallback([=](uint8_t* data, size_t size) {
	//	// 这里是处理帧数据的回调

	//	printf("Received frame data of size: %zu\n", size);
	//	constexpr uint64_t header = 0xd7a22aaa38132a53UL;
	//	constexpr uint64_t noframeheader = 0xd7a22aaa38132a57UL;
	//	static  int sampleCount = 0;
	//	uint64_t x1 = (data)[0];
	//	uint64_t x2 = (data)[1];
	//	uint64_t x3 = (data)[2];
	//	uint64_t x4 = (data)[3];
	//	uint64_t x5 = (data)[4];
	//	uint64_t x6 = (data)[5];
	//	uint64_t x7 = (data)[6];
	//	uint64_t x8 = (data)[7];
	//	uint64_t usbHeader = (x8 << 56) + (x7 << 48) + (x6 << 40) + (x5 << 32) + (x4 << 24) + (x3 << 16) + (x2 << 8) + (x1 << 0);
	//	if (usbHeader == header)
	//	{
	//		uint8_t* dataPtr = (uint8_t*)data;
	//		uint16_t read_channel = *(uint16_t*)(dataPtr + 12);
	//		uint32_t temp = *(uint32_t*)(dataPtr + 8);  // 正确的字节偏移
	//		qDebug() << "temp is  " << temp;
	//		for (int i = 14; i <= read_channel * 2 * 2 + 12; i += 2)
	//		{
	//			uint8_t highByte = *(dataPtr + i);         // 高字节
	//			uint8_t lowByte = *(dataPtr + i + 1);      // 低字节
	//			uint16_t combined = (highByte << 8) | lowByte;
	//			int transNubmer = 14;// read_channel * 2 + 12;
	//			auto index = ui.selectComboBox->currentIndex();
	//			if (i == transNubmer + index * 2 || i == transNubmer + index * 2 * 2 + 12)
	//			{
	//				double data = (combined - 32768) * 0.195;
	//				if (showCurrentChannelWave)
	//				{
	//					emit showChannel(sampleCount, data);
	//					//showCurrentChannelWave->seriesList->append(sampleCount, data);
	//				}
	//				sampleCount += 1;
	//			}
	//		}
	//	}
	//});
	//while (true)
	//{
	//	slotUpdateProgress();
	//}
	//controller->samplingControl(true);
}

void G3_BleTest::stopRun()
{
	timer->stop();
	controller->stopRecord();
	isSample = false;
	emit  closeShowChannel();
	channels = 0;
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
	ui.bleMessage->append(QString::fromLocal8Bit("开始连接"));
	qDebug() << QString::fromLocal8Bit("开始连接");
	controller = getRhxController(ControllerRecordUSB3, SampleRate500Hz, IcuMode);
	if(controller->connectDevice())
	{
		auto a = controller->checkICUStatus();
		qDebug() << QString::number(a);
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
		if(!controller) return;
		static 	int sampleCount = 0;
		if(sampleCount>500)
		{
		  emit clearWave();
			sampleCount = 0;
		}
		sampleCount++;
		//controller->readRecordBlock(&frameData);
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
			uint8_t* dataPtr = (uint8_t*)&frameData;
			uint16_t read_channel = *(uint16_t*)(dataPtr + 12);
			uint32_t temp = *(uint32_t*)(dataPtr + 8);  // 正确的字节偏移
			qDebug() << "temp is  " << temp;
			for (int i = 14; i <=read_channel * 2*2  + 12; i += 2) 
			{
				uint8_t highByte = *(dataPtr + i);         // 高字节
				uint8_t lowByte = *(dataPtr + i + 1);      // 低字节
				uint16_t combined = (highByte << 8) | lowByte;
				int transNubmer = 14;// read_channel * 2 + 12;
				auto index = ui.selectComboBox->currentIndex()*2;
				if (i == transNubmer+ index)
				{
					double data= (combined - 32768) * 0.195;
					if(showCurrentChannelWave)
					{
						emit showChannel(sampleCount, data);
						//showCurrentChannelWave->seriesList->append(sampleCount, data);
					}
					sampleCount += 1;
				}
			}
		}
	}
#endif
}
void G3_BleTest::slotPrintReport()
{
	QString content;
	QString newContent;
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

void G3_BleTest::slotUpadteICU_BLE()
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

void G3_BleTest::slotUpadteICU_STM32()
{
	loadingScreen->show();
	drawTimer->start(1000);
	controller->updateIcuMcu(STM32FileName);
}

void G3_BleTest::slotUpadteICU_FPGA()
{

	//loadingScreen->show();
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
	//ui.BLE_VERSION->append(QString::fromStdString(bleVersion));
}

void G3_BleTest::slotGetSTM32Version()
{
	std::string stm32Version = controller->getIcuMcuVersion();
	//ui.STM32_VERSION->append(QString::fromStdString(stm32Version));
}

void G3_BleTest::slotGetFpgaVersion()
{
	std::string fpgaVersion = controller->getFPGAVersion();
	//ui.FPGA_VERSION->append(QString::fromStdString(fpgaVersion));
}

void G3_BleTest::slotSetDeviceInfo()
{
	uint8_t* data = new uint8_t[9];
	memcpy(data, "OPDDC0005", 9); // **正确，不需要 '\0'**
	controller->setDeviceInfo(data);
}

void G3_BleTest::slotGetDeviceInfo()
{
	std::string fpgaVersion=controller->getDeviceInfo();
	ui.getDeviceInfoMessage->append(QString::fromStdString(fpgaVersion));
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
	uint8_t address = ui.read_line->text().toInt();
	ui.read_value_line->setText(QString::number(controller->getFPGAReg(address)));
}

void G3_BleTest::slotWriteRegValue()
{
	uint8_t address = ui.adress->text().toInt();
	uint32_t value= ui.value->text().toInt();
	controller->setFPGAReg(address,value);
}

void G3_BleTest::slotGetSensorData()
{
	controller->getSensorData();
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
	
}

void G3_BleTest::showDelayVector(QVector<int> delay)
{
	

}

void G3_BleTest::currentIndex()
{ 
	auto index= ui.selectComboBox->currentIndex();
	currentChannel = index;
	emit setCurrentWindowsTitle(QString::fromLocal8Bit("第%1通道数据").arg(ui.selectComboBox->currentText()));

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

void G3_BleTest::updatePlot(double channelData)
{
}

void G3_BleTest::updateChart()
{
	if (controller)
	{

		if (controller->isConnect()==1)
		{
			ui.bleMessage->append(QString::fromLocal8Bit("连接成功"));
			qDebug() << QString::fromLocal8Bit("连接成功");
			drawTimer->stop();
		}
		if(controller->isConnect() == 2)
		{
			ui.bleMessage->append(QString::fromLocal8Bit("连接中"));
			qDebug() << QString::fromLocal8Bit("连接中");
		}
		if(controller->isConnect() == 0)
		{
			ui.bleMessage->append(QString::fromLocal8Bit("连接失败"));
			qDebug() << (QString::fromLocal8Bit("连接失败"));
			drawTimer->stop();
			slotConnectBleConnect();
		
		}
	}


	////int status=controller->isUpdateSuccess();
	//if(oldNum!= status)
	//{
	//	QString text = setStatus(status);
	//	qDebug() << text;
	//	loadingScreen->setStatus(text);
	//	oldNum = status;
	//}
	//if (status==2|| status==3)
	//{
	//	drawTimer->stop();
	//}
}

QString G3_BleTest::setStatus(int num)
{
	switch (num)
	{
	case 0:
		
		return  QString::fromLocal8Bit("文件传输中！");

	case 1:
		return  QString::fromLocal8Bit("文件传输成功！");
	case 2:
		return  QString::fromLocal8Bit("文件传输失败！");
	case 3:
		return  QString::fromLocal8Bit("升级失败！");
	case 4:
		return  QString::fromLocal8Bit("升级成功！");
	case 5:
		return  QString::fromLocal8Bit("升级中！");
	default: return  QString::fromLocal8Bit("文件传输开始！");
	}
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

void G3_BleTest::connectSlot()
{
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
	connect(ui.boardTemp, &QPushButton::clicked, this, &G3_BleTest::slotGetBoardTemp);
	connect(ui.updateSTM32, &QPushButton::clicked, this, &G3_BleTest::slotUpadteICU_STM32);
	connect(ui.update_BLE, &QPushButton::clicked, this, &G3_BleTest::slotUpadteICU_BLE);
	connect(ui.update_FPGA, &QPushButton::clicked, this, &G3_BleTest::slotUpadteICU_FPGA);
	connect(ui.chooseSTM32, &QPushButton::clicked, this, &G3_BleTest::slotOpenSTM32File);
	connect(ui.choose_FPGA, &QPushButton::clicked, this, &G3_BleTest::slotOpenFPGAFile);
	connect(ui.selectComboBox, &QComboBox::currentTextChanged, this, &G3_BleTest::currentIndex);
	connect(ui.STM32Version, &QPushButton::clicked, this, &G3_BleTest::slotGetSTM32Version);
	connect(ui.FPGAversion, &QPushButton::clicked, this, &G3_BleTest::slotGetFpgaVersion);
	connect(ui.BLEversion, &QPushButton::clicked, this, &G3_BleTest::slotGetBleVersion);
	connect(ui.setDeviceInfo, &QPushButton::clicked, this, &G3_BleTest::slotSetDeviceInfo);
	connect(ui.getDeviceInfo, &QPushButton::clicked, this, &G3_BleTest::slotGetDeviceInfo);
	connect(ui.sleepMode, &QPushButton::clicked, this, &G3_BleTest::slotSetPowerOff);
	connect(ui.timeOut, &QPushButton::clicked, this, &G3_BleTest::slotSetTimeOutTime);
	connect(ui.impedance, &QPushButton::clicked, this, &G3_BleTest::slotStartimpedance);
	connect(ui.write, &QPushButton::clicked, this, &G3_BleTest::slotWriteRegValue);
	connect(ui.read, &QPushButton::clicked, this, &G3_BleTest::slotReadRegValue);
	connect(ui.getSensorData, &QPushButton::clicked, this, &G3_BleTest::slotGetSensorData);
	connect(timer, &QTimer::timeout, this, &G3_BleTest::slotUpdateProgress);
	connect(drawTimer, &QTimer::timeout, this, &G3_BleTest::updateChart);
	connect(mySelectChannel, &CSelectChannel::signalSelectChannel, this, &G3_BleTest::slotSetChannel);
	connect(ui.selectChannel, &QPushButton::clicked, [this]
	{
		mySelectChannel->show();
	}
	);
	connect(this, &G3_BleTest::showChannel, showCurrentChannelWave, &CShowCurrentChannelWave::setData);
	connect(this, &G3_BleTest::clearWave, showCurrentChannelWave, &CShowCurrentChannelWave::clearWave);
	connect(this, &G3_BleTest::setCurrentWindowsTitle, showCurrentChannelWave, &CShowCurrentChannelWave::setCurrentWindowsTitle);
	connect(this, &G3_BleTest::closeShowChannel, showCurrentChannelWave, &CShowCurrentChannelWave::closeShowChannel);
}

void G3_BleTest::readJSon()
{

	QString configFilePath="../config/SystemConfig.json";
	QFile file(configFilePath);
	// 1. 打开文件（只读）
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qWarning() << "Failed to open file:" << configFilePath;
		return;
	}

	// 2. 读取文件内容
	QByteArray data = file.readAll();
	file.close();

	// 3. 解析 JSON 文档
	QJsonParseError parseError;
	QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
	if (parseError.error != QJsonParseError::NoError) {
		
		return;
	}

	// 4. 判断 JSON 是否是对象
	if (!doc.isObject()) {

		return;
	}

	// 5. 获取 JSON 对象
	QJsonObject obj = doc.object();

	// 6. 提取字段

	QString hexStr = obj.value("address").toString();  // 例如 "eb998955413d"
	bool ok = false;
	uint64_t address = hexStr.toULongLong(&ok, 16);    // base = 16 for hex

	systemCfg .address= address;
}
