/******************************************************************************
 *  版权所有（C）2022-2024，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *  创建日期:    2024
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __TRANS_INFORMATION_H
#define __TRANS_INFORMATION_H
#include "MessageBase.h"
#include <string>
#include <iguana/json_reader.hpp>
BEGIN_NX_NAMESPACE
enum MessageType {
	PATIENT_INFO = 0,
	GAME_INFO,
	DEVICE_STATUS,
	EVENT,
	ANALYSIS,
	COMMAND,
	GAME_CONTROL
};
enum ActionType {
	LEFT_HAND_CLENCHED = 0,
	EXTEND_LEFT_HAND,
	LEFT_FOOT_EXTENDED,
	KICK_LEFT_FOOT,
	RAISE_LEFT_LEG,
	LEFT_HAND_UPWARD_MOVEMENT,
	LEFT_HAND_LEFT_MOVEMENT,
	LEFT_HAND_RIGHT_MOVEMENT,
	LEFT_HAND_DIAGONAL_MOVEMENT,
	EXTEND_RIGHT_HAND,
	RIGHT_HAND_CLENCHED,
	RIGHT_FOOT_EXTENDED,
	KICK_RIGHT_FOOT,
	RAISE_RIGHT_LEG,
	RIGHT_HAND_UPWARD_MOVEMENT,
	RIGHT_HAND_DOWNWARD_MOVEMENT,
	RIGHT_HAND_RIGHT_MOVEMENT,
	RIGHT_HAND_DIAGONAL_MOVEMENT

};
enum GameStatus {
	START = 0,
	STOP,
	CONTINUE,
	END
};
struct Patient : MessageBase
{
	std::string  Avatar;//头像
	std::string PatientName;//患者姓名
};
REFLECTION(Patient, Avatar, PatientName, CurrentDateTime);
struct GameInfo : MessageBase
{
	int TrainNumber;  //训练总次数
	std::vector<ActionType> ActionName;//训练名
	int CountDown;//倒计时分钟
	int LimitTime;
	bool IsVoice;//是否开启语音提示
	int Volume;//音量
	int Score;
	int SuccessNumber;
	int FailNumber;

	
};
REFLECTION(GameInfo, TrainNumber, ActionName, CountDown, IsVoice, Volume,CurrentDateTime, LimitTime, Score, SuccessNumber, FailNumber);
struct DeviceStatus : MessageBase
{
	int Master;//主控状体
	int Outboard;//外挂机状态
};
REFLECTION(DeviceStatus, Master, Outboard,CurrentDateTime);
struct Event : MessageBase
{
	std::string EventName;
};
REFLECTION(Event, EventName,CurrentDateTime);
struct ActionCommand : MessageBase
{
	ActionType ActionType;
	int ContactRatio;
};
REFLECTION(ActionCommand, ActionType, CurrentDateTime, ContactRatio);
struct GameControl : MessageBase
{
	GameStatus GameStatus;
};
REFLECTION(GameControl, GameStatus, CurrentDateTime);
struct Analysis : MessageBase
{
	std::string BeginTime;
	std::string EndTime;
	std::map<std::string,std::string> AnalysisData;
};
REFLECTION(Analysis, AnalysisData, CurrentDateTime, BeginTime, EndTime);
struct TransInformation :public MessageBase
{
	MessageType MessageType;
	Patient Patient;
	GameInfo GameInfo;
	DeviceStatus DeviceStatus;
	Event Event;
	ActionCommand ActionCommand;
	GameControl GameControl;
	Analysis Analysis;
};
REFLECTION(TransInformation, MessageType, CurrentDateTime, Patient, GameInfo, DeviceStatus, Event, ActionCommand, GameControl, Analysis);
END_NX_NAMESPACE
#endif
