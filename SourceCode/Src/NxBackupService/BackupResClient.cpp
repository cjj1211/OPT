#include "BackupResClient.h"
#include "BackupResImpl.h"
BackupResClient::BackupResClient(const std::string & address)
	: impl(new BackupResServiceImpl(grpc::CreateChannel(address, grpc::InsecureChannelCredentials())))
{}

BackupResClient::~BackupResClient()
{}

void BackupResClient::setBackup( const QString & backupdate)
{
	impl->setBackup(backupdate);
}

