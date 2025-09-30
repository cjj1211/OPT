#ifndef __CBackupResClient_H
#define __CBackupResClient_H
#include "nxcommunication_global.h"
#include "CBackupResClient.h"
#include"Recorder.grpc.pb.h"

#include "NxMessage.pb.h"

#include <grpcpp/grpcpp.h>
#include <QString>
#include <QVector>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using NX::Recorder;
using NX::RestoreMsg;

using NX::EmptyMsg;
BEGIN_NX_NAMESPACE

class CBackupResClient::CBackupResClientImpl
{
 
public :
    explicit CBackupResClientImpl(std::shared_ptr<Channel> channel);

    ~CBackupResClientImpl() = default;
    void setRestore(QVector<QString>  restorePath);
  
private:
    std::unique_ptr<Recorder::Stub>							stub;
};
END_NX_NAMESPACE
#endif
