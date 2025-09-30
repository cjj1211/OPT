#include "SelectChannel.h"
#include <QWidget>
#include <QCheckBox>
#include <QVector>
#include <QVBoxLayout>
#include <QPushButton>
#include <qvalidator.h>
#include <QLineEdit>
#include <QIntValidator>

CSelectChannel::CSelectChannel(QWidget *parent)
    : QWidget(parent)
{


    setWindowTitle(QString::fromLocal8Bit("通道选择"));
    setStyleSheet("background-color: black;");
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QGridLayout* gridLayout = new QGridLayout();
    checkBoxStates.resize(64);
    QVector<QCheckBox*> channelCheckBoxes;
    // 创建复选框并添加到布局
    for (int i = 0; i < 64; ++i) {
        QCheckBox* channelCheckBox = new QCheckBox(QString::fromLocal8Bit("通道 %1").arg(i + 1), this);
        channelCheckBox->setChecked(checkBoxStates[i]);
        channelCheckBox->setStyleSheet(
            "QCheckBox {"
            "    color: white;"  // **让复选框文字变成白色**
            "}"
            "QCheckBox::indicator {"
            "    width: 16px;"
            "    height: 16px;"
            "    border: 2px solid white;"  // **白色边框**
            "    border-radius: 3px;"
            "    background-color: white;"  // **保证背景是黑色**
            "}"
            "QCheckBox::indicator:checked {"
            "    background-color: blue;"  // **选中时，填充白色**
            "}"
            "QCheckBox::indicator:unchecked {"
            "    background-color: white;"  // **未选中时，填充黑色**
            "}"
        );
        channelCheckBoxes.append(channelCheckBox);
        int row = i / 8;
        int col = i % 8;
        gridLayout->addWidget(channelCheckBox, row, col);
        connect(channelCheckBox, &QCheckBox::toggled, [this, i](bool checked) {
            emit signalSelectChannel(i, checked);
        });
    }
    QWidget* gridWidget = new QWidget(this);
    gridWidget->setLayout(gridLayout);
    mainLayout->addWidget(gridWidget);
    // 创建输入框和按钮
    QLineEdit* rowInput = new QLineEdit(this);
    rowInput->setPlaceholderText(QString::fromLocal8Bit("输入行号 (1-8)"));
    rowInput->setValidator(new QIntValidator(1, 8, this)); // 限制只能输入 1-8
    rowInput->setStyleSheet("color:white");
    QPushButton* btnSelectRow = new QPushButton(QString::fromLocal8Bit("选中该行"), this);
    QPushButton* btnSelectAll = new QPushButton(QString::fromLocal8Bit("全选"), this);
    QPushButton* btnDeselectAll = new QPushButton(QString::fromLocal8Bit("全不选"), this);
    QPushButton* ok = new QPushButton(QString::fromLocal8Bit("确定"), this);
    QPushButton* btnSelectNRow = new QPushButton(QString::fromLocal8Bit("选中前N行"), this);
    btnSelectNRow->setStyleSheet(
        "QPushButton {"
        "    border-style: solid;"     // 设定边框样式
        "    border-width: 2px;"       // 边框宽度
        "    border-color: white;"     // 边框颜色
        "    color: white;"            // 文字颜色（白色）
        "    background-color: black;" // 按钮背景色（黑色，防止看不见）
        "    padding: 5px 10px;"       // 内边距，让按钮更美观
        "}"
        "QPushButton:hover {"
        "    background-color: gray;"  // 悬停时按钮颜色变化
        "}"
        "QPushButton:pressed {"
        "    background-color: darkgray;" // 按下时按钮颜色变化
        "}"
    );
    btnSelectRow->setStyleSheet(
        "QPushButton {"
        "    border-style: solid;"     // 设定边框样式
        "    border-width: 2px;"       // 边框宽度
        "    border-color: white;"     // 边框颜色
        "    color: white;"            // 文字颜色（白色）
        "    background-color: black;" // 按钮背景色（黑色，防止看不见）
        "    padding: 5px 10px;"       // 内边距，让按钮更美观
        "}"
        "QPushButton:hover {"
        "    background-color: gray;"  // 悬停时按钮颜色变化
        "}"
        "QPushButton:pressed {"
        "    background-color: darkgray;" // 按下时按钮颜色变化
        "}"
    );
    btnSelectAll->setStyleSheet(
        "QPushButton {"
        "    border-style: solid;"     // 设定边框样式
        "    border-width: 2px;"       // 边框宽度
        "    border-color: white;"     // 边框颜色
        "    color: white;"            // 文字颜色（白色）
        "    background-color: black;" // 按钮背景色（黑色，防止看不见）
        "    padding: 5px 10px;"       // 内边距，让按钮更美观
        "}"
        "QPushButton:hover {"
        "    background-color: gray;"  // 悬停时按钮颜色变化
        "}"
        "QPushButton:pressed {"
        "    background-color: darkgray;" // 按下时按钮颜色变化
        "}"
    );
    btnDeselectAll->setStyleSheet(
        "QPushButton {"
        "    border-style: solid;"     // 设定边框样式
        "    border-width: 2px;"       // 边框宽度
        "    border-color: white;"     // 边框颜色
        "    color: white;"            // 文字颜色（白色）
        "    background-color: black;" // 按钮背景色（黑色，防止看不见）
        "    padding: 5px 10px;"       // 内边距，让按钮更美观
        "}"
        "QPushButton:hover {"
        "    background-color: gray;"  // 悬停时按钮颜色变化
        "}"
        "QPushButton:pressed {"
        "    background-color: darkgray;" // 按下时按钮颜色变化
        "}"
    );
    ok->setStyleSheet(
        "QPushButton {"
        "    border-style: solid;"     // 设定边框样式
        "    border-width: 2px;"       // 边框宽度
        "    border-color: white;"     // 边框颜色
        "    color: white;"            // 文字颜色（白色）
        "    background-color: black;" // 按钮背景色（黑色，防止看不见）
        "    padding: 5px 10px;"       // 内边距，让按钮更美观
        "}"
        "QPushButton:hover {"
        "    background-color: gray;"  // 悬停时按钮颜色变化
        "}"
        "QPushButton:pressed {"
        "    background-color: darkgray;" // 按下时按钮颜色变化
        "}"
    );
    // 创建布局并添加控件
    QHBoxLayout* rowLayout = new QHBoxLayout();
    rowLayout->addWidget(rowInput);
    rowLayout->addWidget(btnSelectRow);
    rowLayout->addWidget(btnSelectNRow);
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(btnSelectAll);
    buttonLayout->addWidget(btnDeselectAll);
    buttonLayout->addWidget(ok);
    mainLayout->addLayout(rowLayout);
    mainLayout->addLayout(buttonLayout);
    connect(ok, &QPushButton::clicked, [=]() {
        this->close();
    });
    // 连接按钮信号到槽
    connect(btnSelectRow, &QPushButton::clicked, [=]() {
        for (QCheckBox* checkBox : channelCheckBoxes) {
            checkBox->setChecked(false);
        }
        bool ok;
        int row = rowInput->text().toInt(&ok);
        if (ok && row >= 1 && row <= 9) {
            int startIdx = (row -1) * 8;
            for (int i = startIdx; i < startIdx + 8; ++i) {
                channelCheckBoxes[i]->setChecked(true);
            }
        }
    });
    // 连接按钮信号到槽
    connect(btnSelectNRow, &QPushButton::clicked, [=]() {
        for (QCheckBox* checkBox : channelCheckBoxes) {
            checkBox->setChecked(false);
        }
        bool ok;
        int row = rowInput->text().toInt(&ok);
        if (ok && row >= 1 && row <= 9) {
            for(int i=1;i<=row;i++)
            {
                int startIdx = (i - 1) * 8;
                for (int j = startIdx; j < startIdx + 8; ++j) {
                    channelCheckBoxes[j]->setChecked(true);
                }
            }
           
        }
    });
    connect(btnSelectAll, &QPushButton::clicked, [=]() {
        for (QCheckBox* checkBox : channelCheckBoxes) {
            checkBox->setChecked(true);
        }
    });

    connect(btnDeselectAll, &QPushButton::clicked, [=]() {
        for (QCheckBox* checkBox : channelCheckBoxes) {
            checkBox->setChecked(false);
        }
    });

}


CSelectChannel::~CSelectChannel()
{

}

