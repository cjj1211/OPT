#pragma once
#include "nxappservice_global.h"
#include <QObject>
#include <QMap>
BEGIN_NX_NAMESPACE
class CInformationProcessing;
class NXAPPSERVICE_EXPORT CHistoryDataAnalysis  : public QObject
{
	Q_OBJECT

public:
	CHistoryDataAnalysis(CInformationProcessing* informationProcessing, QObject *parent);
	~CHistoryDataAnalysis();
private:
	void receiveAnalysisMessage(QMap<QString, QString> historyData);
	signals:
		void sendAnalysisMessage(QMap<QString, QString> historyData);
};
END_NX_NAMESPACE