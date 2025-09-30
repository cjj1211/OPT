/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __STATISTICS_PAGE_H
#define __STATISTICS_PAGE_H
#include "../Global.h"
#include "ui_StatisticsPage.h"

#include <QWidget>
#include <memory>
BEGIN_NX_NAMESPACE

class StatisticsPage : public QWidget
{
    Q_OBJECT
public:
    StatisticsPage(QWidget* parent = nullptr);
    ~StatisticsPage();

private:
    std::unique_ptr<Ui::StatisticsPage> ui;
};

END_NX_NAMESPACE
#endif
