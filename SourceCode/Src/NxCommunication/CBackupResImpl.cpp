#include "CBackupResImpl.h"

#include <CLogger.h>
BEGIN_NX_NAMESPACE
using namespace  NX;
 CBackupResClient ::CBackupResClientImpl::CBackupResClientImpl(std::shared_ptr<Channel> channel)
	 :stub(Recorder::NewStub(channel))
{
	
}

 void CBackupResClient::CBackupResClientImpl::setRestore(QVector<QString> restorePath)
 {
	 RestoreMsg request;
	 for (int i =0;i< restorePath.size();i++)
	 {
		 request.set_restorepath(i, restorePath.at(i).toStdString()); 

	 }
	 EmptyMsg response;
	 ClientContext context;
	 if (const auto status = stub->startRestore(&context, request, &response); !status.ok())
	 {
		 LOG_INFO(std::format("Send msg fail. Error code: {}, \nError msg: {}, \nError details: {}",
			 static_cast<int>(status.error_code()), status.error_message(), status.error_details()));
	 }
}
 END_NX_NAMESPACE







