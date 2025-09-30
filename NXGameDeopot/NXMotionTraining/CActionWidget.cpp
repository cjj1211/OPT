#include "CActionWidget.h"
#include "../NxMessage/TransInformation.h"
#include "../NxAppService/CCommandParser.h"
#include <QFontDatabase>
#include <QPixmap>
BEGIN_NX_NAMESPACE
CActionWidget::CActionWidget(CInformationProcessing* informationProcessing, const GameInfo gameInfo,QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	auto boldId = QFontDatabase::addApplicationFont(QStringLiteral("./SiYuanHeiTi/SourceHanSansCN-Bold.ttf"));
	auto boldFamilies = QFontDatabase::applicationFontFamilies(boldId);

	auto fontId = QFontDatabase::addApplicationFont(QStringLiteral("./SiYuanHeiTi/SourceHanSansCN-Normal.ttf"));
	QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);

	QFont font_bold(boldFamilies.at(0), 40, QFont::Bold);
	QFont font_bolds(boldFamilies.at(0), 30, QFont::Bold);
	QFont font_normal(fontFamilies.at(0), 18);
	ui.actionname->setFont(font_bold);
	ui.counts->setFont(font_bolds);
	ui.Prompt->setFont(font_normal);
	ui.label->setFont(font_bolds);
	ui.label_2->setFont(font_bold);
	ui.result->setFont(font_bold);
	connect(ui.begin, &QPushButton::clicked, this, &CActionWidget::slotChangeGameStatus);
	QPixmap pixmapBegin(":/NXMotionTraining/images/trainPage/pause.svg"); // 
	ui.begin->setIcon(pixmapBegin);
	
	commandParser = new CCommandParser(informationProcessing, this);
	connect(commandParser, &CCommandParser::sendActionCommand, this, &CActionWidget::receiveCommand);
	totalSeconds = gameInfo.CountDown;
	totalActionNumber = gameInfo.TrainNumber;
	limitTime =gameInfo.LimitTime;
	cutDown = limitTime;
	vectorSize = gameInfo.ActionName.size();
	singleActionNumber = totalSeconds * totalActionNumber / limitTime / vectorSize;
	actionNameVector = gameInfo.ActionName;
	slotUpdateCutDown();
	cutDownTimer = new QTimer(this);
	actionTimer= new QTimer(this);
	actionTimer->setInterval(1000);
	cutDownTimer->setInterval(1000);
	connect(actionTimer, &QTimer::timeout, this, &CActionWidget::checkType);

	connect(cutDownTimer, &QTimer::timeout, this, &CActionWidget::slotUpdateCutDown);
	currentActionType = gameInfo.ActionName.at(0);
	//receiveCommand(gameInfo.ActionName.at(0));
	actionTimer->start();
	cutDownTimer->start();

}

CActionWidget::~CActionWidget()
{
}

int  CActionWidget::getGameResult()
{
	double res = successNumber * 1.00 / (vectorSize * singleActionNumber * 1.00);
	score = res * 100;
	return  score;
}



void CActionWidget::receiveCommand(ActionCommand actionCommand)
{
	if (!cutDownTimer->isActive())
	{
		return;
	}
	switch (actionCommand.ActionType)
	{
	case LEFT_HAND_CLENCHED:
	{
		
		if (actionCommand.ContactRatio==100)
		{
			QPixmap pixmapLEFT_HAND_CLENCHED(":/NXMotionTraining/images/action/LEFT_HAND_CLENCHED.svg"); //
		/*	pixmapLEFT_HAND_CLENCHED = pixmapLEFT_HAND_CLENCHED.scaled(ui.actionpicture->size());*/
			ui.resultpicture->setPixmap(pixmapLEFT_HAND_CLENCHED);
			checkType();
		}
		if (actionCommand.ContactRatio==80)
		{
			QPixmap pixmapLEFT_HAND_CLENCHED80(":/NXMotionTraining/images/action/LEFT_HAND_CLENCHED_80.svg"); //
			/*pixmapLEFT_HAND_CLENCHED80 = pixmapLEFT_HAND_CLENCHED80.scaled(ui.actionpicture->size());*/
			ui.resultpicture->setPixmap(pixmapLEFT_HAND_CLENCHED80);
			checkType();
		}
		if (actionCommand.ContactRatio == 0)
		{
			QPixmap pixmapLEFT_HAND_CLENCHED80(":/NXMotionTraining/images/action/LEFT_HAND_CLENCHED_0.svg"); //
			/*pixmapLEFT_HAND_CLENCHED80 = pixmapLEFT_HAND_CLENCHED80.scaled(ui.actionpicture->size());*/
			ui.resultpicture->setPixmap(pixmapLEFT_HAND_CLENCHED80);
			checkType();
		}

		receiveActionType = actionCommand;
		
		break;
	}
	case ActionType::LEFT_FOOT_EXTENDED:
	{
		QPixmap pixmapLEFT_FOOT_EXTENDED(":/NXMotionTraining/images/action/LEFT_FOOT_EXTENDED.svg"); //
		pixmapLEFT_FOOT_EXTENDED = pixmapLEFT_FOOT_EXTENDED.scaled(ui.actionpicture->size(), Qt::KeepAspectRatio);
		ui.resultpicture->setPixmap(pixmapLEFT_FOOT_EXTENDED);
		receiveActionType = actionCommand;
		checkType();
		break;
	}
	case KICK_LEFT_FOOT:
	{
		QPixmap pixmapKICK_LEFT_FOOT(":/NXMotionTraining/images/action/KICK_LEFT_FOOT.svg"); //
		pixmapKICK_LEFT_FOOT = pixmapKICK_LEFT_FOOT.scaled(ui.actionpicture->size(), Qt::KeepAspectRatio);
		ui.resultpicture->setPixmap(pixmapKICK_LEFT_FOOT);
		receiveActionType = actionCommand;
		checkType();
		break;
	}
	case ActionType::EXTEND_LEFT_HAND:
	{
		QPixmap pixmapEXTEND_LEFT_HAND(":/NXMotionTraining/images/action/EXTEND_LEFT_HAND.svg"); //
		pixmapEXTEND_LEFT_HAND = pixmapEXTEND_LEFT_HAND.scaled(ui.actionpicture->size(), Qt::KeepAspectRatio);
		ui.resultpicture->setPixmap(pixmapEXTEND_LEFT_HAND);
		receiveActionType = actionCommand;
		checkType();
		break;
	}
	case ActionType::LEFT_HAND_DIAGONAL_MOVEMENT:
	{
		QPixmap pixmapLEFT_HAND_DIAGONAL_MOVEMENT(":/NXMotionTraining/images/action/LEFT_HAND_DIAGONAL_MOVEMENT.svg"); //
		pixmapLEFT_HAND_DIAGONAL_MOVEMENT = pixmapLEFT_HAND_DIAGONAL_MOVEMENT.scaled(ui.actionpicture->size(), Qt::KeepAspectRatio);
		ui.resultpicture->setPixmap(pixmapLEFT_HAND_DIAGONAL_MOVEMENT);
		receiveActionType = actionCommand;
		checkType();
		break;
	}
	case ActionType::LEFT_HAND_LEFT_MOVEMENT:
	{
		QPixmap pixmapLEFT_HAND_LEFT_MOVEMENT(":/NXMotionTraining/images/action/LEFT_HAND_LEFT_MOVEMENT.svg"); //
		pixmapLEFT_HAND_LEFT_MOVEMENT = pixmapLEFT_HAND_LEFT_MOVEMENT.scaled(ui.actionpicture->size(), Qt::KeepAspectRatio);
		ui.resultpicture->setPixmap(pixmapLEFT_HAND_LEFT_MOVEMENT);
		receiveActionType = actionCommand;
		checkType();
		break;
	}
	case ActionType::LEFT_HAND_RIGHT_MOVEMENT:
	{
		QPixmap pixmapLEFT_HAND_RIGHT_MOVEMENT(":/NXMotionTraining/images/action/LEFT_HAND_RIGHT_MOVEMENT.svg"); //
		pixmapLEFT_HAND_RIGHT_MOVEMENT = pixmapLEFT_HAND_RIGHT_MOVEMENT.scaled(ui.actionpicture->size(), Qt::KeepAspectRatio);
		ui.resultpicture->setPixmap(pixmapLEFT_HAND_RIGHT_MOVEMENT);
		receiveActionType = actionCommand;
		checkType();
		break;
	}
	case ActionType::LEFT_HAND_UPWARD_MOVEMENT:
	{
		QPixmap pixmapLEFT_HAND_UPWARD_MOVEMENT(":/NXMotionTraining/images/action/LEFT_HAND_UPWARD_MOVEMENT.svg"); //
		pixmapLEFT_HAND_UPWARD_MOVEMENT = pixmapLEFT_HAND_UPWARD_MOVEMENT.scaled(ui.actionpicture->size(), Qt::KeepAspectRatio);
		ui.resultpicture->setPixmap(pixmapLEFT_HAND_UPWARD_MOVEMENT);
		receiveActionType = actionCommand;
		checkType();
		break;
	}
	case ActionType::RAISE_LEFT_LEG:
	{
		QPixmap pixmapRAISE_LEFT_LEG(":/NXMotionTraining/images/action/RAISE_LEFT_LEG.svg"); //

		pixmapRAISE_LEFT_LEG = pixmapRAISE_LEFT_LEG.scaled(ui.actionpicture->size(), Qt::KeepAspectRatio);
		ui.resultpicture->setPixmap(pixmapRAISE_LEFT_LEG);
		receiveActionType = actionCommand;
		checkType();
		break;
	}
	case ActionType::RIGHT_HAND_CLENCHED:
	{
		QPixmap pixmapRIGHT_HAND_CLENCHED(":/NXMotionTraining/images/action/RIGHT_HAND_CLENCHED.svg"); 	pixmapRIGHT_HAND_CLENCHED = pixmapRIGHT_HAND_CLENCHED.scaled(ui.actionpicture->size(), Qt::KeepAspectRatio); 
		ui.resultpicture->setPixmap(pixmapRIGHT_HAND_CLENCHED);
		receiveActionType = actionCommand;
		checkType();
		break;
	}
	case ActionType::EXTEND_RIGHT_HAND:
	{
		QPixmap pixmapEXTEND_RIGHT_HAND(":/NXMotionTraining/images/action/EXTEND_RIGHT_HAND.svg"); //

		pixmapEXTEND_RIGHT_HAND = pixmapEXTEND_RIGHT_HAND.scaled(ui.actionpicture->size(), Qt::KeepAspectRatio);
		ui.resultpicture->setPixmap(pixmapEXTEND_RIGHT_HAND);
		receiveActionType = actionCommand;
		checkType();
		break;
	}
	case ActionType::KICK_RIGHT_FOOT:
	{
		QPixmap pixmapKICK_RIGHT_FOOTD(":/NXMotionTraining/images/action/KICK_RIGHT_FOOT.svg"); //
		pixmapKICK_RIGHT_FOOTD = pixmapKICK_RIGHT_FOOTD.scaled(ui.actionpicture->size(), Qt::KeepAspectRatio);
		ui.resultpicture->setPixmap(pixmapKICK_RIGHT_FOOTD);
		receiveActionType = actionCommand;
		checkType();
		break;
	}
	case ActionType::RAISE_RIGHT_LEG:
	{
		QPixmap pixmapRAISE_RIGHT_LEG(":/NXMotionTraining/images/action/RAISE_RIGHT_LEG.svg"); //

		pixmapRAISE_RIGHT_LEG = pixmapRAISE_RIGHT_LEG.scaled(ui.actionpicture->size(), Qt::KeepAspectRatio);
		ui.resultpicture->setPixmap(pixmapRAISE_RIGHT_LEG);
		receiveActionType = actionCommand;
		checkType();
		break;
	}
	case ActionType::RIGHT_FOOT_EXTENDED:
	{
		QPixmap pixmapRIGHT_FOOT_EXTENDED(":/NXMotionTraining/images/action/RIGHT_FOOT_EXTENDED.svg"); //

		pixmapRIGHT_FOOT_EXTENDED = pixmapRIGHT_FOOT_EXTENDED.scaled(ui.actionpicture->size(), Qt::KeepAspectRatio);
		ui.resultpicture->setPixmap(pixmapRIGHT_FOOT_EXTENDED);
		receiveActionType = actionCommand;
		checkType();
		break;
	}
	case ActionType::RIGHT_HAND_DIAGONAL_MOVEMENT:
	{
		QPixmap pixmapRIGHT_HAND_DIAGONAL_MOVEMENT(":/NXMotionTraining/images/action/RIGHT_HAND_DIAGONAL_MOVEMENT.svg"); //

		pixmapRIGHT_HAND_DIAGONAL_MOVEMENT = pixmapRIGHT_HAND_DIAGONAL_MOVEMENT.scaled(ui.actionpicture->size(), Qt::KeepAspectRatio);
		ui.resultpicture->setPixmap(pixmapRIGHT_HAND_DIAGONAL_MOVEMENT);
		receiveActionType = actionCommand;
		checkType();
		break;
	}
	case ActionType::RIGHT_HAND_DOWNWARD_MOVEMENT:
	{
		QPixmap pixmapRIGHT_HAND_DOWNWARD_MOVEMENT(":/NXMotionTraining/images/action/RIGHT_HAND_DOWNWARD_MOVEMENT.svg"); //

		pixmapRIGHT_HAND_DOWNWARD_MOVEMENT = pixmapRIGHT_HAND_DOWNWARD_MOVEMENT.scaled(ui.actionpicture->size(), Qt::KeepAspectRatio);
		ui.resultpicture->setPixmap(pixmapRIGHT_HAND_DOWNWARD_MOVEMENT);
		receiveActionType = actionCommand;
		checkType();
		break;
	}
	case ActionType::RIGHT_HAND_RIGHT_MOVEMENT:
	{
		QPixmap pixmapRIGHT_HAND_RIGHT_MOVEMENT(":/NXMotionTraining/images/action/RIGHT_HAND_RIGHT_MOVEMENT.svg"); //


		pixmapRIGHT_HAND_RIGHT_MOVEMENT = pixmapRIGHT_HAND_RIGHT_MOVEMENT.scaled(ui.actionpicture->size(), Qt::KeepAspectRatio);
		ui.resultpicture->setPixmap(pixmapRIGHT_HAND_RIGHT_MOVEMENT);
		receiveActionType = actionCommand;
		checkType();
		break;
	}
	case ActionType::RIGHT_HAND_UPWARD_MOVEMENT:
	{
		QPixmap pixmapRIGHT_HAND_UPWARD_MOVEMENT(":/NXMotionTraining/images/action/RIGHT_HAND_UPWARD_MOVEMENT.svg"); //

		pixmapRIGHT_HAND_UPWARD_MOVEMENT = pixmapRIGHT_HAND_UPWARD_MOVEMENT.scaled(ui.actionpicture->size(), Qt::KeepAspectRatio);
		ui.resultpicture->setPixmap(pixmapRIGHT_HAND_UPWARD_MOVEMENT);
		receiveActionType = actionCommand;
		checkType();
		break;
	}
	default:
		break;

	}

}

void CActionWidget::updateWidget(ActionType actionCommand)
{
	switch (actionCommand)
	{
	case LEFT_HAND_CLENCHED:
	{

		//新建QMovie对象并用资源文件初始化
		QMovie* mov = new QMovie(":/NXMotionTraining/images/action/LEFT_HAND_CLENCHED.gif");
		mov->setScaledSize(QSize(ui.actionpicture->width(), ui.actionpicture->height()));
		//播放动画
		mov->start();
		//setMoive
		ui.actionpicture->setMovie(mov);
		ui.actionpicture->setOpenExternalLinks(true);
		currentActionType = actionCommand;
		break;
	}
	case ActionType::LEFT_FOOT_EXTENDED:
	{
		ui.actionpicture->clear();
		QPixmap pixmapLEFT_FOOT_EXTENDED(":/NXMotionTraining/images/action/LEFT_FOOT_EXTENDED.svg"); // 
		ui.actionpicture->setPixmap(pixmapLEFT_FOOT_EXTENDED);
		currentActionType = actionCommand;
		break;
	}
	case KICK_LEFT_FOOT:
	{		ui.actionpicture->clear();
		QPixmap pixmapKICK_LEFT_FOOT(":/NXMotionTraining/images/action/KICK_LEFT_FOOT.svg"); // 
		ui.actionpicture->setPixmap(pixmapKICK_LEFT_FOOT);
		currentActionType = actionCommand;
		
		break;
	}
	case ActionType::EXTEND_LEFT_HAND:
	{		ui.actionpicture->clear();
		QPixmap pixmapEXTEND_LEFT_HAND(":/NXMotionTraining/images/action/EXTEND_LEFT_HAND.svg"); // 
		ui.actionpicture->setPixmap(pixmapEXTEND_LEFT_HAND);
		currentActionType = actionCommand;
		
		break;
	}
	case ActionType::LEFT_HAND_DIAGONAL_MOVEMENT:
	{		ui.actionpicture->clear();
		QPixmap pixmapLEFT_HAND_DIAGONAL_MOVEMENT(":/NXMotionTraining/images/action/LEFT_HAND_DIAGONAL_MOVEMENT.svg"); // 
		ui.actionpicture->setPixmap(pixmapLEFT_HAND_DIAGONAL_MOVEMENT);
		currentActionType = actionCommand;
		
		break;
	}
	case ActionType::LEFT_HAND_LEFT_MOVEMENT:
	{		ui.actionpicture->clear();
		QPixmap pixmapLEFT_HAND_LEFT_MOVEMENT(":/NXMotionTraining/images/action/LEFT_HAND_LEFT_MOVEMENT.svg"); // 
		ui.actionpicture->setPixmap(pixmapLEFT_HAND_LEFT_MOVEMENT);
		currentActionType = actionCommand;
		
		break;
	}
	case ActionType::LEFT_HAND_RIGHT_MOVEMENT:
	{		ui.actionpicture->clear();
		QPixmap pixmapLEFT_HAND_RIGHT_MOVEMENT(":/NXMotionTraining/images/action/LEFT_HAND_RIGHT_MOVEMENT.svg"); // 
		ui.actionpicture->setPixmap(pixmapLEFT_HAND_RIGHT_MOVEMENT);
		currentActionType = actionCommand;
		
		break;
	}
	case ActionType::LEFT_HAND_UPWARD_MOVEMENT:
	{		ui.actionpicture->clear();
		QPixmap pixmapLEFT_HAND_UPWARD_MOVEMENT(":/NXMotionTraining/images/action/LEFT_HAND_UPWARD_MOVEMENT.svg"); // 
		ui.actionpicture->setPixmap(pixmapLEFT_HAND_UPWARD_MOVEMENT);
		currentActionType = actionCommand;
	
		break;
	}
	case ActionType::RAISE_LEFT_LEG:
	{		ui.actionpicture->clear();
		QPixmap pixmapRAISE_LEFT_LEG(":/NXMotionTraining/images/action/RAISE_LEFT_LEG.svg"); // 
		ui.actionpicture->setPixmap(pixmapRAISE_LEFT_LEG);
		currentActionType = actionCommand;
		
		break;
	}
	case ActionType::RIGHT_HAND_CLENCHED:
	{		ui.actionpicture->clear();
		QPixmap pixmapRIGHT_HAND_CLENCHED(":/NXMotionTraining/images/action/RIGHT_HAND_CLENCHED.svg"); // 
		ui.actionpicture->setPixmap(pixmapRIGHT_HAND_CLENCHED);
		
		currentActionType = actionCommand;
		break;
	}
	case ActionType::EXTEND_RIGHT_HAND:
	{		ui.actionpicture->clear();
		QPixmap pixmapEXTEND_RIGHT_HAND(":/NXMotionTraining/images/action/EXTEND_RIGHT_HAND.svg"); // 
		ui.actionpicture->setPixmap(pixmapEXTEND_RIGHT_HAND);
		
		currentActionType = actionCommand;
		break;
	}
	case ActionType::KICK_RIGHT_FOOT:
	{		ui.actionpicture->clear();
		QPixmap pixmapKICK_RIGHT_FOOTD(":/NXMotionTraining/images/action/KICK_RIGHT_FOOT.svg"); // 
		ui.actionpicture->setPixmap(pixmapKICK_RIGHT_FOOTD);
		currentActionType = actionCommand;
		
		break;
	}
	case ActionType::RAISE_RIGHT_LEG:
	{		ui.actionpicture->clear();
		QPixmap pixmapRAISE_RIGHT_LEG(":/NXMotionTraining/images/action/RAISE_RIGHT_LEG.svg"); // 
		ui.actionpicture->setPixmap(pixmapRAISE_RIGHT_LEG);
		currentActionType = actionCommand;
		
		break;
	}
	case ActionType::RIGHT_FOOT_EXTENDED:
	{		ui.actionpicture->clear();
		QPixmap pixmapRIGHT_FOOT_EXTENDED(":/NXMotionTraining/images/action/RIGHT_FOOT_EXTENDED.svg"); // 
		ui.actionpicture->setPixmap(pixmapRIGHT_FOOT_EXTENDED);
		
		currentActionType = actionCommand;
		break;
	}
	case ActionType::RIGHT_HAND_DIAGONAL_MOVEMENT:
	{		ui.actionpicture->clear();
		QPixmap pixmapRIGHT_HAND_DIAGONAL_MOVEMENT(":/NXMotionTraining/images/action/RIGHT_HAND_DIAGONAL_MOVEMENT.svg"); // 
		ui.actionpicture->setPixmap(pixmapRIGHT_HAND_DIAGONAL_MOVEMENT);
		currentActionType = actionCommand;
	
		break;
	}
	case ActionType::RIGHT_HAND_DOWNWARD_MOVEMENT:
	{		ui.actionpicture->clear();
		QPixmap pixmapRIGHT_HAND_DOWNWARD_MOVEMENT(":/NXMotionTraining/images/action/RIGHT_HAND_DOWNWARD_MOVEMENT.svg"); // 
		ui.actionpicture->setPixmap(pixmapRIGHT_HAND_DOWNWARD_MOVEMENT);
		currentActionType = actionCommand;
		
		break;
	}
	case ActionType::RIGHT_HAND_RIGHT_MOVEMENT:
	{		ui.actionpicture->clear();
		QPixmap pixmapRIGHT_HAND_RIGHT_MOVEMENT(":/NXMotionTraining/images/action/RIGHT_HAND_RIGHT_MOVEMENT.svg"); // 
		ui.actionpicture->setPixmap(pixmapRIGHT_HAND_RIGHT_MOVEMENT);
		currentActionType = actionCommand;
	
		break;
	}
	case ActionType::RIGHT_HAND_UPWARD_MOVEMENT:
	{		ui.actionpicture->clear();
		QPixmap pixmapRIGHT_HAND_UPWARD_MOVEMENT(":/NXMotionTraining/images/action/RIGHT_HAND_UPWARD_MOVEMENT.svg"); // 
		ui.actionpicture->setPixmap(pixmapRIGHT_HAND_UPWARD_MOVEMENT);
		currentActionType = actionCommand;
		
		break;
	}
	default:
		break;

	}


}
void CActionWidget::checkType()
{
	if (!cutDownTimer->isActive())
	{
		return;
	}
	else
	{
	if (cutDown > 0)
	{
		cutDown= cutDown-1;
		ui.cutDown->setText(QString("%1").arg(cutDown));

		if (currentActionType == receiveActionType.ActionType&& receiveActionType.ContactRatio == 100)
		{
			successNumber++;
			cutDown = limitTime;
			ui.cutDown->setText(QString("%1").arg(limitTime));
			gameNumber++;
		}
	}
	else
	{
		cutDown = limitTime;
		ui.cutDown->setText(QString("%1").arg(limitTime));
		gameNumber++;
		if (currentActionType == receiveActionType.ActionType&& receiveActionType.ContactRatio==100)
		{
			successNumber++;
			cutDown = limitTime;
			ui.cutDown->setText(QString("%1").arg(limitTime));
		}

		else
		{
			FailNumber++;
		}
	}

			

	if(currentSingleActionNumber<singleActionNumber)
	{
		currentSingleActionNumber++;
		if(currentNumber < vectorSize)
		{

			updateWidget(actionNameVector.at(currentNumber));

			ui.counts->setText(QString::number(gameNumber));
		}
		else
		{
			if (!isEnd)
			{
				isEnd = true;
				emit  endGame();
				cutDownTimer->stop();
			}
		}
	}
	else
	{
		currentSingleActionNumber = 0;
		if (currentNumber <vectorSize)
		{
			updateWidget(actionNameVector.at(currentNumber));
			ui.counts->setText(QString::number(gameNumber));
			currentNumber++;
		}
		
	
	}

	actionTimer->stop();
	actionTimer->start();
	}

}



void CActionWidget::slotChangeGameStatus()
{
	if (gameStatus == START)
	{
		gameStatus = STOP;
		QPixmap pixmap(":/NXMotionTraining/images/trainPage/pause.svg"); // 
		ui.begin->setIcon(pixmap);

	}
	else if (gameStatus == STOP)
	{
		gameStatus = CONTINUE;
		QPixmap pixmap(":/NXMotionTraining/images/trainPage/play.svg"); // 
		ui.begin->setIcon(pixmap);
		cutDownTimer->stop();
	}
	else if (gameStatus == CONTINUE)
	{
		gameStatus = STOP;
		QPixmap pixmap(":/NXMotionTraining/images/trainPage/pause.svg"); // 
		ui.begin->setIcon(pixmap);
		cutDownTimer->start();
	}

}

void CActionWidget::slotUpdateCutDown()
{
	if (totalSeconds >= 0)
	{

		const int minutes = (totalSeconds % 3600) / 60; // 计算分钟
		const int seconds = totalSeconds % 60; // 计算秒数
	
		emit changeCutDownNumber(minutes, seconds);
		totalSeconds--;
	}
	if (totalSeconds == 0)
	{
		emit changeCutDownNumber(0, 0);
		if(!isEnd)
		{
			isEnd = true;
			emit  endGame();
			cutDownTimer->stop();
		}
	
	}


}

END_NX_NAMESPACE
