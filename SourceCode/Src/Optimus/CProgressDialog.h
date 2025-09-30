/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   October 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_PROGRESS_DIALOG_H
#define __C_PROGRESS_DIALOG_H
#include "Global.h"
#include "CDragableDialog.h"
#include "ui_CProgressDialog.h"
#include <QDialog>
#include <memory>

class QMovie;
BEGIN_NX_NAMESPACE

class CProgressDialog  : public CDragableDialog
{
    Q_OBJECT
    QT_TRANSLATE_HANDLER
public:
    CProgressDialog(Qt::WindowModality modality = Qt::WindowModal, QWidget *parent = nullptr);
    ~CProgressDialog() override;
    void setContent(const QString& content);

private:
    std::unique_ptr<Ui::CProgressDialog> ui;
    QMovie* movie;
};

END_NX_NAMESPACE
#endif
