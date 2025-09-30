#include "CWebSocketService.h"
#include <QRandomGenerator>
#include "CWebSocketServer.h"
#include "NxDBManager/CTreatmentRecordRepository.h"
#include "NxEntity/TreatmentRecord.h"
#include <ylt/struct_json/json_writer.h>
#include <ylt/struct_json/json_reader.h>
#include <QImage>
BEGIN_NX_NAMESPACE
	CWebSocketService::CWebSocketService(int port, QObject* parent)
		:treatmentRepository(std::make_unique<CTreatmentRecordRepository>())
{
	webSocketServer = new CWebSocketServer(port, 3000, 5, this);
	connect(webSocketServer, &CWebSocketServer::ReceiveMessage, this, &CWebSocketService::onMessageReceived);
	testTimer = new QTimer(this);
	testTimer->setInterval(3000);
	connect(testTimer, &QTimer::timeout, this, &CWebSocketService::sendTestAction);
	testTimer->start();
}

	CWebSocketService::~CWebSocketService()
	{

	}


	void CWebSocketService::onMessageReceived(const QString& message)
{
		if (message == "InitGame")
		{
			PatientInfo patient;
			patient.PatientName = patient_.PatientName;
			patient_.UpdateDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString();;
			if (!identity_info.Avatar.empty()) {
				QByteArray ba;
				for (const char i : identity_info.Avatar)
				{
					ba.append(i);
				}

				
				patient.Avatar = QString::fromLatin1(ba.toBase64().data()).toStdString();
			}
		
			GameInfo gameInfo;
			gameInfo.TrainNumber = standard_treatment_.TrialCounts;
			gameInfo.CountDown = standard_treatment_.TrialTime * 60;
			gameInfo.LimitTime = standard_treatment_.CountDown;
			QStringList actionList;
			actionList = QString::fromStdString(standard_treatment_.Action).split(",");
			std::vector<ActionType>vec;
			for (auto action: actionList)
			{
				vec=chooseAction(vec,action.toInt()-1);
			}
			gameInfo.ActionName = vec;
			TransInformation ts;
			ts.MessageType = GAME_CONTROL;
			GameControl gm;
			gm.GameStatus = START;
			ts.GameControl = gm;
			ts.GameInfo = gameInfo;
			ts.Patient = patient;
			std::string initInfo;
			struct_json::to_json(ts, initInfo); // 序列化

			webSocketServer->SendTextMessage(QString::fromStdString(initInfo));
		}
		else
		{
			TransInformation transInformation;
			struct_json::from_json(transInformation, message.toStdString()); // 反序列化
			if (transInformation.MessageType == MessageType::EVENT)
			{
				/*ui->receivetext->setText(QString::fromStdString(transInformation.Event.CurrentDateTime + ":" + transInformation.Event.EventName));*/
			}
			if (transInformation.MessageType == ANALYSIS)
			{
				/*ui->receivetext->setText(QString::fromStdString(transInformation.Analysis.BeginTime + "-----" + transInformation.Analysis.EndTime));*/
				std::map<std::string, std::string>map;
				map["2024-03-04"] = "30";
				map["2024-03-08"] = "80";
				map["2024-03-11"] = "100";
				map["2024-03-14"] = "95";
				map["2024-03-24"] = "96";

				;
				transInformation.Analysis.AnalysisData = map;
				std::string test;
				struct_json::to_json(transInformation, test); // 序列化
				webSocketServer->SendTextMessage(QString::fromStdString(test));

			}
			if (transInformation.MessageType == MessageType::GAME_CONTROL)
			{
				if (transInformation.GameControl.GameStatus==END)
				{
					TreatmentRecord treatmentRecord;
					treatmentRecord.Difficult = standard_treatment_.Difficult;
					treatmentRecord.TrainType = Action;
					treatmentRecord.FailTimes = transInformation.GameInfo.FailNumber;
					treatmentRecord.Score = transInformation.GameInfo.Score;
					treatmentRecord.SuccessTimes = transInformation.GameInfo.SuccessNumber;
					treatmentRecord.PatientFK = patient_.UID;
					treatmentRecord.StartDatetime = patient_.UpdateDateTime;
					int result = (transInformation.GameInfo.Score > standard_treatment_.MinScore) ? 1 : 0;
					treatmentRecord.IsSuccess = result;
					treatmentRecord.TrainTimeLen = QDateTime::fromString(QString::fromStdString(patient_.UpdateDateTime), "yyyy-MM-dd hh:mm:ss").secsTo(QDateTime::fromString(QString::fromStdString(transInformation.GameInfo.CurrentDateTime), "yyyy-MM-dd hh:mm:ss"));
					treatmentRepository->Insert(treatmentRecord);
				}
			}
		}

}

	void CWebSocketService::sendTestAction()
	{

		TransInformation testTransInformation;
		QRandomGenerator generator;
		testTransInformation.MessageType = COMMAND;
		testTransInformation.ActionCommand.ActionType = LEFT_HAND_CLENCHED;
		if (currentAction==0)
		{
			testTransInformation.ActionCommand.ContactRatio = 80;
		}
		if (currentAction==80)
		{
			testTransInformation.ActionCommand.ContactRatio = 100;
		}
		if (currentAction==100)
		{
			testTransInformation.ActionCommand.ContactRatio = 0;
		}
		currentAction = testTransInformation.ActionCommand.ContactRatio;
		std::string test;
		struct_json::to_json(testTransInformation, test); // 序列化
		webSocketServer->SendTextMessage(QString::fromStdString(test));
		/*if (randomNumber==0)
		{
			testTransInformation.ActionCommand.ContactRatio = 0;
		}
		if (randomNumber==1)
		{
			testTransInformation.ActionCommand.ContactRatio = 80;
		}
		if(randomNumber==2)
		{
			testTransInformation.ActionCommand.ContactRatio = 100;
		}*/

		// 生成 0 到 18 之间的随机整数
		//int  randomNumber = generator.bounded(19); // 生成的随机数范围是 [0, 19)
		//if (currentAction!= randomNumber)
		//{
		//	currentAction = randomNumber;
		//}
		//else
		//{
		//	currentAction++;
		//}
		//testTransInformation=	chooseAction(testTransInformation, currentAction);

	}

	TransInformation CWebSocketService::chooseAction(TransInformation transInformation, int action)
	{
		switch (action)
		{
		case 0:transInformation.ActionCommand.ActionType = LEFT_HAND_CLENCHED;
			break;
		case 1:transInformation.ActionCommand.ActionType = EXTEND_LEFT_HAND;
			break;
		case 2:transInformation.ActionCommand.ActionType = LEFT_FOOT_EXTENDED;
			break;
		case 3:transInformation.ActionCommand.ActionType = KICK_LEFT_FOOT;
			break;
		case 4:transInformation.ActionCommand.ActionType = RAISE_LEFT_LEG;
			break;
		case 5:transInformation.ActionCommand.ActionType = LEFT_HAND_UPWARD_MOVEMENT;
			break;
		case 6:transInformation.ActionCommand.ActionType = LEFT_HAND_LEFT_MOVEMENT;
			break;
		case 7:transInformation.ActionCommand.ActionType = LEFT_HAND_RIGHT_MOVEMENT;
			break;
		case 8:transInformation.ActionCommand.ActionType = LEFT_HAND_DIAGONAL_MOVEMENT;
			break;
		case 9:transInformation.ActionCommand.ActionType = EXTEND_RIGHT_HAND;
			break;
		case 10:transInformation.ActionCommand.ActionType = RIGHT_HAND_CLENCHED;
			break;
		case 11:transInformation.ActionCommand.ActionType = RIGHT_FOOT_EXTENDED;
			break;
		case 12:transInformation.ActionCommand.ActionType = KICK_RIGHT_FOOT;
			break;
		case 13:transInformation.ActionCommand.ActionType = RAISE_RIGHT_LEG;
			break;
		case 14:transInformation.ActionCommand.ActionType = RIGHT_HAND_UPWARD_MOVEMENT;
			break;
		case 15:transInformation.ActionCommand.ActionType = RIGHT_HAND_DOWNWARD_MOVEMENT;
			break;
		case 16:transInformation.ActionCommand.ActionType = RIGHT_HAND_RIGHT_MOVEMENT;
			break;
		case 17:transInformation.ActionCommand.ActionType = RIGHT_HAND_DIAGONAL_MOVEMENT;
			break;
		default:
			break;
		}
		return transInformation;
	}

	std::vector<ActionType> CWebSocketService::chooseAction(std::vector<ActionType> actionVector, int action)
	{
		switch (action)
		{
		case 0:actionVector.push_back(LEFT_HAND_CLENCHED) ;
			break;
		case 1:actionVector.push_back( EXTEND_LEFT_HAND);
			break;
		case 2:actionVector.push_back (LEFT_FOOT_EXTENDED);
			break;
		case 3:actionVector.push_back (KICK_LEFT_FOOT);
			break;
		case 4:actionVector.push_back (RAISE_LEFT_LEG);
			break;
		case 5:actionVector.push_back (LEFT_HAND_UPWARD_MOVEMENT);
			break;
		case 6:actionVector.push_back (LEFT_HAND_LEFT_MOVEMENT);
			break;
		case 7:actionVector.push_back (LEFT_HAND_RIGHT_MOVEMENT);
			break;
		case 8:actionVector.push_back (LEFT_HAND_DIAGONAL_MOVEMENT);
			break;
		case 9:actionVector.push_back (EXTEND_RIGHT_HAND);
			break;
		case 10:actionVector.push_back (RIGHT_HAND_CLENCHED);
			break;
		case 11:actionVector.push_back (RIGHT_FOOT_EXTENDED);
			break;
		case 12:actionVector.push_back (KICK_RIGHT_FOOT);
			break;
		case 13:actionVector.push_back (RAISE_RIGHT_LEG);
			break;
		case 14:actionVector.push_back (RIGHT_HAND_UPWARD_MOVEMENT);
			break;
		case 15:actionVector.push_back (RIGHT_HAND_DOWNWARD_MOVEMENT);
			break;
		case 16:actionVector.push_back( RIGHT_HAND_RIGHT_MOVEMENT);
			break;
		case 17:actionVector.push_back (RIGHT_HAND_DIAGONAL_MOVEMENT);
			break;

		default:
			break;
		}
		return actionVector;
	}

	void CWebSocketService::initGame(IdentityInfo identityInfo, Patient patient, StandardTreatment standardTreatment)
{
		identity_info = identityInfo;
		patient_ = patient;
		standard_treatment_ = standardTreatment;
}

	void CWebSocketService::sendActionType(ActionType type)
	{
		TransInformation testTransInformation;
		
		testTransInformation.MessageType = COMMAND;
		
		testTransInformation.ActionCommand.ActionType= type;
		std::string actionType;
		struct_json::to_json(testTransInformation, actionType); // 序列化
		webSocketServer->SendTextMessage(QString::fromStdString(actionType));
	}

END_NX_NAMESPACE
