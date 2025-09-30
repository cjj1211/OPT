#include "CHistoryDataAnalysis.h"
#include "CInformationProcessing.h"
BEGIN_NX_NAMESPACE
CHistoryDataAnalysis::CHistoryDataAnalysis(CInformationProcessing* informationProcessing, QObject *parent)
	: QObject(parent)
{

	connect(informationProcessing, &CInformationProcessing::receiveAnalysisMessage, this, &CHistoryDataAnalysis::receiveAnalysisMessage);
}

CHistoryDataAnalysis::~CHistoryDataAnalysis()
{


}

void CHistoryDataAnalysis::receiveAnalysisMessage(QMap<QString, QString> historyData)
{
	emit sendAnalysisMessage(historyData);
}

END_NX_NAMESPACE
