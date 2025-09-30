/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#include "StatisticsPage.h"

BEGIN_NX_NAMESPACE
StatisticsPage::StatisticsPage(QWidget* parent)
    :QWidget(parent)
{
    ui = std::make_unique<Ui::StatisticsPage>();
    ui->setupUi(this);
}

StatisticsPage::~StatisticsPage()
{
}

END_NX_NAMESPACE