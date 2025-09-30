#pragma once
#include <QWidget>
#include "ui_GameStartCountdown.h"
#include "Global.h"
#include <QTimer>
BEGIN_NX_NAMESPACE
class GameStartCountdown : public QWidget
{
	Q_OBJECT

public:
	GameStartCountdown(QWidget *parent = nullptr);
	~GameStartCountdown();
	signals:
		void startGame();
private:
	void cutDown();
private slots:
	void slotChangeGameStatus();
	void slotChangeVoiceStatus();
private:
	Ui::GameStartCountdownClass ui;
	QTimer* cutDownTimer;
	int cutDownNumber = 5;
	int gameStatus = 0;
	bool isMute = false;
};
END_NX_NAMESPACE