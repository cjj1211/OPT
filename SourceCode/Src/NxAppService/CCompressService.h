/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_OSCILLOGRAPH_SERVICE_H
#define __C_OSCILLOGRAPH_SERVICE_H
#include "nxappservice_global.h"
#include "IAppService.h"

#include <QObject>
#include <QTime>
#include <QMap>
#include <QFuture>
#include <memory>
#include <functional>
#include <vector>

BEGIN_NX_NAMESPACE


//class CCompressThread;

class NX_APP_SERVICE_EXPORT CCompressService : public IAppService
{
    Q_OBJECT
public:
    CCompressService( QObject* parent = nullptr);
    virtual ~CCompressService() override;
    void exportToDir(const QString& outDir, const QString& inputDir);
    void importToDir(const QString& outDir, const QString& inputDir);
signals:
    
    void signalCompressProgress(QString fileName, int currentIndex, int allIndex);
    void signalExtractProgress(QString fileName, int currentIndex, int allIndex);
protected:
   
    //CCompressThread* compressThread;
   

};

END_NX_NAMESPACE

#endif