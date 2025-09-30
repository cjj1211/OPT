/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   November 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_NX_MESSAGE_BOX_H
#define __C_NX_MESSAGE_BOX_H
#include "Global.h"
#include <QWidget>
#include <memory>
#include "CDragableDialog.h"
#include "ui_CNxMessageBox.h"

//QT_BEGIN_NAMESPACE
//namespace Ui { class CNxMessageBox; };
//QT_END_NAMESPACE

BEGIN_NX_NAMESPACE

class CNxMessageBox : public CDragableDialog
{
    Q_OBJECT
    QT_TRANSLATE_HANDLER
public:
    CNxMessageBox(const QString& title,const QString& info, QWidget *parent = nullptr);
    ~CNxMessageBox();

    bool isConfirmed() {
        return confirmed;
    };

public slots:
    void slotConfirmed();

private:
    std::unique_ptr<Ui::CNxMessageBox> ui;
    bool confirmed;
};

END_NX_NAMESPACE

#endif