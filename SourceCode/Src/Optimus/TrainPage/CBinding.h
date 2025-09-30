/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_BINDING_H
#define __C_BINDING_H
#include "../Global.h"
#include "ui_Binding.h"
#include <QWidget>
#include <memory>

class CBinding : public QWidget
{
    Q_OBJECT
        QT_TRANSLATE_HANDLER
public:
    CBinding(QWidget* parent = nullptr);
    ~CBinding() override;

private:
    std::unique_ptr<Ui::Binding> ui;
};

#endif