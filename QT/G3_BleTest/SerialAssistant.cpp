#include "SerialAssistant.h"
#include <QMessageBox>
#include <QDateTime>
#include <QSerialPortInfo>
#include <QDebug>
#include <QRegExp>
#include <QTextCodec>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "../NxG3ECU/EcuProtocol/G3_Protocol.h"
SerialAssistant::SerialAssistant(QWidget* parent)
    : QMainWindow(parent),
    serial(new QSerialPort(this)),
    autoSendTimer(new QTimer(this)),
    xCount(0)
{
    setupUi();
    refreshSerialPorts();

    connect(serial, &QSerialPort::readyRead, this, &SerialAssistant::readSerialData);
    connect(autoSendTimer, &QTimer::timeout, this, &SerialAssistant::on_sendButton_clicked);
    connect(autoSendCheckBox, &QCheckBox::toggled, this, [=](bool checked) {
        if (checked)
        {
            autoSendTimer->start(intervalSpinBox->value());
			showCurrentChannelWave->show();
        }
        else
        {
            autoSendTimer->stop();
            showCurrentChannelWave->close();
        }
        });
    connect(intervalSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int val) {
        if (autoSendCheckBox->isChecked()) autoSendTimer->start(val);
        });

    loadCommandsFromFile();
}

SerialAssistant::~SerialAssistant()
{
    saveCommandsToFile();
}

void SerialAssistant::setupUi()
{
    
        // 初始化波形显示区域
        showCurrentChannelWave = new CShowCurrentChannelWave(this);
        showCurrentChannelWave->setVisible(false);  // 初始隐藏

        QWidget* central = new QWidget(this);
        central->setStyleSheet("background-color: #f5f5f5; color: #202020;");

        QHBoxLayout* mainLayout = new QHBoxLayout(central);
        mainLayout->setContentsMargins(12, 12, 12, 12);
        mainLayout->setSpacing(10);

        // 左侧垂直布局
        QVBoxLayout* leftLayout = new QVBoxLayout;
        leftLayout->setSpacing(10);

        // ===== 串口设置区域 =====
        QGroupBox* serialGroup = new QGroupBox(QString::fromLocal8Bit("串口设置"), this);
        QHBoxLayout* serialLayout = new QHBoxLayout(serialGroup);

        portComboBox = new QComboBox(this);
        portComboBox->setFixedWidth(140);
        portComboBox->setStyleSheet("QComboBox { background: white; padding: 2px; }");

        openButton = new QPushButton(QString::fromLocal8Bit("打开串口"), this);
        openButton->setStyleSheet("QPushButton { background-color: #0078D7; color: white; padding: 5px 10px; border-radius: 4px; }");

        statusLabel = new QLabel(QString::fromLocal8Bit("状态：未连接"), this);
        statusLabel->setMinimumWidth(150);

        serialLayout->addWidget(new QLabel(QString::fromLocal8Bit("串口:"), this));
        serialLayout->addWidget(portComboBox);
        serialLayout->addWidget(openButton);
        serialLayout->addStretch();
        serialLayout->addWidget(statusLabel);

        leftLayout->addWidget(serialGroup);

        // ===== 发送设置区域 =====
        QGroupBox* sendGroup = new QGroupBox(QString::fromLocal8Bit("发送设置"), this);
        QVBoxLayout* sendLayout = new QVBoxLayout(sendGroup);

        sendTextEdit = new QTextEdit(this);
        sendTextEdit->setPlaceholderText(QString::fromLocal8Bit("请输入发送内容..."));
        sendTextEdit->setStyleSheet("QTextEdit { background: white; font-size: 14px; border: 1px solid gray; border-radius: 4px; }");
        sendTextEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sendLayout->addWidget(sendTextEdit, 10);

        QHBoxLayout* controlLayout = new QHBoxLayout;
        sendButton = new QPushButton(QString::fromLocal8Bit("发送"), this);
        sendButton->setStyleSheet("QPushButton { background-color: #28a745; color: white; padding: 6px 20px; border-radius: 6px; font-weight: bold; font-size: 14px; }");

        hexSendCheckBox = new QCheckBox(QString::fromLocal8Bit("HEX发送"), this);
        autoSendCheckBox = new QCheckBox(QString::fromLocal8Bit("定时发送"), this);

        intervalSpinBox = new QSpinBox(this);
        intervalSpinBox->setSuffix(" ms");
        intervalSpinBox->setRange(1, 10000);
        intervalSpinBox->setValue(2);
        intervalSpinBox->setFixedWidth(100);

        controlLayout->addWidget(sendButton);
        controlLayout->addSpacing(10);
        controlLayout->addWidget(hexSendCheckBox);
        controlLayout->addWidget(autoSendCheckBox);
        controlLayout->addWidget(intervalSpinBox);
        controlLayout->addStretch();

        sendLayout->addLayout(controlLayout, 0);

        leftLayout->addWidget(sendGroup);

        // ===== 接收区域 =====
        QGroupBox* recvGroup = new QGroupBox(QString::fromLocal8Bit("接收数据"), this);
        QVBoxLayout* recvLayout = new QVBoxLayout(recvGroup);

        QHBoxLayout* recvTopLayout = new QHBoxLayout;
        hexRecvCheckBox = new QCheckBox(QString::fromLocal8Bit("HEX显示"), this);
        clearRecvButton = new QPushButton(QString::fromLocal8Bit("清除"), this);
        clearRecvButton->setStyleSheet("QPushButton { background-color: #dc3545; color: white; padding: 2px 10px; border-radius: 4px; }");
        clearRecvButton->setFixedHeight(26);

        recvTopLayout->addWidget(hexRecvCheckBox);
        recvTopLayout->addStretch();
        recvTopLayout->addWidget(clearRecvButton);

        recvTextEdit = new QTextEdit(this);
        recvTextEdit->setReadOnly(true);
        recvTextEdit->setStyleSheet("QTextEdit { background: #ffffff; font-family: Consolas, monospace; font-size: 13px; }");
        recvTextEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        recvLayout->addLayout(recvTopLayout);
        recvLayout->addWidget(recvTextEdit);

        leftLayout->addWidget(recvGroup);

        // ===== 波形显示区域（新增）=====
        QGroupBox* waveGroup = new QGroupBox(QStringLiteral("波形显示"), this);
        QVBoxLayout* waveLayout = new QVBoxLayout(waveGroup);
        waveLayout->setContentsMargins(4, 4, 4, 4);
        waveLayout->addWidget(showCurrentChannelWave);
        waveGroup->setVisible(false);  // 初始隐藏
        leftLayout->addWidget(waveGroup, 2);

        // ===== 右侧自定义命令区 =====
        QGroupBox* customCmdGroup = new QGroupBox(QString::fromLocal8Bit("自定义命令"), this);
        customCmdGroup->setMinimumWidth(340);
        QVBoxLayout* rightLayout = new QVBoxLayout(customCmdGroup);
        rightLayout->setSpacing(8);

        commandScrollArea = new QScrollArea(customCmdGroup);
        commandScrollArea->setWidgetResizable(true);

        QWidget* commandWidget = new QWidget;
        commandLayout = new QVBoxLayout(commandWidget);
        commandLayout->setContentsMargins(4, 4, 4, 4);
        commandLayout->setSpacing(6);
        commandLayout->addStretch();
        commandScrollArea->setWidget(commandWidget);
        rightLayout->addWidget(commandScrollArea);

        QHBoxLayout* cmdButtonLayout = new QHBoxLayout;
        addCommandButton = new QPushButton(QString::fromLocal8Bit("新增指令"), customCmdGroup);
        sendSelectedCommandsButton = new QPushButton(QString::fromLocal8Bit("发送选中"), customCmdGroup);

        addCommandButton->setCursor(Qt::PointingHandCursor);
        sendSelectedCommandsButton->setCursor(Qt::PointingHandCursor);

        QString btnStyle = R"(
        QPushButton {
            background-color: #0078D7;
            color: white;
            border-radius: 5px;
            padding: 8px 20px;
            font-weight: bold;
            font-size: 15px;
        }
        QPushButton:hover {
            background-color: #005A9E;
        }
        QPushButton:pressed {
            background-color: #003E73;
        }
    )";

        addCommandButton->setStyleSheet(btnStyle);
        sendSelectedCommandsButton->setStyleSheet(btnStyle);

        cmdButtonLayout->addWidget(addCommandButton);
        cmdButtonLayout->addWidget(sendSelectedCommandsButton);

        rightLayout->addLayout(cmdButtonLayout);

        mainLayout->addLayout(leftLayout, 3);
        mainLayout->addWidget(customCmdGroup, 2);
        setCentralWidget(central);

        // ===== 信号连接 =====
        connect(openButton, &QPushButton::clicked, this, &SerialAssistant::on_openButton_clicked);
        connect(sendButton, &QPushButton::clicked, this, &SerialAssistant::on_sendButton_clicked);
        connect(addCommandButton, &QPushButton::clicked, this, &SerialAssistant::on_addCommand_clicked);
        connect(sendSelectedCommandsButton, &QPushButton::clicked, this, &SerialAssistant::on_sendSelectedCommands_clicked);
        connect(clearRecvButton, &QPushButton::clicked, this, [=]() {
            recvTextEdit->clear();
            });

        connect(autoSendCheckBox, &QCheckBox::toggled, this, [=](bool checked) {
            if (checked) {
                autoSendTimer->start(intervalSpinBox->value());
                waveGroup->setVisible(true);
            }
            else {
                autoSendTimer->stop();
                waveGroup->setVisible(false);
            }
            });

        connect(intervalSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int val) {
            if (autoSendCheckBox->isChecked()) autoSendTimer->start(val);
            });

        hexSendCheckBox->setStyleSheet("QCheckBox::indicator { border: 2px solid black; background: white; }");
        autoSendCheckBox->setStyleSheet("QCheckBox::indicator { border: 2px solid black; background: white; }");
        hexRecvCheckBox->setStyleSheet("QCheckBox::indicator { border: 2px solid black; background: white; }");
  }



void SerialAssistant::refreshSerialPorts()
{
    portComboBox->clear();
    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo& info : ports) {
        portComboBox->addItem(info.portName());
    }
    
}

void SerialAssistant::on_openButton_clicked()
{
    if (serial->isOpen()) {
        serial->close();
        openButton->setText(QString::fromLocal8Bit("打开串口"));
        statusLabel->setText(QString::fromLocal8Bit("状态：串口已关闭"));
    }
    else {
        serial->setPortName(portComboBox->currentText());
        serial->setBaudRate(QSerialPort::Baud115200);
        serial->setDataBits(QSerialPort::Data8);
        serial->setParity(QSerialPort::NoParity);
        serial->setStopBits(QSerialPort::OneStop);
        serial->setFlowControl(QSerialPort::NoFlowControl);
        if (serial->open(QIODevice::ReadWrite)) {
            openButton->setText(QString::fromLocal8Bit("关闭串口"));
            statusLabel->setText(QString::fromLocal8Bit("状态：串口已打开"));
        }
        else {
            QMessageBox::warning(this, QString::fromLocal8Bit("错误"), QString::fromLocal8Bit("无法打开串口"));
        }
    }
    
}

void SerialAssistant::on_sendButton_clicked()
{
    if (!serial->isOpen()) return;

    if (autoSendCheckBox->isChecked())
    {
        timeCount++;
        if
            (timeCount > 1000) {
            timeCount = 0;
            showCurrentChannelWave->clearWave();
               
		}
    }
    QString data = sendTextEdit->toPlainText();
    QByteArray outData;
    if (hexSendCheckBox->isChecked()) {
        QStringList hexList = data.split(QRegExp("\\s+"));
        for (const QString& hexStr : hexList) {
            bool ok;
            uint8_t byte = hexStr.toUInt(&ok, 16);
            if (ok)
                outData.append(byte);
            else
                qDebug() << "无效的 HEX 字节：" << hexStr;
        }
    }
    else {
        outData = data.toUtf8();
    }
    serial->write(outData);
}

QString formatHexWithAsciiOffset(const QByteArray& data, int bytesPerLine = 16)
{
    QString result;
    int total = data.size();

    for (int i = 0; i < total; i += bytesPerLine) {
        QString line;

        // 地址偏移，例如 0000:
        line += QString("%1:  ").arg(i, 4, 16, QLatin1Char('0')).toUpper();

        QString hexPart;
        QString asciiPart;

        for (int j = 0; j < bytesPerLine; ++j) {
            if (i + j < total) {
                uint8_t byte = static_cast<uint8_t>(data[i + j]);
                hexPart += QString("%1 ").arg(byte, 2, 16, QLatin1Char('0')).toUpper();

                if (byte >= 0x20 && byte <= 0x7E)  // 可显示字符
                    asciiPart += QChar(byte);
                else
                    asciiPart += '.';  // 不可见字符
            }
            else {
                hexPart += "   "; // 补空格对齐
                asciiPart += " ";
            }
        }

        line += hexPart + " " + asciiPart;
        result += line + "\n";
    }

    return result.trimmed();
}

QString safeUtf8ToString(const QByteArray& data)
{
    QTextCodec* codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::ConverterState state;
    QString decoded = codec->toUnicode(data.constData(), data.size(), &state);

    if (state.invalidChars > 0) {
        // 非法字符，退回到 hex 格式显示
        return formatHexWithAsciiOffset(data);
    }
    else {
        return decoded;
    }
}

QString sanitizeText(const QByteArray& data)
{
    QString result;
    for (char byte : data) {
        uint8_t c = static_cast<uint8_t>(byte);
        if (c >= 0x20 && c <= 0x7E) {
            result += QChar(c);
        }
        else if (c == '\n' || c == '\r' || c == '\t') {
            result += QChar(c);
        }
        else {
            result += ".";
        }
    }
    return result;
}
const Frame* parseQStringToFrame(const QString& str)
{
    QByteArray data;
    QStringList hexList = str.split(" ");
    for (const QString& hex : hexList) {
        bool ok;
        uint8_t byte = hex.toUInt(&ok, 16);
        if (ok) data.append(byte);
    }

    if (data.size() < sizeof(Frame)) return nullptr;

    const Frame* frame = reinterpret_cast<const Frame*>(data.constData());

    int totalLen = sizeof(Frame) - 1 + frame->dataLen;
    if (data.size() < totalLen) return nullptr;

    if (frame->header != 0xAA55) return nullptr;

    return frame;
}


void SerialAssistant::readSerialData()
{

    QByteArray data = serial->readAll();
   
    QString displayStr;
    if (hexRecvCheckBox->isChecked()) {
        displayStr = formatHexWithAsciiOffset(data);
    }
    else {
        displayStr = safeUtf8ToString(data);
        displayStr = sanitizeText(displayStr.toUtf8());
    }
    if (autoSendCheckBox->isChecked())
    {
;
        QByteArray binData = QByteArray::fromHex(displayStr.toUtf8());
        const Frame* frame = reinterpret_cast<const Frame*>(binData.constData());
      
        if (frame->type == 0x57)
        {
            showCurrentChannelWave->setAxisXName(QString::fromLocal8Bit("时间"));
            showCurrentChannelWave->setAxisYName(QString::fromLocal8Bit("温度℃"));
			showCurrentChannelWave->setCurrentWindowsTitle(QString::fromLocal8Bit("温度曲线"));
            showCurrentChannelWave->setAxisXRange(0, 1000);
			showCurrentChannelWave->setAxisYRange(37, 100);
            showCurrentChannelWave->setAxisYTickInterval(1);
            showCurrentChannelWave->setData(timeCount, frame->data[0]);
            return;
        }
    }

    recvTextEdit->moveCursor(QTextCursor::End);
    recvTextEdit->insertPlainText(displayStr + "\n");
    const int maxLines = 200;
    while (recvTextEdit->document()->blockCount() > maxLines) {
        QTextCursor c(recvTextEdit->document());
        c.movePosition(QTextCursor::Start);
        c.select(QTextCursor::BlockUnderCursor);
        c.removeSelectedText();
        c.deleteChar();
    }
}

void SerialAssistant::addCommandRow(const QString& name, const QString& cmd, bool checked)
{
    QWidget* rowWidget = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(rowWidget);
    layout->setContentsMargins(0, 0, 0, 0);

    QLineEdit* nameEdit = new QLineEdit(rowWidget);
    nameEdit->setFixedWidth(100);
    nameEdit->setText(name);
	nameEdit->setStyleSheet("QLineEdit { background: white; padding: 2px; border: 1px solid black; }");
    QLineEdit* cmdEdit = new QLineEdit(rowWidget);
    cmdEdit->setText(cmd);
    cmdEdit->setStyleSheet("QLineEdit { background: white; padding: 2px; border: 1px solid black; }");
    QCheckBox* enableCheckBox = new QCheckBox(rowWidget);
    enableCheckBox->setChecked(checked);
    enableCheckBox -> setStyleSheet("QCheckBox::indicator { border: 2px solid black; background: white; }");
    QPushButton* delButton = new QPushButton(QString::fromLocal8Bit("删除"), rowWidget);
    delButton->setFixedWidth(50);

    layout->addWidget(nameEdit);
    layout->addWidget(cmdEdit);
    layout->addWidget(enableCheckBox);
    layout->addWidget(delButton);
    commandLayout->insertWidget(commandLayout->count() - 1, rowWidget);
    CommandRow row{ nameEdit, cmdEdit, enableCheckBox, delButton, rowWidget };
    commandRows.append(row);

    connect(delButton, &QPushButton::clicked, this, [=]() {
        commandLayout->removeWidget(rowWidget);
        commandRows.removeOne(row);
        rowWidget->deleteLater();
        saveCommandsToFile();
        });

    connect(nameEdit, &QLineEdit::editingFinished, this, &SerialAssistant::saveCommandsToFile);
    connect(cmdEdit, &QLineEdit::editingFinished, this, &SerialAssistant::saveCommandsToFile);
    connect(enableCheckBox, &QCheckBox::toggled, this, &SerialAssistant::saveCommandsToFile);
}

void SerialAssistant::on_addCommand_clicked()
{
    addCommandRow(QString::fromLocal8Bit("新指令"), QString(), false);
  
    saveCommandsToFile();
}

void SerialAssistant::on_sendSelectedCommands_clicked()
{
    if (!serial->isOpen()) {
        qDebug() << QString::fromLocal8Bit("串口未打开，无法发送命令");
        return;
    }
    for (const CommandRow& row : qAsConst(commandRows)) {
        if (!row.enableCheckBox->isChecked()) continue;

        QString data = row.cmdEdit->text();
        QByteArray outData;
        outData= QByteArray::fromHex(data.toUtf8());
       
        if (!outData.isEmpty()) {
            serial->write(outData);
        }
        else {
            qDebug() << QString::fromLocal8Bit("指令为空，跳过发送：") << row.cmdEdit->text();
        }
    }

}

void SerialAssistant::loadCommandsFromFile()
{
    QFile file("commands.json");
    if (!file.open(QIODevice::ReadOnly)) return;

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) return;

    QJsonArray arr = doc.array();
    for (const QJsonValue& val : arr) {
        if (val.isObject()) {
            QJsonObject obj = val.toObject();
            QString name = obj.value("name").toString();
            QString cmd = obj.value("cmd").toString();
            bool checked = obj.value("checked").toBool();
            addCommandRow(name, cmd, checked);
        }
    }
}

void SerialAssistant::saveCommandsToFile()
{
    QJsonArray arr;
    for (const CommandRow& row : qAsConst(commandRows)) {
        QJsonObject obj;
        obj["name"] = row.nameEdit->text();
        obj["cmd"] = row.cmdEdit->text();
        obj["checked"] = row.enableCheckBox->isChecked();
        arr.append(obj);
    }
    QJsonDocument doc(arr);
    QFile file("commands.json");
    if (!file.open(QIODevice::WriteOnly)) return;
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
}
