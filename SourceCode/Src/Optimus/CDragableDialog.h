/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   October 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_DRAGABLE_DIALOG_H
#define __C_DRAGABLE_DIALOG_H
#include "Global.h"
#include <QDialog>

BEGIN_NX_NAMESPACE

class CDragableDialog  : public QDialog
{
    Q_OBJECT

public:
    CDragableDialog(QWidget *parent = nullptr);
    ~CDragableDialog();

protected:
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    QPoint mousePoint;
    bool mousePress;
};

END_NX_NAMESPACE
#endif