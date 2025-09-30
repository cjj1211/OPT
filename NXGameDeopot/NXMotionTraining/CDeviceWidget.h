/******************************************************************************
 *  版权所有（C）2022-2024，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *  创建日期:    2024
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __CDEVICE_WIDGET_H
#define  __CDEVICE_WIDGET_H

#include "Global.h"
#include "ui_CDeviceWidget.h"

BEGIN_NX_NAMESPACE
class CDeviceStatus;

struct DeviceStatus;

class CInformationProcessing;
class CDeviceWidget : public QWidget
{
	Q_OBJECT

public:
	CDeviceWidget(CInformationProcessing* informationProcessing,QWidget *parent = nullptr);
	~CDeviceWidget();

	//设置设备状态
	 void setDeviceStatus(DeviceStatus deviceStatus);
private:
	Ui::CDeviceWidgetClass ui;
	CDeviceStatus* deviceStatus;
};
END_NX_NAMESPACE
#endif
