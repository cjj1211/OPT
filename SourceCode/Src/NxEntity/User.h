/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __USER_H
#define __USER_H

#include "nxentity_global.h"
#include "EntityBase.h"
#include <string>
#include <iguana/reflection.hpp>
#include <ormpp/dbng.hpp>

BEGIN_NX_NAMESPACE

struct User : public EntityBase {
	std::string UserName;
	std::string IdentifyFK;
	std::string Departments;

};

REFLECTION(User, UID, CreateDateTime, UpdateDateTime, IsDeleted, UserName, IdentifyFK, Departments);

END_NX_NAMESPACE

#endif
