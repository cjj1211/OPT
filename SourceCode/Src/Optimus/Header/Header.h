/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
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

class Header : public QWidget
{
    Q_OBJECT
    QT_TRANSLATE_HANDLER
public:
    Header(QWidget* parent = nullptr);
    ~Header();

    void checkPageBtn(PageID id) const;
    void adminLogin() const;
    void setCurrentUserName(const QString& userName);
    void showAvatar(const std::vector<char>& avatar) const;
    void userLogin() const;

signals:
    void signalJumpToPage(PageID id);
    void signalShutdown();
    void signalLogout();

private slots:
    void slotTrainBtnClick();
    void slotPatientAdminBtnClick();
    void slotStatisticsPageClick();
    void slotStandardTreatmentClick();
    void slotShutdown();
    void slotLogout();
    void slotSettingsClick();

private:
    std::unique_ptr<Ui::Header> ui;
    QString currentUserName;
};
END_NX_NAMESPACE
#endif
