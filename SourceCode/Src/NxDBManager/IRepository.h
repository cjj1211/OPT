/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
 * <崔俊杰><junjir.cui@neuroxess.com><增加查询条件 isdeleted=0>
******************************************************************************/
#ifndef __I_REPOSITORY_H
#define __I_REPOSITORY_H

#include "nxdbmanager_global.h"
#include "CDBManager.h"
#include "../NxEntity/EntityBase.h"
#include "../NxEntity/User.h"

#include <ormpp/mysql.hpp>
#include <ormpp/dbng.hpp>
#include <iguana/reflection.hpp>
#include <QString>

using namespace ormpp;

BEGIN_NX_NAMESPACE

template<typename T>
class Q_DECL_EXPORT IRepository
{
public:
    IRepository();
    virtual ~IRepository();

    bool Insert(const T& data);

    bool Delete(T& data);

    bool DeleteByUID(QString UID);

    bool ForceDelete(T& data);

    bool ForceDeleteByUID(QString UID);
    
    bool Update(const T& data);

    bool GetByUID(QString UID, T& data) ;

    std::vector<T> GetAll() const;

protected:
    static std::shared_ptr<dbng<mysql>> db;
};

template<typename T>
std::shared_ptr<dbng<mysql>> IRepository<T>::db = CDBManager::getDbManager();

template<typename T>
inline IRepository<T>::IRepository()
{
}

template<typename T>
inline IRepository<T>::~IRepository()
{
}

template<typename T>
inline bool IRepository<T>::Insert(const T& data)
{
    if (!(db->insert(data)==1))
    {
        LOG_ERR("Insert data fail, data is rollbacked.");
        return false;
    }
  
    return true;
}

template<typename T>
inline bool IRepository<T>::Delete(T& data)
{
    auto base = dynamic_cast<EntityBase*>(&data);
    std::vector<T> vec = db->query<T>(std::format("UID='{}'", base->UID));
    if (vec.size() == 0)
    {
        LOG_ERR(std::format("Cannot find entity, entity name: {}, UID: {}", iguana::get_name<T>(), base->UID));
        return false;
    }

    vec[0].IsDeleted = true;
    db->begin();
    if (!db->update<T>(vec[0]))
    {
        db->rollback();
        LOG_ERR("Insert data fail, data is rollbacked.");
        return false;
    }
    db->commit();
    
    return true;
}

template<typename T>
inline bool IRepository<T>::DeleteByUID(QString UID)
{
    std::vector<T> vec = db->query<T>(std::format("UID='{}'",UID.toStdString()));
    if (vec.size() == 0)
    {
        LOG_ERR(std::format("Cannot find entity, entity name: {}, UID: {}", iguana::get_name<T>(), UID.toStdString()));
        return false;
    }

    vec[0].IsDeleted = true;
    db->begin();
    if (!db->update<T>(vec[0]))
    {
        db->rollback();
        LOG_ERR(std::format("Insert {} data fail, data is rollbacked.", iguana::get_name<T>()));
        return false;
    }
    db->commit();
    return true;
}

template<typename T>
inline bool IRepository<T>::Update(const T& data)
{
    db->begin();
    if (!(db->update(data)==1))
    {
        db->rollback();
        LOG_ERR(std::format("Update entity fail, entity name: {}", iguana::get_name<T>()));
        return false;
    }
    db->commit();
    return true;
}

template<typename T>
inline bool IRepository<T>::GetByUID(QString UID, T& data)
{
    auto where = ("UID='"+ UID.toStdString()+"'");
    std::vector<T> vec = db->query<T>(where+ " and IsDeleted = 0");
    if (vec.size() == 0)
    {
        std::string msg = std::format("Get {} fail. Entity Uid: {}", iguana::get_name<T>(), UID.toStdString());
        LOG_ERR(msg);
        return false;
    }
    else
    {
        data = vec[0];
        return true;
    }
}

template<typename T>
inline std::vector<T> IRepository<T>::GetAll() const
{
    return db->query<T>("IsDeleted =0");
}




END_NX_NAMESPACE

#endif

