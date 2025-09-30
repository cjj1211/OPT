#include "BackupResImpl.h"

#include <CLogger.h>
; using namespace  NX;
 BackupResClient ::BackupResServiceImpl::BackupResServiceImpl(std::shared_ptr<Channel> channel)
	 :stub(Recorder::NewStub(channel))
{
	
}

 void BackupResClient::BackupResServiceImpl::setBackup(const QString& backupdate)
 {
	 BackupRestoreMsg request;

	 request.set_startdatetime(backupdate.toStdString());
	 EmptyMsg response;
	 ClientContext context;
	 if (const auto status = stub->startBackup(&context, request, &response); !status.ok())
	 {
		 LOG_INFO(std::format("Send msg fail. Error code: {}, \nError msg: {}, \nError details: {}",
			 static_cast<int>(status.error_code()), status.error_message(), status.error_details()));
		
		 return;
	 }
 }





