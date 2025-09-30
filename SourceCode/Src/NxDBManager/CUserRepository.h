/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_USER_REPOSITORY_H
#define __C_USER_REPOSITORY_H

#include "nxdbmanager_global.h"
#include "IRepository.h"

BEGIN_NX_NAMESPACE

class NXDBMANAGER_EXPORT CUserRepository: public IRepository<User>
{
public:
    CUserRepository();
    virtual ~CUserRepository() override;
    std::vector<User> getUserByLoginName(QString username);
    std::vector<User> getUserByIdentifyFK(const QString& IdentifyFK);
    std::vector<User> getAllUser();
};

END_NX_NAMESPACE

#endif
