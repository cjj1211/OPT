/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
 * <崔俊杰><junjie.cui@@neuroxess.com> <将实体名改成IdentityInfo>
******************************************************************************/
#ifndef __C_IDENTIFY_INFO_REPOSITORY_H
#define __C_IDENTIFY_INFO_REPOSITORY_H
#include "nxdbmanager_global.h"
#include "IRepository.h"
#include "../NxEntity/IdentityInfo.h"

BEGIN_NX_NAMESPACE

class NXDBMANAGER_EXPORT CIdentifyInfoRepository: public IRepository<IdentityInfo>
{
public:
    CIdentifyInfoRepository();
    ~CIdentifyInfoRepository() override;
    IdentityInfo getIdentityInfoByUid(const QString& uid);
    std::vector<IdentityInfo> getIdInfoByLoginName( const QString& loginName);

    std::vector<IdentityInfo> getAllIdInfos();
};

END_NX_NAMESPACE
#endif
