/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   November 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_MASTER_SERVER_H
#define __C_MASTER_SERVER_H
#include "nxcommunication_global.h"
#include <functional>

namespace grpc
{
	class ServerBuilder;
	class Server;
}

BEGIN_NX_NAMESPACE

class NXCOMMUNICATION_EXPORT CMasterServer
{
public:
	CMasterServer(const std::string& address);

	~CMasterServer();

	void setPlaybackStopedFunc(const std::function<void()>& func) const;
private:
	class CMasterServerImpl;
	CMasterServerImpl* impl;
	grpc::ServerBuilder* builder;
	std::unique_ptr<grpc::Server>           server;
};

END_NX_NAMESPACE
#endif