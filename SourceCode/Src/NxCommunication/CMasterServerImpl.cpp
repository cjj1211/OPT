#include "CMasterServerImpl.h"

#include <QDebug>
#include <CLogger.h>

BEGIN_NX_NAMESPACE

CMasterServer::CMasterServerImpl::CMasterServerImpl()
	: Service()
{

}

CMasterServer::CMasterServerImpl::~CMasterServerImpl()
{
}

Status CMasterServer::CMasterServerImpl::playbackStoped(ServerContext* context, const EmptyMsg* request,
	EmptyMsg* response)
{
	if (playbackStopedFunc)
	{
		playbackStopedFunc();
	}
	return Status::OK;
}


END_NX_NAMESPACE