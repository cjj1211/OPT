#include "CDBManager.h"
#include "../NxEntity/User.h"
#include <QString>


BEGIN_NX_NAMESPACE


std::shared_ptr<dbng<mysql>> CDBManager::getDbManager() {
	static std::shared_ptr<dbng<mysql>> instance = std::make_shared<dbng<mysql>>();
	return instance;
}




void CDBManager::initDB(std::string ip, std::string user, std::string password, std::string name,int port)
{
	LOG_INFO("initDB........................start");
	auto instance = getDbManager();
	bool res = instance->connect(ip.c_str(), user.c_str(), password.c_str(), name.c_str(), -1, port);
	// 连接前，需要创建数据库，否则会返回Unknown Database错误
	if (!res)
	{
		instance->disconnect();
		MYSQL mysqlInstance;
		
		mysql_init(&mysqlInstance);
		auto res = mysql_real_connect(&mysqlInstance, ip.c_str(), user.c_str(), password.c_str(), "sys", port, 0, 0);

		std::string queryStr = "create database if not exists ";
		queryStr += name + " ;";


		if (0 == mysql_query(&mysqlInstance, queryStr.c_str()))
		{
			queryStr = "use ";
			queryStr += name;
			if (0 == mysql_query(&mysqlInstance, queryStr.c_str()))
			{
				LOG_INFO("Create database " + name + " success.");
			}
			else {
				LOG_ERR(instance->get_last_error().c_str());
			}
		}
		else
		{
			LOG_ERR(instance->get_last_error().c_str());
		}
		instance->connect(ip.c_str(), user.c_str(), password.c_str(), name.c_str(), -1, port);
	}

	res = instance->execute("CREATE DATABASE IF NOT EXISTS `optimus`");

	std::string sql = "CREATE TABLE IF NOT EXISTS `IdentityInfo`( \
		UID VARCHAR(256)  PRIMARY KEY , UNIQUE(UID), \
		CreateDateTime DATETIME NOT NULL, \
		UpdateDateTime   DATETIME NOT NULL, \
		IsDeleted   TINYINT NOT NULL, \
		Password   VARCHAR(256)  NOT NULL, \
		Avatar   BLOB  NOT NULL, \
		Icon   BLOB  NOT NULL, \
		Feature  BLOB  NOT NULL, \
        LoginCounts   TINYINT , \
        InitialLogin TINYINT  , \
        LogInName   VARCHAR(256)  NOT NULL,\
		Role   TINYINT  NOT NULL) DEFAULT CHARSET=utf8";
	res = instance->execute(sql);



	sql = "CREATE TABLE IF NOT EXISTS `VersionInfo`( \
		UID VARCHAR(256)  PRIMARY KEY , UNIQUE(UID), \
		DbVersion VARCHAR(256) NOT NULL, \
		SoftwareVersion   VARCHAR(256) NOT NULL\
       ) DEFAULT CHARSET=utf8";
	res = instance->execute(sql);

	sql = "INSERT INTO `optimus`.`versioninfo` (`UID`, `DbVersion`, `SoftwareVersion`) VALUES('1', '0.10.0', '0.10.0');";
	res = instance->execute(sql);
	


	sql = "CREATE TABLE IF NOT EXISTS `User`( \
		UID VARCHAR(256)  PRIMARY KEY , UNIQUE(UID), \
	CreateDateTime DATETIME NOT NULL, \
	UpdateDateTime   DATETIME NOT NULL, \
	IsDeleted   TINYINT NOT NULL, \
		UserName   VARCHAR(256)  NOT NULL, UNIQUE(UserName),\
	     Departments   VARCHAR(256)  ,\
		IdentifyFK   VARCHAR(256)  NOT NULL \
		) DEFAULT CHARSET=utf8";
	res = instance->execute(sql);

	sql = "CREATE TABLE IF NOT EXISTS `Patient`( \
		UID VARCHAR(256)  PRIMARY KEY , UNIQUE(UID), \
		CreateDateTime DATETIME NOT NULL, \
		UpdateDateTime DATETIME , \
		IsDeleted   TINYINT NOT NULL, \
        PatientName   VARCHAR(256)  NOT NULL, \
        IdentifyFK   VARCHAR(256)  NOT NULL, \
		CaseID   VARCHAR(256)  NOT NULL, \
		DeviceID   VARCHAR(256)  NOT NULL, \
        Gender char(1), \
		IllType	char(1), \
		IllStage char(1),\
        Birthday   DATETIME  NOT NULL, \
        Note TEXT  NOT NULL,\
		DiseaseCourse TEXT  NOT NULL, \
		Sensitivity TINYINT NOT NULL,\
		PaperSpeed TINYINT NOT NULL\
		) DEFAULT CHARSET=utf8";
	res = instance->execute(sql);

	sql = "CREATE TABLE IF NOT EXISTS `ChannelPlan`( \
		UID VARCHAR(256)  PRIMARY KEY , UNIQUE(UID), \
		CreateDateTime DATETIME NOT NULL, \
		UpdateDateTime   DATETIME NOT NULL, \
		IsDeleted   TINYINT NOT NULL, \
        PatientFK   VARCHAR(256)  NOT NULL, \
		RecordUidFK VARCHAR(256), \
        Indexs CHAR NOT NULL, \
        Position VARCHAR(256) NOT NULL, \
		Area VARCHAR(256) NOT NULL,\
		AreaNum VARCHAR(256) NOT NULL, \
        ChannelCounts INT NOT NULL,\
		DetectorType VARCHAR(256) NOT NULL,\
		SampleRate INT NOT NULL,\
		LowCut FLOAT NOT NULL,\
		HighCut FLOAT NOT NULL,\
		HwLowCut FLOAT NOT NULL, \
		HwHighCut Float NOT NULL, \
		DspCutoff Float NOT NULL, \
		Notch VARCHAR(256) NOT NULL, \
		EnableDSP TINYINT NOT NULL, \
		EnableLowCut TINYINT NOT NULL, \
		EnableHighCut TINYINT NOT NULL, \
		SignalType VARCHAR(256) NOT NULL, \
		DisabledChannels VARCHAR(4096),\
		Enabled TINYINT NOT NULL \
		) DEFAULT CHARSET=utf8";
	res = instance->execute(sql);

	sql = "CREATE TABLE IF NOT EXISTS `ChannelName`( \
		UID VARCHAR(256)  PRIMARY KEY , UNIQUE(UID), \
		CreateDateTime DATETIME NOT NULL, \
		UpdateDateTime   DATETIME NOT NULL, \
		IsDeleted   TINYINT NOT NULL, \
        PatientFK   VARCHAR(256)  NOT NULL, \
        ChannelPlanFK VARCHAR(256) NOT NULL,  \
        ChannelNumber VARCHAR(256) NOT NULL, \
		ChannelCoordX INT NOT NULL,\
		ChannelCoordY INT NOT NULL, \
        Indexs VARCHAR(256) NOT NULL, \
		CustomName VARCHAR(64) NOT NULL\
) DEFAULT CHARSET=utf8";


	res = instance->execute(sql);

	sql = "CREATE TABLE IF NOT EXISTS `TreatmentCfg`( \
		UID VARCHAR(256)  PRIMARY KEY , UNIQUE(UID), \
		CreateDateTime DATETIME NOT NULL, \
		UpdateDateTime   DATETIME NOT NULL, \
		IsDeleted   TINYINT NOT NULL, \
        PatientFK   VARCHAR(256)  NOT NULL, \
        IsChecked TINYINT NOT NULL,  \
        TrainType TINYINT NOT NULL, \
		TrialTime TINYINT NOT NULL,\
		TrialCounts TINYINT NOT NULL, \
		CountDown TINYINT NOT NULL,\
        MinScore TINYINT NOT NULL,\
        MaxScore TINYINT NOT NULL,\
        Difficult TINYINT NOT NULL,\
        MaxDifficult TINYINT NOT NULL\
		) DEFAULT CHARSET=utf8";
	res = instance->execute(sql);

	sql = "CREATE TABLE IF NOT EXISTS `StandardTreatment`( \
	   UID VARCHAR(256)  PRIMARY KEY , UNIQUE(UID), \
		CreateDateTime DATETIME NOT NULL, \
		UpdateDateTime   DATETIME NOT NULL, \
		IsDeleted   TINYINT NOT NULL, \
        PatientFK   VARCHAR(256)  NOT NULL, \
        Staging  TINYINT NOT NULL,  \
        DiseaseName  TINYINT  NOT NULL,  \
        Action  VARCHAR(256),  \
        IsChecked VARCHAR(256),  \
        TrainType TINYINT NOT NULL, \
		TrialTime TINYINT NOT NULL,\
		TrialCounts TINYINT NOT NULL, \
		CountDown TINYINT NOT NULL,\
        MinScore TINYINT NOT NULL,\
        MaxScore TINYINT NOT NULL,\
        Difficult TINYINT NOT NULL,\
        MaxDifficult TINYINT NOT NULL\
		) DEFAULT CHARSET=utf8";
	res = instance->execute(sql);

	sql = "CREATE TABLE IF NOT EXISTS `TreatmentRecord`( \
		UID VARCHAR(256)  PRIMARY KEY , UNIQUE(UID), \
		CreateDateTime DATETIME NOT NULL, \
		UpdateDateTime   DATETIME NOT NULL, \
		IsDeleted   TINYINT NOT NULL, \
        PatientFK   VARCHAR(256)  NOT NULL, \
        Score TINYINT NOT NULL,  \
        TrainDataPath VARCHAR(1024) NOT NULL, \
		StartDatetime DATETIME NOT NULL,\
		TrainTimeLen INT NOT NULL, \
		TrainType TINYINT NOT NULL,\
        IsSuccess TINYINT NOT NULL,\
        FailTimes TINYINT NOT NULL,\
        SuccessTimes TINYINT NOT NULL,\
        Difficult TINYINT\
		) DEFAULT CHARSET=utf8";
	res = instance->execute(sql);

	sql = "CREATE TABLE IF NOT EXISTS `TrainEvents`( \
		UID VARCHAR(256)  PRIMARY KEY , UNIQUE(UID), \
		CreateDateTime DATETIME NOT NULL, \
		UpdateDateTime   DATETIME NOT NULL, \
		IsDeleted   TINYINT NOT NULL, \
        TreatmentRecordFK VARCHAR(256) NOT NULL, \
        TrainType TINYINT NOT NULL,  \
        EventType TINYINT NOT NULL, \
		EventDateTime DATETIME NOT NULL,\
		IsSuccess  TINYINT \
		) DEFAULT CHARSET=utf8";
	res = instance->execute(sql);


	//sql = "INSERT INTO `optimus`.`patient` (`UID`, `CreateDateTime`,\
	//	`UpdateDateTime`, `IsDeleted`, `PatientName`, `IdentifyFK`, \
	//	`CaseID`, `DeviceID`, `Gender`, `Birthday`, `Note`, `DiseaseCourse`, \
	//	`Sensitivity`, `PaperSpeed`) \
	//	VALUES ('123', '2023-09-07 16:47:55', '2023-09-08 16:47:58', 0, \
	//	'zhangsan', '123', '123', '0', '0', '2023-09-23 16:50:01', 'XXXXXXXXX', 'Demo' , 9, 3 );";

	//res = instance->execute(sql);

	sql = "INSERT INTO `optimus`.`user` (`UID`, `CreateDateTime`, `UpdateDateTime`, `IsDeleted`, `UserName`, `IdentifyFK`) VALUES ('123', '2023-09-08 16:47:17', '2023-09-15 16:47:19', 0, 'Administrator', '123');";
	res = instance->execute(sql);
	sql = "INSERT INTO `optimus`.`identityinfo` (`UID`, `CreateDateTime`, `UpdateDateTime`, `IsDeleted`, `Password`, `Avatar`, `Icon`, `Feature`, `LoginCounts`, `InitialLogin`, `Role`,`LogInName`) VALUES('123', '2023-09-08 16:47:17', '2023-09-15 16:47:19', 0, 'admin123', '', '', '', 0, 0, 1,'Sysuser');";
	res = instance->execute(sql);
	int num = 0;
	for (int i = 0; i <5; i++)
	{
		switch (i)
		{case 0: 
			for (int j=0; j < 7; j++)
			{
				for (int k = 0; k < 5; k++)
				{
					QString insertSql = "INSERT INTO `optimus`.`standardtreatment` (`UID`, `CreateDateTime`, `UpdateDateTime`, `IsDeleted`, `IsChecked`, `DiseaseName`, `Staging`, `Action`, `TrainType`, `TrialTime`, `TrialCounts`, `CountDown`, `MinScore`, `MaxScore`, `Difficult`, `MaxDifficult`,`PatientFK`) VALUES ('%1', '2023-09-07 16:47:55', '2023-09-08 16:47:58', 0, 0, %2, %3, '1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18', %4, 10,10, 5, 60,100,60,100 ,'123');";
					insertSql=insertSql.arg(QString::number(num), QString::number(i), QString::number(j), QString::number(k));
					res = instance->execute(insertSql.toStdString());
					num++;
				}
				
			}
			break;

		case 1:
			for (int j = 0; j < 4; j++)
			{
				for (int k = 0; k < 5; k++)
				{
					QString insertsql = "INSERT INTO `optimus`.`standardtreatment` (`UID`, `CreateDateTime`, `UpdateDateTime`, `IsDeleted`, `IsChecked`, `DiseaseName`, `Staging`, `Action`, `TrainType`, `TrialTime`, `TrialCounts`, `CountDown`, `MinScore`, `MaxScore`, `Difficult`, `MaxDifficult`,`PatientFK`) VALUES ('%1', '2023-09-07 16:47:55', '2023-09-08 16:47:58', 0, 0, %2, %3, '1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18', %4, 10,10, 5, 60,100,60,100 ,'123');";
					insertsql = insertsql.arg(QString::number(num), QString::number(i), QString::number(j), QString::number(k));
					res = instance->execute(insertsql.toStdString());
					num++;

				}

			}
			break;
		case 2:
			for (int j = 0; j < 3; j++)
			{
				for (int k = 0; k < 5; k++)
				{
					QString insertsql = "INSERT INTO `optimus`.`standardtreatment` (`UID`, `CreateDateTime`, `UpdateDateTime`, `IsDeleted`, `IsChecked`, `DiseaseName`, `Staging`, `Action`, `TrainType`, `TrialTime`, `TrialCounts`, `CountDown`, `MinScore`, `MaxScore`, `Difficult`, `MaxDifficult`,`PatientFK`) VALUES ('%1', '2023-09-07 16:47:55', '2023-09-08 16:47:58', 0, 0, %2, %3, '1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18', %4, 10,10, 5, 60,100,60,100 ,'123');";
					insertsql = insertsql.arg(QString::number(num), QString::number(i), QString::number(j), QString::number(k));
					res = instance->execute(insertsql.toStdString());
					num++;

				}

			}
			break;
		case 3:
			for (int j = 0; j < 4; j++)
			{
				for (int k = 0; k < 5; k++)
				{
					QString insertsql = "INSERT INTO `optimus`.`standardtreatment` (`UID`, `CreateDateTime`, `UpdateDateTime`, `IsDeleted`, `IsChecked`, `DiseaseName`, `Staging`, `Action`, `TrainType`, `TrialTime`, `TrialCounts`, `CountDown`, `MinScore`, `MaxScore`, `Difficult`, `MaxDifficult`,`PatientFK`) VALUES ('%1', '2023-09-07 16:47:55', '2023-09-08 16:47:58', 0, 0, %2, %3, '1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18', %4, 10,10, 5, 60,100,60,100 ,'123');";
					insertsql = insertsql.arg(QString::number(num), QString::number(i), QString::number(j), QString::number(k));
					res = instance->execute(insertsql.toStdString());
					num++;

				}

			}
			break;
		case 4:
			for (int j = 0; j < 7; j++)
			{
				for (int k = 0; k < 5; k++)
				{
					QString insertsql = "INSERT INTO `optimus`.`standardtreatment` (`UID`, `CreateDateTime`, `UpdateDateTime`, `IsDeleted`, `IsChecked`, `DiseaseName`, `Staging`, `Action`, `TrainType`, `TrialTime`, `TrialCounts`, `CountDown`, `MinScore`, `MaxScore`, `Difficult`, `MaxDifficult`,`PatientFK`) VALUES ('%1', '2023-09-07 16:47:55', '2023-09-08 16:47:58', 0, 0, %2, %3, '1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18', %4, 10,10, 5, 60,100,60,100 ,'123');";
					insertsql = insertsql.arg(QString::number(num), QString::number(i), QString::number(j), QString::number(k));
					res = instance->execute(insertsql.toStdString());
					num++;

				}

			}
			break;
		case 5:
			for (int j = 0; j < 7; j++)
			{
				for (int k = 0; k < 5; k++)
				{
					QString insertsql = "INSERT INTO `optimus`.`standardtreatment` (`UID`, `CreateDateTime`, `UpdateDateTime`, `IsDeleted`, `IsChecked`, `DiseaseName`, `Staging`, `Action`, `TrainType`, `TrialTime`, `TrialCounts`, `CountDown`, `MinScore`, `MaxScore`, `Difficult`, `MaxDifficult`,`PatientFK`) VALUES ('%1', '2023-09-07 16:47:55', '2023-09-08 16:47:58', 0, 0, %2, %3, '1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18', %4, 10,10, 5, 60,100,60,100 ,'123');";
					insertsql = insertsql.arg(QString::number(num), QString::number(i), QString::number(j), QString::number(k));
					res = instance->execute(insertsql.toStdString());
					num++;

				}

			}
			break;
		default:
			break;
		}
	}
	

	LOG_INFO("initDB........................finish");
}

END_NX_NAMESPACE
