#ifndef CLOADINGSCREEN_H
#define CLOADINGSCREEN_H

#include <QWidget>
#include "ui_CLoadingScreen.h"
#include "../Global.h"
#include <QtWidgets/QMainWindow>
#include <QTranslator>
BEGIN_NX_NAMESPACE
class CLoadingScreen : public QWidget
{
    Q_OBJECT

public:
    CLoadingScreen(QWidget *parent = nullptr);

signals:
    void signalShowLogin();

private slots:
    void checkProgress(int value);
    void updateProgress();
private:
    std::unique_ptr<Ui::CLoadingScreen> ui;
    int progress = 0;
    QTimer* timer;
};
#endif // CLOADINGSCREEN_H
END_NX_NAMESPACE