/******************************************************************************
 *  版权所有（C）2022-2024，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *  创建日期:    2024
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __CACTION_WIDGET_H
#define  __CACTION_WIDGET_H
#include "Global.h"
#include "ui_CActionWidget.h"
#include "../NxMessage/TransInformation.h"
#include <QTimer>
#include <QMovie>
BEGIN_NX_NAMESPACE
struct ActionCommand;
struct GameControl;
struct GameInfo;
class CCommandParser;
class CInformationProcessing;
class CActionWidget : public QWidget
{
	Q_OBJECT
public:
	CActionWidget(CInformationProcessing * informationProcessing, GameInfo gameInfo,QWidget *parent = nullptr);
	~CActionWidget();
	int getGameResult();

public:

	int score=0;
	int successNumber = 0;//成功次数
	int FailNumber = 0;
	signals:
		void changeActionNumber(int currentActionNumber);
		void  changeCutDownNumber(int min, int second);
		void  endGame();
private:
	void receiveCommand(ActionCommand actionCommand);
	void updateWidget(ActionType actionCommand);
	void checkType();

private slots:

	void slotChangeGameStatus();
	void slotUpdateCutDown();
	
private:
	Ui::CActionWidgetClass ui;
	GameStatus gameStatus = STOP;
	CCommandParser* commandParser;
	int currentActionNumber;//当前游戏次数
	int totalSeconds;
	ActionType currentActionType;//当前游戏类型
	ActionCommand receiveActionType;//收到游戏类型
	int totalActionNumber=1;//总游戏次数

	int singleActionNumber;//单个动作次数
	int currentSingleActionNumber=0;//当前动作次数
	int cutDown;
	int limitTime;//每个动作限时
	int currentNumber=0;//这个动作index
	int vectorSize;
	int gameNumber=0;
	QTimer* cutDownTimer;
	QTimer* actionTimer;
	std::vector<ActionType> actionNameVector;
	bool isEnd = false;


};

END_NX_NAMESPACE
#endif