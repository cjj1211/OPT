#pragma once
#include <grpcpp/grpcpp.h>
#include"Recorder.grpc.pb.h"
#include "Recorder.pb.h"
#include "NxMessage.grpc.pb.h"
#include  "NxMessage.pb.h"
#include "BackupResClient.h"
#include<QString>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using NX::Recorder;
using NX::BackupRestoreMsg;
using NX::EmptyMsg;
class BackupResClient::BackupResServiceImpl
{
 
public :
    explicit BackupResServiceImpl(std::shared_ptr<Channel> channel);

    ~BackupResServiceImpl() = default;
    void setBackup(const QString& backupdate);
  
private:
    std::unique_ptr<Recorder::Stub>							stub;
};

