#include "CChannelMapWidget.h"
#include "ui_CChannelMapWidget.h"
#include "../../NxAppService/CChannelNameService.h"
#include "../../NxAppService/CChannelPlanService.h"
#include "../../NxEntity/ChannelName.h"
#include "../../NxEntity/ChannelPlan.h"
#include "../../NxElectrodeMap/CElectrodePreview.h"
#include "../CNxMessageBox.h"
#include <QAction>
#include <QDateTime>
#include <QUuid>

BEGIN_NX_NAMESPACE

CChannelMapWidget::CChannelMapWidget(QWidget *parent)
    : QWidget(parent)
   , channelNameService(new CChannelNameService(this))
    , channelPlanService(new CChannelPlanService(this))
  
{
    ui = std::make_unique<Ui::CChannelMapWidget>();
    ui->setupUi(this);
   
    LOG_INFO("new..................electrodePreview.");
    electrodePreview = new CElectrodePreview(this);
    ui->verticalLayout_3->addWidget(electrodePreview);

    LOG_INFO("end new..................electrodePreview.");
    connect(electrodePreview, &CElectrodePreview::signalShowCurrentChannelName, this, &CChannelMapWidget::showCurrentChannelName);
    connect(electrodePreview, &CElectrodePreview::signalUpdateCurrentChannelName, this, &CChannelMapWidget::updateCurrentChannelName);

    ui->warning->setVisible(false);
    ui->lineEdit->setLayoutDirection(Qt::LeftToRight); // 设置文本方向从左到右
    auto searchbtn = new QPushButton(this);
    searchbtn->setFixedSize(20, 18);//设置固定大小
    searchbtn->setFlat(true);//去掉按钮边框
    searchbtn->setToolTip(QString::fromLocal8Bit("搜索"));//设置鼠标悬浮的提示
    searchbtn->setStyleSheet("QPushButton {background-image: url(:/Optimus/images/trainPage/search.svg);border:none;}");//设置样式表
    //水平布局
    QHBoxLayout *hlayout = new QHBoxLayout(this);
    hlayout->addStretch();
    hlayout->addWidget(searchbtn);//将按钮加到布局中
    hlayout->setContentsMargins(0, 0, 5, 0);//设置布局的边距
    //第一个编辑框添加布局（将按钮放在编辑框上）
    ui->lineEdit->setLayout(hlayout);
    searchbtn->show();
    connect(ui->goback,&QPushButton::clicked,this,&CChannelMapWidget::channelMapWidgetHide);
    connect(searchbtn, &QPushButton::clicked, this, &CChannelMapWidget::searchChannelMap);
    connect(ui->Save, &QPushButton::clicked, this, &CChannelMapWidget::saveChannelMap);
    connect(ui->lineEdit, &QLineEdit::textEdited, [=](const QString& text) {
        ui->warning->setVisible(false);
        });
}

CChannelMapWidget::~CChannelMapWidget()
{
    
}

void  CChannelMapWidget::showCurrentChannelName(int currentButtonX, int currentButtonY, QString channelNumber, QString currentTabIndex)
{
    auto intdex = channelMap.at(currentTabIndex.toInt()).first;
    currentSelectChannelName.first.first= currentButtonX;
    currentSelectChannelName.first.second = currentButtonY;
    currentSelectChannelName.second.first = channelNumber;
    currentSelectChannelName.second.second = currentTabIndex;
    auto channelMaps = channelNameService->getChannelBychannelPosition(currentButtonX, currentButtonY, intdex,patientFK);
    auto channelPlan = channelPlanService->getChannelPlanByIndexs(QString::fromStdString(channelMaps.PatientFK), intdex);
    if (channelMaps.UID != ""&& channelPlan.UID== channelMaps.ChannelPlanFK)
    {
        ui->lineEdit->setText(QString::fromStdString(channelMaps.CustomName));
    }
    else
    {
       
        ui->lineEdit->setText(intdex+ QString("-%1").arg((channelNumber).toInt(), 3, 10, QLatin1Char('0')));
    }
    ui->warning->setVisible(false);
}
void CChannelMapWidget::updateCurrentChannelName(int currentButtonX, int currentButtonY, QString currentTabIndex,int channelnumber)
{
    auto intdex = channelMap.at(currentTabIndex.toInt()).first;
    auto channelMaps = channelNameService->getChannelBychannelPosition(currentButtonX, currentButtonY, intdex,patientFK);
    if (channelMaps.UID.empty())
    {
        electrodePreview->currentButtonName = intdex + QString("-%1").arg(channelnumber, 3, 10, QLatin1Char('0'));
    }
    else { electrodePreview->currentButtonName = QString::fromStdString(channelMaps.CustomName); }
}
void CChannelMapWidget::channelMapWidgetHide()
{
    emit hideChannelMapWidget();

}
void CChannelMapWidget::saveChannelMap()
{
    auto intdex = channelMap.at(currentSelectChannelName.second.second.toInt()).first;
    auto currentchannelMap = channelNameService->getChannelBychannelPosition(currentSelectChannelName.first.first, currentSelectChannelName.first.second, intdex,patientFK);
    if (currentchannelMap.UID!="")
    {
        for (auto channelMapName : electrodePreview->allChannel)
        {
            auto thischannelMap = channelNameService->getChannelBychannelPosition(currentSelectChannelName.first.first, currentSelectChannelName.first.second,intdex,patientFK);
            for (auto channel: channelMapName.electrodeSites)
            {
                if (QString::fromStdString(currentchannelMap.Indexs) + QString("-%1").arg(QString::fromStdString(currentchannelMap.ChannelNumber).toInt(), 3, 10, QLatin1Char('0')) == ui->lineEdit->text())
                {
                   
                    break;
                }
                else
                {
                    if (QString::fromStdString(thischannelMap.Indexs) + QString("-%1").arg(QString::fromStdString(channel.channelNumber).toInt(), 3, 10, QLatin1Char('0')) == ui->lineEdit->text())
                    {
                        ui->warning->setText(QString::fromLocal8Bit("有重复名"));
                        ui->warning->setVisible(true);
                        return;
                    }
                    if (channelNameService->getChannelPositon(patientFK,ui->lineEdit->text()).first != -1)
                    {
                        ui->warning->setText(QString::fromLocal8Bit("有重复名"));
                        ui->warning->setVisible(true);
                        return;
                    }
                }
               
            }

        }
       
  
        currentchannelMap.CustomName = ui->lineEdit->text().toStdString();
        QDateTime currentDateTime = QDateTime::currentDateTime();

        // 将日期时间格式化为 "yyyy-MM-dd hh:mm:ss" 格式的字符串
        QString formattedDateTime = currentDateTime.toString("yyyy-MM-dd hh:mm:ss");
        currentchannelMap.UpdateDateTime = formattedDateTime.toStdString();
        channelNameService->updateChannelName(currentchannelMap);
        ui->warning->setText(QString::fromLocal8Bit("保存成功"));
        ui->warning->setVisible(true);
    }
    else
    {
        for (auto channelMapName : electrodePreview->allChannel)
        {
            for (auto channel : channelMapName.electrodeSites)
            {
                if (channel.customName == ui->lineEdit->text())
                {
                    ui->warning->setText(QString::fromLocal8Bit("有重复名"));
                    ui->warning->setVisible(true);
                    return;
                }
                if (channelNameService->getChannelPositon(patientFK,ui->lineEdit->text()).first!=-1)
                {
                    ui->warning->setText(QString::fromLocal8Bit("有重复名"));
                    ui->warning->setVisible(true);
                    return;
                }
            }

        }
       
        currentchannelMap.Indexs = channelMap.at(currentSelectChannelName.second.second.toInt()).first.toStdString();
        currentchannelMap.ChannelNumber = currentSelectChannelName.second.first.toStdString();
        currentchannelMap.CustomName = ui->lineEdit->text().toStdString();
            QDateTime currentDateTime = QDateTime::currentDateTime();
            // 将日期时间格式化为 "yyyy-MM-dd hh:mm:ss" 格式的字符串
            QString formattedDateTime = currentDateTime.toString("yyyy-MM-dd hh:mm:ss");
            currentchannelMap.UpdateDateTime = formattedDateTime.toStdString();
            currentchannelMap.CreateDateTime = formattedDateTime.toStdString();
            currentchannelMap.ChannelCoordX = currentSelectChannelName.first.first;
            currentchannelMap.ChannelCoordY = currentSelectChannelName.first.second;
            currentchannelMap.ChannelPlanFK = channelPlanFK.toStdString();
            currentchannelMap.PatientFK = patientFK.toStdString();
            currentchannelMap.IsDeleted = false;
            currentchannelMap.UID = QUuid::createUuid().toString().remove("{").remove("}").remove("-").toStdString();
            channelNameService->updateChannelName(currentchannelMap);
            ui->warning->setText(QString::fromLocal8Bit("保存成功"));
            ui->warning->setVisible(true);
            return;
       
        
    }
}
void  CChannelMapWidget::initChannelMap(const QString & ChannelPlanFK)
{
    
      
        connect(electrodePreview, &CElectrodePreview::signalShowCurrentChannelName, this, &CChannelMapWidget::showCurrentChannelName);
        connect(electrodePreview, &CElectrodePreview::signalUpdateCurrentChannelName, this, &CChannelMapWidget::updateCurrentChannelName);
 
     
      
        LOG_INFO("channelMap................................size"+ QString::number(channelMap.size()).toStdString());
        for (int i = 0; i < channelMap.size(); i++)
        {
            LOG_INFO(" electrodePreview   populateTabWidget...................");
            electrodePreview->populateTabWidget(channelMap.at(i));         
        }
       
        ui->warning->setVisible(false);
        LOG_INFO(" start   setCurrentTab...................");
        electrodePreview->setCurrentTab(currentTab);
        LOG_INFO(" end   setCurrentTab...................");
        channelPlanFK = ChannelPlanFK;
   
}    
void CChannelMapWidget::searchChannelMap()
{
    auto custname = ui->lineEdit->text();
    auto position = channelNameService->getChannelPositon(patientFK,custname);
    if (position == QPair(-1, -1))
    {
        ui->warning->setText(QString::fromLocal8Bit("未找到该通道!"));
        ui->warning->setVisible(true);
        return;
        /*auto number = custname.mid(2, custname.length());
       
        if (electrodePreview->searchChannelMap(custname.mid(0, 1), QString::number(number.toInt())))
        {
            return;
        }
        else
        {
            ui->warning->setText(QString::fromLocal8Bit("未找到该通道!"));
            ui->warning->setVisible(true);
            return;
        }*/
        
    }
    else
    {
       
      
        
            QPair  <int, int>posi;
            posi.first = position.first;
            posi.second = position.second;
            if (electrodePreview->searchChannelMap(custname.mid(0, 1), posi))
            {
                ui->warning->setVisible(false);
            }

            else
            {
                ui->warning->setText(QString::fromLocal8Bit("未找到该通道!"));
                ui->warning->setVisible(true);
            }
      
      
    }
}

END_NX_NAMESPACE