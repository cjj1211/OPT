#include "CMotionTraining.h"
#include <qevent.h>
#include "Header/Header.h"
#include "CActionWidget.h"
#include "CDeviceWidget.h"
#include "CLoadingScreen.h"
#include "../NxAppService/CInformationProcessing.h"
#include "GameStartCountdown.h"
#include <ylt/struct_json/json_writer.h>
#include "CAnalysisWidget.h"
BEGIN_NX_NAMESPACE
CMotionTraining::CMotionTraining(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    informationProcessing = new CInformationProcessing(this);
    loadingScreen = new CLoadingScreen(this);
    informationProcessing->initClient();
    connect(informationProcessing, &CInformationProcessing::initGame, this, &CMotionTraining::initGame);
    ui.load->addWidget(loadingScreen);
    setWindowFlags(Qt::FramelessWindowHint);
    this->showMaximized();
    informationProcessing->continueGame();
}
void CMotionTraining::slotClose()
{
	if (actionWidget==nullptr)
	{
        this->close();
        return;
	}
    TransInformation ts;
    ts.MessageType = MessageType::GAME_CONTROL;
    ts.GameInfo.FailNumber = actionWidget->FailNumber;
    ts.GameInfo.Score = actionWidget->getGameResult();
    ts.GameInfo.SuccessNumber = actionWidget->successNumber;
    ts.GameControl.GameStatus = END;
    std::string end;
    struct_json::to_json(ts, end); // 序列化
    informationProcessing->sendMessage(QString::fromStdString(end));
    this->close();
}

void CMotionTraining::initGame( TransInformation initInfo)
{
	    currentInfo = initInfo;
	    gameStartCountdown = new GameStartCountdown(this);
        connect(gameStartCountdown, &GameStartCountdown::startGame, this, &CMotionTraining::startGame);
        header = new Header(currentInfo.Patient, currentInfo.GameInfo);
        ui.mainFrame->addWidget(gameStartCountdown);
        ui.head->addWidget(header);
        deviceWidget = new CDeviceWidget(informationProcessing, this);
        ui.device->addWidget(deviceWidget);
        connect(header, &Header::shutDown, this, &CMotionTraining::slotClose);
        ui.load->removeWidget(loadingScreen);
   
   /* else
    {
        header = new Header(initInfo.Patient, initInfo.GameInfo);
        deviceWidget = new CDeviceWidget(informationProcessing, this);
        gameStartCountdown = new GameStartCountdown(this);
        connect(gameStartCountdown, &GameStartCountdown::startGame, this, &CMotionTraining::startGame);
        ui.load->removeWidget(loadingScreen);
        ui.head->addWidget(header);
        ui.mainFrame->addWidget(gameStartCountdown);
        ui.device->addWidget(deviceWidget);
        connect(header, &Header::shutDown, this, &CMotionTraining::slotClose);
	    
    }
    */

}

void CMotionTraining::mousePressEvent(QMouseEvent* event)
{
    TransInformation ts;
    ts.MessageType = EVENT;
    if (event->button()== Qt::LeftButton)
    {
        ts.Event.EventName = QString("Left Mouse Button Clicked").toStdString();
    }
    if (event->button() == Qt::RightButton)
    {
        ts.Event.EventName = QString("Right Mouse Button Clicked").toStdString();
    }
    std::string test;
    struct_json::to_json(ts, test); // 序列化
    informationProcessing->sendMessage(QString::fromStdString(test));
	QWidget::mousePressEvent(event);
}

void CMotionTraining::keyPressEvent(QKeyEvent* event)
{
    TransInformation ts;
    ts.MessageType = EVENT;
    ts.Event.EventName = event->text().toStdString();
    std::string test;
    struct_json::to_json(ts, test); // 序列化
    informationProcessing->sendMessage(QString::fromStdString(test));
	QWidget::keyPressEvent(event);
}

void CMotionTraining::endGame()
{
	if (analysisWidget)
	{
        analysisWidget = new CAnalysisWidget(informationProcessing, this);
        analysisWidget->score = actionWidget->getGameResult();
        analysisWidget->analysisRequest(informationProcessing, "2024-03-04", "2024-03-28");
        ui.mainFrame->removeWidget(actionWidget);
        ui.mainFrame->addWidget(analysisWidget);
        connect(analysisWidget, &CAnalysisWidget::shutDown, this, &CMotionTraining::slotClose);
        connect(analysisWidget, &CAnalysisWidget::initGame, informationProcessing, &CInformationProcessing::continueGame);
	}
	else
	{
		return;
	}

}

void CMotionTraining::startGame()
{
  

    actionWidget = new CActionWidget(informationProcessing, currentInfo.GameInfo, this);
    connect(actionWidget, &CActionWidget::changeCutDownNumber, header, &Header::slotUpdateCutDown);
    connect(actionWidget, &CActionWidget::changeActionNumber, header, &Header::changeActionNumber);
    connect(actionWidget, &CActionWidget::endGame, this, &CMotionTraining::endGame);
    ui.mainFrame->removeWidget(gameStartCountdown);
    ui.mainFrame->addWidget(actionWidget);
   
  

}

END_NX_NAMESPACE
