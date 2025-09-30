#include "GameStartCountdown.h"
#include <QFontDatabase>
BEGIN_NX_NAMESPACE
GameStartCountdown::GameStartCountdown(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	auto boldId = QFontDatabase::addApplicationFont(QStringLiteral("./SiYuanHeiTi/SourceHanSansCN-Bold.ttf"));
	auto boldFamilies = QFontDatabase::applicationFontFamilies(boldId);

	auto fontId = QFontDatabase::addApplicationFont(QStringLiteral("./SiYuanHeiTi/SourceHanSansCN-Normal.ttf"));
	QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);

	QFont font_bold(boldFamilies.at(0), 80, QFont::Bold);
	QFont font_normal(fontFamilies.at(0), 12);
	cutDownTimer = new QTimer(this);
	cutDownTimer->setInterval(1000);
	connect(cutDownTimer, &QTimer::timeout, this, &GameStartCountdown::cutDown);
	connect(ui.startBtn, &QPushButton::clicked, this, &GameStartCountdown::slotChangeGameStatus);
	connect(ui.voiceBtn, &QPushButton::clicked, this, &GameStartCountdown::slotChangeVoiceStatus);
	ui.cutDown->setFont(font_bold);
	ui.cutDown->setText(QString("%1").arg(cutDownNumber));
	cutDownTimer->start();
	ui.voiceBtn->setStyleSheet("border-radius: 20px; image: url(:/NXMotionTraining/images/common/mute.svg);");
}

GameStartCountdown::~GameStartCountdown()
{
}


void GameStartCountdown::cutDown()
{
	if (cutDownNumber>0)
	{
		cutDownNumber = cutDownNumber - 1;
		ui.cutDown->setText(QString("%1").arg(cutDownNumber));
	}
	else
	{
		cutDownTimer->stop();
		emit  startGame();

	}

}

void GameStartCountdown::slotChangeGameStatus()
{
	if (gameStatus == 0)
	{
		gameStatus = 1;
		ui.startBtn->setStyleSheet("border-radius: 20px; image: url(:/NXMotionTraining/images/trainPage/play.svg);");
		cutDownTimer->stop();

	}
	else if (gameStatus == 1)
	{
		gameStatus = 0;
		
		QPixmap pixmap(":/NXMotionTraining/images/trainPage/pause.svg"); //
		pixmap.scaled(QSize(ui.startBtn->width(), ui.startBtn->height()));
		ui.startBtn->setStyleSheet("border-radius: 20px; image: url(:/NXMotionTraining/images/trainPage/pause.svg);");


	}
	
}

void GameStartCountdown::slotChangeVoiceStatus()
{
	if (!isMute)
	{
		isMute=true;
		ui.voiceBtn->setStyleSheet("border-radius: 20px; image: url(:/NXMotionTraining/images/common/mute.svg);");
	

	}
	else
	{
		isMute = false;
		ui.voiceBtn->setStyleSheet("border-radius: 20px; image: url(:/NXMotionTraining/images/common/non-mute.svg);");
	}
	
}

END_NX_NAMESPACE
