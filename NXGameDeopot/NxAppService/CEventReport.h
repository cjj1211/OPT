#pragma once
#include "nxappservice_global.h"
#include <QObject>
BEGIN_NX_NAMESPACE
class NXAPPSERVICE_EXPORT CEventReport  : public QObject
{
	Q_OBJECT

public:
	CEventReport(QObject *parent);
	~CEventReport();
};
END_NX_NAMESPACE