/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/


#ifndef __C_DB_MANAGER_H
#define __C_DB_MANAGER_H

#include "nxdbmanager_global.h"

#include <ormpp/utility.hpp>
#include <ormpp/mysql.hpp>
#include <ormpp/dbng.hpp>
#include <memory>
#include <mysql.h>
BEGIN_NX_NAMESPACE
using namespace ormpp;


class NXDBMANAGER_EXPORT CDBManager
{
public:
	static std::shared_ptr<dbng<mysql>> getDbManager();
	static MYSQL getMysql();
	static void initDB(std::string Ip, std::string User, std::string Password, std::string Name, int Port);

};

END_NX_NAMESPACE
#endif