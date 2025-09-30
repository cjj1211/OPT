#include "CCommandParser.h"
#include "CInformationProcessing.h"
BEGIN_NX_NAMESPACE
CCommandParser::CCommandParser(CInformationProcessing* informationProcessing,QObject *parent)
	: QObject(parent)
{

	connect(informationProcessing, &CInformationProcessing::receiveCommandMessage, this, &CCommandParser::receiveMessage);

}

CCommandParser::~CCommandParser()
{
	
}

void CCommandParser::receiveMessage(ActionCommand command)
{
	emit sendActionCommand(command);
}
END_NX_NAMESPACE
