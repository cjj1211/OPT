/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   November 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_MASTER_SERVER_IMPL
#define __C_MASTER_SERVER_IMPL
#include "CMasterServer.h"
#include "Master.pb.h"
#include "Master.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

BEGIN_NX_NAMESPACE

class CMasterServer::CMasterServerImpl: public Master::Service
{
public:
    CMasterServerImpl();
    ~CMasterServerImpl() override;

    Status playbackStoped(ServerContext* context, const EmptyMsg* request, EmptyMsg* response) override;

    void setPlaybackStopedFunc(const std::function<void()>& func)
    {
        playbackStopedFunc = func;
    }

private:
    std::function<void()> playbackStopedFunc;
};

END_NX_NAMESPACE
#endif