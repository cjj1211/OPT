#include "CBackupResClient.h"

#include <grpcpp/create_channel.h>

#include "CBackupResImpl.h"

BEGIN_NX_NAMESPACE
CBackupResClient::CBackupResClient(const std::string & address)
	: impl(new CBackupResClientImpl(grpc::CreateChannel(address, grpc::InsecureChannelCredentials())))
{}

CBackupResClient::~CBackupResClient()
{}

void CBackupResClient::setRestore(QVector<QString> restorePath)
{

	impl->setRestore(restorePath);
}

END_NX_NAMESPACE
