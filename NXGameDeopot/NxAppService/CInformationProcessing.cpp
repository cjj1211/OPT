#include "CInformationProcessing.h"
#include "../NXWebSocketClient/CWebSocketClient.h"
#include "../NxSystemCfg/CSystemCfg.h"
#include <ylt/struct_json/json_reader.h>
#include <QMap>
CInformationProcessing::CInformationProcessing(QObject *parent)
	: QObject(parent)
{
	
	
}


void CInformationProcessing::receiveMessage(const QString& receiveMessage)
{
	TransInformation transInformation;
	struct_json::from_json(transInformation, receiveMessage.toStdString()); // ∑¥–Ú¡–ªØ
	switch (transInformation.MessageType)
	{
		case PATIENT_INFO:  // NOLINT(bugprone-branch-clone)
			break;
		case GAME_INFO:
			break;
		case ANALYSIS:
		{
			QMap<QString, QString> qmap;
			for (const auto& pair : transInformation.Analysis.AnalysisData) {
				QString key = QString::fromStdString(pair.first);
				QString value = QString::fromStdString(pair.second);
				qmap.insert(key, value);
			}
			emit receiveAnalysisMessage(qmap);
			break;
		}
		case COMMAND:
			emit  receiveCommandMessage(transInformation.ActionCommand);
		break;
		case DEVICE_STATUS:  
			emit receiveDeviceMessage(transInformation.DeviceStatus);
			break;
		case EVENT:

		break;
		case GAME_CONTROL:
			if (transInformation.GameControl.GameStatus==START)
			{
				emit initGame(transInformation);
			}
		break;
	}

}

void CInformationProcessing::sendMessage(const QString& sendMessage)
{
	// web  send
	emit webSocketClient->SendTextMessageSignal(sendMessage);
}

bool CInformationProcessing::isConnect()
{
	
	return  isClientConnect;
}

void CInformationProcessing::initClient()
{
	auto commmn = systemConfig.GetSystemCfg().Common;
	webSocketClient = new CWebSocketClient(QString::fromStdString(commmn.ClientAddress), commmn.HeartBeatTime, commmn.MissedHeartbeatsThreshold);
	connect(webSocketClient, &CWebSocketClient::ReceiveMessage, this, &CInformationProcessing::receiveMessage);

}

void CInformationProcessing::continueGame()
{
	webSocketClient->initGame();
}
