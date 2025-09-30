/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __OPTIMUS_GLOBAL_H
#define __OPTIMUS_GLOBAL_H
#include <CLogger.h>

using namespace NX;

#define QT_TRANSLATE_HANDLER  protected slots:\
void changeEvent(QEvent* e) override\
{\
    QWidget::changeEvent(e);\
    switch (e->type())\
    {\
    case QEvent::LanguageChange:\
        ui->retranslateUi(this);\
        break;\
    default:\
        break;\
    }\
}



#endif


