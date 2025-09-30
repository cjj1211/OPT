#pragma once
#include "nxappservice_global.h"
#include <QObject>
BEGIN_NX_NAMESPACE
class NXAPPSERVICE_EXPORT CGameControl  : public QObject
{
	Q_OBJECT

public:
	CGameControl(QObject *parent);
	~CGameControl();
};
END_NX_NAMESPACE