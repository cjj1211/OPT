#include "CMasterServer.h"
#include "CMasterServerImpl.h"
#include "Master.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <string>

BEGIN_NX_NAMESPACE
CMasterServer::CMasterServer(const std::string& address)
	: impl(new CMasterServerImpl())
	, builder(new ServerBuilder())
	, server(builder->AddListeningPort(address
		, grpc::InsecureServerCredentials()).RegisterService(impl).BuildAndStart())
{

}

CMasterServer::~CMasterServer()
{
	if (server)  server->Shutdown();
	delete impl;
}

void CMasterServer::setPlaybackStopedFunc(const std::function<void()>& func) const
{
	impl->setPlaybackStopedFunc(func);
}

END_NX_NAMESPACE
