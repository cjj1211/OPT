/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/

#ifndef __OPTIMUS_H
#define __OPTIMUS_H
#include "Global.h"
#include "ui_Optimus.h"
#include <QtWidgets/QMainWindow>
#include <QTranslator>

BEGIN_NX_NAMESPACE

class MainFrame;
class CLoginPage;
class CLoadingScreen;

class Optimus final : public QMainWindow
{
    Q_OBJECT
public:
    explicit Optimus(QWidget *parent = nullptr);
    ~Optimus() override;

public slots:
    void ChangeLanguage(const QString& language) const;
    void  showMainFrame(bool isLogin);
    void  showLogin() const;

private slots:
    void changeEvent(QEvent* e) override;

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    Ui::Optimus ui;
    MainFrame* mainFrame;
    CLoginPage* loginPage;
    CLoadingScreen* loadingScreen;
    QTranslator* translator;
};

END_NX_NAMESPACE
#endif