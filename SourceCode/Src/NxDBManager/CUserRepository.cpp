/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/

#include "CUserRepository.h"

BEGIN_NX_NAMESPACE

CUserRepository::CUserRepository()
    :IRepository()
{
}

CUserRepository::~CUserRepository()
{
}

std::vector<User> CUserRepository::getUserByLoginName(QString loginname)
{
	auto user = IRepository::db->query<User>(" LogInName ='" + loginname.toStdString()+"'" + " and IsDeleted = 0");
	if (user.empty())
	{
		return std::vector<User>();
	}
	else
	{
		return user ;
	}
}
std::vector<User>  CUserRepository::getUserByIdentifyFK(const QString& IdentifyFK)
{
	auto user = IRepository::db->query<User>(" IdentifyFK ='" + IdentifyFK.toStdString() + "'" + " and IsDeleted = 0");
	if (user.empty())
	{
		return std::vector<User>();
	}
	else
	{
		return user;
	}
}
std::vector<User> CUserRepository::getAllUser()
{
	auto user = IRepository::db->query<User>( " where IsDeleted = 0 order by UpdateDateTime  desc");
	if (user.empty())
	{
		return std::vector<User>();
	}
	else
	{
		return user;
	}

}

END_NX_NAMESPACE


