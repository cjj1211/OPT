// /******************************************************************************
// *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
// *  作者 : 宋志杰 zhijie.song@neuroxess.com
// *  创建日期:   3 2024
// *  修改记录:
// *  <修改者姓名><邮件地址>   <修改内容>
// ******************************************************************************/
#ifndef C_PROGRESS_BAR_DIALOG_H
#define C_PROGRESS_BAR_DIALOG_H
#include "Global.h"
#include "CDragableDialog.h"
#include "ui_CProgressBarDialog.h"
#include <memory>


BEGIN_NX_NAMESPACE

class CProgressBarDialog : public CDragableDialog
{
    Q_OBJECT
    QT_TRANSLATE_HANDLER
public:
    CProgressBarDialog(QWidget* parent = nullptr);
    ~CProgressBarDialog() override;

public slots:
    void slotUpdateProgress(int progress);
    void slotUpdateContentText(QString text);

private:
    std::unique_ptr<Ui::CProgressBarDialog> ui;
};

END_NX_NAMESPACE
#endif