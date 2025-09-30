/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   December 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#include "Global.h"
#include <QWidget>
#include "ui_CIllnessInfo.h"

QT_BEGIN_NAMESPACE
namespace Ui { class CIllnessInfo; };
QT_END_NAMESPACE

BEGIN_NX_NAMESPACE

class CIllnessInfo : public QWidget
{
    Q_OBJECT
    QT_TRANSLATE_HANDLER
public:
    CIllnessInfo(QWidget *parent = nullptr);
    ~CIllnessInfo();

    void setInfo(const QString& info, const QString& date);

private:
    Ui::CIllnessInfo* ui;
};

END_NX_NAMESPACE