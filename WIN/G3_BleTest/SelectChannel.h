#ifndef CSELECTCHANNEL_H
#define CSELECTCHANNEL_H

#include <QWidget>


class CSelectChannel : public QWidget
{
    Q_OBJECT
public:
    CSelectChannel(QWidget *parent = nullptr);
    ~CSelectChannel();
    QVector<bool> checkBoxStates; // 初始状态为 false（未选中）
signals:
void signalSelectChannel(int channelNumber,bool isScelect);
};
#endif // CSELECTCHANNEL_H
