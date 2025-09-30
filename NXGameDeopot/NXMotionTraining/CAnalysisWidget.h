#pragma once

#include "Global.h"
#include "ui_CAnalysisWidget.h"
#include <QLabel>
BEGIN_NX_NAMESPACE
class CResultDisplayWidget;
class CHistoryDataWidget;
class  CInformationProcessing;
class CHistoryDataAnalysis;
class CAnalysisWidget : public QWidget
{
	Q_OBJECT
		
public:
	CAnalysisWidget(CInformationProcessing *informationProcessing, QWidget *parent = nullptr);
	~CAnalysisWidget();
	void setAnalysis(QMap<QString, QString>data);
	void analysisRequest(CInformationProcessing* informationProcessing,QString beginTime, QString endTime);
	void continueGame();
public:
	int score;
signals:
	void initGame();
	void shutDown();
private:
	Ui::CAnalysisWidgetClass ui;
	CResultDisplayWidget* resultDisplayWidget;
	CHistoryDataWidget* historyDataWidget;
	QLabel* gifLabel;
	CHistoryDataAnalysis* historyDataAnalysis;


};

END_NX_NAMESPACE
