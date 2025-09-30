#ifndef __C_ELECTRODEPREVIEW
#define __C_ELECTRODEPREVIEW
/******************************************************************************
*  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
*  作者 : <崔俊杰><junjie.cui@neuroxess.com>
*  修改记录:
*  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#include "nxelectrodemap_global.h"
#include <QWidget>
#include <memory>
#include <QVector>
#include <QMap>
#include "CElectrodeMapDataStructure.h"
#include "PageView.h"
class QTabWidget;
class Ui_CElectrodePreview;
BEGIN_NX_NAMESPACE
class CElectrodeSettingInterface;
class PageView;
class  NXELECTRODEMAP_EXPORT CElectrodePreview : public QWidget
{
	Q_OBJECT
public:
	explicit CElectrodePreview(QWidget* parent = nullptr);
	~CElectrodePreview() override;
 /**
     @fn     populateTabWidget
     @brief  
     @param   - 
     @author CuiJunJie
     @date   18.10.2023
 **/
	void populateTabWidget(const  QPair <QString, QString>&);
	bool searchChannelMap(const QString& channnelIndex, QPair<int, int>position);
	bool searchChannelMap(const QString& channnelIndex, const QString& customname);
	/**
	  @fn     setButtonStyel
	  @brief  设置通道颜色 ,通道用户自定义名字，颜色
	  @param  buttonStyel - button styel
	  @author CuiJunJie
	  @date   18.10.2023
  **/
	void setButtonStyle(QMap<QString, QColor> buttonStyle);
	void setImpedanceValue(QMap<QString, QString> ImpedanceValue);
	int getCurrentTabIndex(); 

public:
	QList<QPair <QString, int>>channelMapList;
	int channelType = 1;
	QVector<Group> allChannel;  //包含登录系统的角色的所有通道信息，通道名称需要通过数据库去查找 
	void setCurrentTab(int currentTab);
	QString currentButtonName;
signals:
	void signalShowCurrentChannelName(int currentButtonX, int currentButtonY, QString channelNumber,QString currentTabIndex);
	void signalUpdateCurrentChannelName(int currentButtonX, int currentButtonY, QString currentTabIndex, int channelnumber);
public slots:
	void slotLoad(const QString& filename);
	void slotPageChanged(int index);
private:
	void clearTabWidget() const; // Delete PageViews and clear the tab widget
private:
	std::unique_ptr<Ui_CElectrodePreview>			ui;
	ElectrodeMapSettings* electrodeMapSetting;
	ElectrodeMapSettings* currerntElectrodeMapSetting;
	CElectrodeSettingInterface* probeMapSettingsInterface;
	QTabWidget* pageTabWidget;
	PageView* pageView;
	QString											statusCoords;
	QString											statusSiteInfo;
	QString											xmlFilePath;
	QMap<QString, QColor>                           allButtonStyle;
	QMap<QString, QString> impedanceValue;
	int currentPage;
};
END_NX_NAMESPACE
#endif