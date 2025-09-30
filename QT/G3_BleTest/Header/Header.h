/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/

#ifndef __HEADER_H
#define __HEADER_H
#include "../Global.h"
#include "ui_Header.h"

#include <QWidget>
#include <memory>

BEGIN_NX_NAMESPACE
struct Patient;
struct GameInfo;
class Header : public QWidget
{
    Q_OBJECT
    QT_TRANSLATE_HANDLER
public:
    Header(Patient patient, GameInfo gameInfo ,QWidget* parent = nullptr);
    ~Header() override;
    //设备状态控制警报图片
    void changeWarnLabel(bool isWarning) ;
    void slotUpdateCutDown(int minutes, int seconds);
    void changeActionNumber(int number);
private:
    void showAvatar(const QString& avatar) const;
signals:
    void shutDown();
    void endGame();
private slots:
    void slotShutDown();

private:
    std::unique_ptr<Ui::Header> ui;
    QString currentUserName;

    int totalSeconds = 60*60;

};

END_NX_NAMESPACE

#endif