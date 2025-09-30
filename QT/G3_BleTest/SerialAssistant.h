#ifndef SERIALASSISTANT_H
#define SERIALASSISTANT_H

#include <QMainWindow>
#include <QSerialPort>
#include <QTimer>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QCheckBox>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QWidget>
#include <QJsonArray>
#include <QScrollArea>
#include "CShowCurrentChannelWave.h"
struct CommandRow {
    QLineEdit* nameEdit;
    QLineEdit* cmdEdit;
    QCheckBox* enableCheckBox;
    QPushButton* delButton;
    QWidget* rowWidget;
    bool operator==(const CommandRow& other) const {
        // 比较唯一标识，比如指针是否相同即可
        return rowWidget == other.rowWidget;
    }
};

class SerialAssistant : public QMainWindow
{
    Q_OBJECT

public:
    explicit SerialAssistant(QWidget* parent = nullptr);
    ~SerialAssistant();

private slots:
    void on_openButton_clicked();
    void on_sendButton_clicked();
    void refreshSerialPorts();

    void readSerialData();

    void on_addCommand_clicked();
    void on_sendSelectedCommands_clicked();

private:
    void setupUi();
    void addCommandRow(const QString& name, const QString& cmd, bool checked);
    void loadCommandsFromFile();
    void saveCommandsToFile();

private:
    QSerialPort* serial;
    QTimer* autoSendTimer;

    QComboBox* portComboBox;
    QPushButton* openButton;
    QPushButton* clearRecvButton;
    QLabel* statusLabel;

    QTextEdit* sendTextEdit;
    QPushButton* sendButton;
    QCheckBox* hexSendCheckBox;
    QCheckBox* autoSendCheckBox;
    QSpinBox* intervalSpinBox;

    QCheckBox* hexRecvCheckBox;
    QTextEdit* recvTextEdit;

    // 自定义命令区
    QVBoxLayout* commandLayout;
    QPushButton* addCommandButton;
    QPushButton* sendSelectedCommandsButton;
    QList<CommandRow> commandRows;
	QScrollArea* commandScrollArea;
    int timeCount = 0;
    int xCount;
	CShowCurrentChannelWave* showCurrentChannelWave;
};

#endif // SERIALASSISTANT_H
