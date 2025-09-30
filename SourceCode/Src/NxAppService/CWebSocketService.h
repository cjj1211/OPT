/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 :  zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef _C_WEBSOCKET_SERVICE_H
#define _C_WEBSOCKET_SERVICE_H

#include "IAppService.h"
#include "NxEntity/Patient.h"
#include "NxEntity/StandardTreatment.h"
#include "NxEntity/TransInformation.h"
#include <QTimer>

#include "NxEntity/IdentityInfo.h"
BEGIN_NX_NAMESPACE
	class CWebSocketServer;
class CTreatmentRecordRepository;
struct TreatmentRecord;
class NX_APP_SERVICE_EXPORT CWebSocketService: public IAppService
{
    Q_OBJECT
public:
    CWebSocketService(int  port,QObject* parent = nullptr);
    ~CWebSocketService() override;
    void initGame(IdentityInfo identityInfo, Patient patient, StandardTreatment standardTreatment);
    void sendActionType(ActionType type);
public slots:
    void onMessageReceived(const QString& message);
    void sendTestAction();
private:
    TransInformation chooseAction(TransInformation transInformation,int action);
    std::vector<ActionType> chooseAction(std::vector<ActionType>actionVector, int action);
private:

    CWebSocketServer* webSocketServer;

    QTimer *testTimer;
    int  currentAction;
    IdentityInfo identity_info;
    Patient patient_;
    StandardTreatment standard_treatment_;
    std::unique_ptr<CTreatmentRecordRepository> treatmentRepository;
};

END_NX_NAMESPACE
#endif