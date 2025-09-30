#include "CAnalysisWidget.h"
#include "CResultDisplayWidget.h"
#include "CHistoryDataWidget.h"
#include  "../NxAppService/CInformationProcessing.h"
#include  "../NxAppService/CHistoryDataAnalysis.h"
#include <ylt/struct_json/json_writer.h>


BEGIN_NX_NAMESPACE
CAnalysisWidget::CAnalysisWidget(CInformationProcessing* informationProcessing,QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	resultDisplayWidget = new CResultDisplayWidget(this);
	historyDataWidget = new CHistoryDataWidget(this);
	historyDataAnalysis = new CHistoryDataAnalysis(informationProcessing, this);
	connect(historyDataAnalysis, &CHistoryDataAnalysis::sendAnalysisMessage, this, &CAnalysisWidget::setAnalysis);
	connect(ui.continueGame, &QPushButton::clicked, this, &CAnalysisWidget::continueGame);
	connect(ui.end, &QPushButton::clicked, this, &CAnalysisWidget::shutDown);
	ui.history->addWidget(historyDataWidget);
	ui.result->addWidget(resultDisplayWidget);
}

CAnalysisWidget::~CAnalysisWidget()
{


}

void CAnalysisWidget::setAnalysis(QMap<QString, QString> data)
{
	historyDataWidget->showData(data);
	resultDisplayWidget->setBackground(score);
}

void CAnalysisWidget::analysisRequest(CInformationProcessing* informationProcessing, QString beginTime, QString endTime)
{
	TransInformation ts;
	ts.MessageType = ANALYSIS;
	ts.Analysis.BeginTime = beginTime.toStdString();
	ts.Analysis.EndTime= endTime.toStdString();
	std::string analysisRequests;
	struct_json::to_json(ts, analysisRequests); // ÐòÁÐ»¯
	informationProcessing->sendMessage(QString::fromStdString(analysisRequests));
}

void CAnalysisWidget::continueGame()
{
	emit initGame();
}


END_NX_NAMESPACE
