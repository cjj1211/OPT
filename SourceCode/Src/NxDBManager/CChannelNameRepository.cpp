/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : junjie.cui@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/


#include "CChannelNameRepository.h"
#include"../NxEntity/ChannelName.h"
BEGIN_NX_NAMESPACE
using namespace ormpp;
CChannelNameRepository::CChannelNameRepository()
    :IRepository()
{
}

CChannelNameRepository::~CChannelNameRepository()
{
}

QPoint CChannelNameRepository::getChannelPositionByUid(const QString& channelPlanUid, const QString& customName)
{
    auto channelCustomName = IRepository::db->query<ChannelName>(" ChannelPlanFK = '" + channelPlanUid.toStdString() + "'" +
        " and CustomName = '" + customName.toStdString() + "'");
    if (channelCustomName.empty()) {
        QString sql = " Indexs = '" + customName.left(0) + "'" +
            " and  ChannelNumber = '" + customName.mid(2, customName.length()) + "'";
        auto channelNativeName = IRepository::db->query<ChannelName>(sql.toStdString());
        if (channelNativeName.empty()) {

            return QPoint(-1, -1);
        }
        else {
            if (channelNativeName[0].IsDeleted == 0)
            {
                return QPoint(channelNativeName[0].ChannelCoordX, channelNativeName[0].ChannelCoordY);
            }
            else
            {
                return QPoint(-1, -1);
            }
        }

    }
    else {
        if (channelCustomName[0].IsDeleted == 0)
        {
            return QPoint(channelCustomName[0].ChannelCoordX, channelCustomName[0].ChannelCoordY);
        }
        else
        {
            return QPoint(-1, -1);
        }


    }
}



QPair<int, int> CChannelNameRepository::getChannelPositon(const QString& fk,const QString& customName)
{
 
    auto channelCustomName  = IRepository::db->query<ChannelName>(" PatientFK = '" + fk.toStdString() + "'" +
        " and IsDeleted = 0"+" and customname =  '" + customName.toStdString() + "'");
    if (channelCustomName.empty()){

        return QPair(-1, -1);
       /* QString sql = " Indexs = '" + customName.left(1) + "'" +
            " and  ChannelNumber = '" + customName.mid(2, customName.length()) + "'";
        auto channelNativeName= IRepository::db->query<ChannelName>(sql.toStdString());
        if (channelNativeName.empty()){
            
            return QPair(-1, -1);
        }
        else {
            if (channelNativeName[0].IsDeleted==0)
            {
                return QPair(channelNativeName[0].ChannelCoordX, channelNativeName[0].ChannelCoordY);
            }
            else
            {
                return QPair(-1, -1);
            }
        }*/

    } else{
        if (channelCustomName[0].IsDeleted == 0)
        {
            return QPair(channelCustomName[0].ChannelCoordX, channelCustomName[0].ChannelCoordY);
        }
        else
        {
            return QPair(-1, -1);
        }
       
           
    }
  
}

void CChannelNameRepository::deleteChannelBychannelFK(const QString& fk)
{
  
   /* auto channelname = IRepository::db->query<ChannelName>(" ChannelPlanFK = '" + fk.toStdString() + "'" + 
        " and IsDeleted = 0");*/
    IRepository::db->delete_records<ChannelName>(" ChannelPlanFK = '" + fk.toStdString() + "'");
    
}
std::vector<ChannelName> CChannelNameRepository::getChannelNameByChannelPlanFK(const QString& fk)
{

    auto channelname= IRepository::db->query<ChannelName>(" ChannelPlanFK = '" + fk.toStdString() + "'" + 
        " and IsDeleted = 0");

    return channelname;

}
ChannelName  CChannelNameRepository::getChannelBychannelPosition(const QString& channelPlanUid, const int x, const int y, const QString& index)
{

    auto channelname = IRepository::db->query<ChannelName>(" ChannelPlanFK = '" + channelPlanUid.toStdString() + "'" +
        " and ChannelCoordX = " + QString::number(x).toStdString() + 
        " and ChannelCoordY = " + QString::number(y).toStdString() + 
        " and Indexs =  '" + index.toStdString() + "'" +
        " and IsDeleted = 0");

    if (channelname.empty())
    {
        return ChannelName();
    }
    else
    {
        return channelname[0];
    }
}
ChannelName CChannelNameRepository::getChannelBychannelPosition(const int x, const int y, const QString& index, const QString& fk)
{
    auto channelname = IRepository::db->query<ChannelName>(" where ChannelCoordX = " + QString::number(x).toStdString() + 
        " and ChannelCoordY = " + QString::number(y).toStdString() + 
        " and Indexs =  '" + index.toStdString() +"'" + 
        " and PatientFK = '" + fk.toStdString() + "'" +
        " and IsDeleted = 0 "
        " order by UpdateDateTime DESC ");

    if (channelname.empty())
    {
        return ChannelName();
    }
    else
    {
        return channelname[0];
    }
}
void CChannelNameRepository::updateChannelName(ChannelName channelname)
{
    IRepository::db->update(channelname);
}
void CChannelNameRepository::updateChannelName(std::vector<ChannelName> channelname)
{

    IRepository::db->update(channelname);

}

END_NX_NAMESPACE


