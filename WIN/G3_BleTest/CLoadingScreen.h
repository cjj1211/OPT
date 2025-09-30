#ifndef CLOADINGSCREEN_H
#define CLOADINGSCREEN_H

#include <QDialog>
#include <QMovie>
QT_BEGIN_NAMESPACE
namespace Ui { class CLoadingScreen; }
QT_END_NAMESPACE

class CLoadingScreen : public QDialog
{
    Q_OBJECT

public:
    CLoadingScreen(QWidget *parent = nullptr);
    ~CLoadingScreen();
    void setStatus(QString status);

private:
    Ui::CLoadingScreen *ui;
    QMovie* loadingMovie;
};
#endif // CLOADINGSCREEN_H
