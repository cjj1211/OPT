/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __IDENTIFY_H
#define __IDENTIFY_H

#include "EntityBase.h"

BEGIN_NX_NAMESPACE

struct IdentityInfo :public EntityBase {
    char Role;
    std::string Password;
    std::string LoginName;
    std::vector<char> Avatar;
    std::vector<char> Icon;
    std::vector<char> Feature;
    char LoginCounts{0};
    char InitialLogin{1} ; // 0: 超级管理员

    RoleType getRoleType() const { return static_cast<RoleType>(Role); }
    void setRole(const RoleType role) { Role = static_cast<char>(role); }

    bool isSuperAdmin() const { return InitialLogin == 0; }
};

REFLECTION(IdentityInfo, UID, CreateDateTime, UpdateDateTime, IsDeleted, Role, Password, Avatar, Icon, Feature, LoginCounts, InitialLogin, LoginName);

END_NX_NAMESPACE

#endif
