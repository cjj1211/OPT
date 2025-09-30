/******************************************************************************
 *  版权所有（C）2022-2024，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *  创建日期:    2024
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __MESSAGE_BASE_H
#define __MESSAGE_BASE_H

#include <QDateTime>

BEGIN_NX_NAMESPACE
struct MessageBase {
public:
	MessageBase() {
	
		CurrentDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString();
	}

	 QDateTime getCurrentDatetime() {
		return QDateTime::fromString(QString::fromStdString(CurrentDateTime), "yyyy-MM-dd hh:mm:ss");

	}

	QDate getCurrentDate() {
		return getCurrentDatetime().date();
	}
	std::string CurrentDateTime;

};

END_NX_NAMESPACE
#endif
