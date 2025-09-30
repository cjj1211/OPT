#pragma once


#include "ui_CMotionTraining.h"
#include "Global.h"
#include  "../NxMessage/TransInformation.h"
BEGIN_NX_NAMESPACE
class Header;
class CActionWidget;
class CDeviceWidget;
struct Patient;
struct GameInfo;
class CAnalysisWidget;
class CInformationProcessing;
class CLoadingScreen;
class GameStartCountdown;
class CMotionTraining : public QWidget
{
    Q_OBJECT

public:
    CMotionTraining(QWidget *parent = nullptr);
    ~CMotionTraining()=default;
private slots:
    void slotClose();
    void initGame(TransInformation  initInfo);
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void endGame();
    void startGame();


private:
    Ui::CMotionTrainingClass ui;
    Header* header;
    CActionWidget* actionWidget=nullptr;
    CDeviceWidget* deviceWidget;
    CAnalysisWidget* analysisWidget;
    CInformationProcessing* informationProcessing;
    CLoadingScreen* loadingScreen;
    GameStartCountdown* gameStartCountdown;
    TransInformation currentInfo;
};
END_NX_NAMESPACE