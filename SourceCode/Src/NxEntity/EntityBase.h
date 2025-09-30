/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef ENTITY_BASE_H
#define ENTITY_BASE_H

#include "nxentity_global.h"
#include "BasicTypes.h"
#include <QUUid>
#include <QDateTime>

BEGIN_NX_NAMESPACE

using NX_DB_UUID = std::string ;

struct EntityBase {
	EntityBase() {
		UID = QUuid::createUuid().toString().replace("{", "").replace("}", "").toStdString();
		CreateDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString();
		UpdateDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString();
		IsDeleted = false;
	}

	QDateTime getUpdateDatetime() const
    {
		return QDateTime::fromString(QString::fromStdString(UpdateDateTime), "yyyy-MM-dd hh:mm:ss");

	}

	QDate getUpdateDate() const
    {
		return getUpdateDatetime().date();
	}

	QDateTime getCreateDateTime() const
    {
		return QDateTime::fromString(QString::fromStdString(CreateDateTime), "yyyy-MM-dd hh:mm:ss");
	}

	QDate getCreateDate() const
    {
		return getCreateDateTime().date();
	}

	QString getQUid() const
    {
		return QString::fromStdString(UID);
	}

	bool getIsDeleted() const
    {
		return IsDeleted;
	}

	std::string UID;
	std::string CreateDateTime;
	std::string UpdateDateTime;
	char IsDeleted;
};

END_NX_NAMESPACE
#endif
