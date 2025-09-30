/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#include "TrainPage.h"
#include "CBinding.h"
#include "CTreatmentCfg.h"
#include "CTrainRecord.h"
#include "CChannelPlanWidget.h"
#include "CDetect.h"
#include "CChannelMapWidget.h"
#include "../CNxMessageBox.h"
#include "NxAppService/CTreatmentRecordService.h"
#include "NxAppService/CStandardTreatmentService.h"
#include "NxAppService/CTrainService.h"
#include "NxEntity/TreatmentRecord.h"
#include "NxEntity/Patient.h"
#include "NxEntity/IdentityInfo.h"
#include "Optimus/CPatientInfo.h"
#include "NxSystemCfg/CSystemCfg.h"
#include "NxAppService/CWebSocketService.h"

#include <QImage>
#include <QBitmap>
#include <QPainter>
#include <QProcess>

BEGIN_NX_NAMESPACE
TrainPage::TrainPage(QWidget* parent)
    : QWidget(parent),
    service(new CTrainService(this)),
    channelMapWidget(new CChannelMapWidget(this)),
    recordComponent(nullptr),
    channelPlanComponent(nullptr),
    detectComponent(nullptr),
    treatmentCfgComponent(nullptr),
    bindingComponent(nullptr),
    patientInfo(nullptr),
    currentComponentID(ID_UNKNOWN),
    preComponentID(ID_UNKNOWN),
    isFoldPanel(false)
{
    ui = std::make_unique<Ui::TrainPage>();
    ui->setupUi(this);

    Patient patientInfo;
    service->getPatientInfo(currentPatientUid, patientInfo);
    IdentityInfo identityInfo;
    service->getPatientIdentifyInfo(QString::fromStdString(patientInfo.IdentifyFK), identityInfo);

    webSocketService = new CWebSocketService(systemConfig.GetSystemCfg().Common.TrainAddress, this);
    standardTreatmentService = new CStandardTreatmentService(QString::fromStdString(patientInfo.UID), this);
    auto standardTreatmentList = standardTreatmentService->getStandardTreatment(patientInfo.IllType-1, patientInfo.IllStage);
    if (standardTreatmentList.empty())
    {
        standardTreatmentService->currentPatientUid = "123";
        standardTreatmentList= standardTreatmentService->getStandardTreatment(patientInfo.IllType, patientInfo.IllStage);
    }
    for (int i=0;i< standardTreatmentList.size();i++)
    {
	    if (standardTreatmentList[i].IsChecked)
	    {
            st = standardTreatmentList[i];
            break;
	    }
    }
    
    webSocketService->initGame(identityInfo, patientInfo, st);
    connect(ui->startBtn, &QPushButton::clicked, this, &TrainPage::slotStartTraining);

    ui->bindingBtn->setEnabled(false);
    ui->bindingFoldBtn->setEnabled(false);

    currentPatientUid = IAppService::getCurrentPatientUid();

    updatePatientInfo();

    checkStepBtn(ID_TRAIN_RECORD);
    connect(ui->editPatient, SIGNAL(clicked()), this, SLOT(slotEditPatient()));
    connect(ui->channelPlanBtn, SIGNAL(clicked()), this, SLOT(slotChannelPlanBtnClicked()));
    connect(ui->realtimeDetectBtn, SIGNAL(clicked()), this, SLOT(slotRealtimeDetectBtnClicked()));
    connect(ui->treatmentCfgBtn, SIGNAL(clicked()), this, SLOT(slotTreatmentCfgBtnClicked()));
    connect(ui->bindingBtn, SIGNAL(clicked()), this, SLOT(slotBindingBtnClicked()));
    connect(ui->trainRecordBtn, SIGNAL(clicked()), this, SLOT(slotTrainRecordBtnClicked()));

    connect(ui->channelPlanFoldBtn, SIGNAL(clicked()), this, SLOT(slotChannelPlanBtnClicked()));
    connect(ui->detectFoldBtn, SIGNAL(clicked()), this, SLOT(slotRealtimeDetectBtnClicked()));
    connect(ui->treatmentCfgFoldBtn, SIGNAL(clicked()), this, SLOT(slotTreatmentCfgBtnClicked()));
    connect(ui->bindingFoldBtn, SIGNAL(clicked()), this, SLOT(slotBindingBtnClicked()));
    connect(ui->recordFoldBtn, SIGNAL(clicked()), this, SLOT(slotTrainRecordBtnClicked()));

    connect(ui->foldBtn, SIGNAL(clicked()), this, SLOT(slotTogglePanel()));
    connect(ui->extendBtn, SIGNAL(clicked()), this, SLOT(slotTogglePanel()));
    connect(channelMapWidget, &CChannelMapWidget::hideChannelMapWidget, this, &TrainPage::hideChannelMap);

    if (isFoldPanel) {
        ui->foldPanel->show();
        ui->leftPanel->hide();
    }
    else {
        ui->foldPanel->hide();
        ui->leftPanel->show();
    }

    slotSetComponentStatus();
}

TrainPage::~TrainPage() = default;

bool TrainPage::isRecording() const
{
    if (detectComponent) {
        bool isRun = detectComponent->isRecording();
        return isRun;
    }
    return false;
}

void TrainPage::stopRecord() const
{
    if (detectComponent) {
        return detectComponent->stopRecord();
    }
}

void TrainPage::slotTogglePanel() {
    isFoldPanel = !isFoldPanel;
    if (isFoldPanel) {
        ui->foldPanel->show();
        ui->leftPanel->hide();
    }
    else {
        ui->foldPanel->hide();
        ui->leftPanel->show();
    }
}

void TrainPage::slotChannelPlanBtnClicked()
{
    isInitOscillograph = false;
    checkStepBtn(ID_CHANNEL_PLAN);
}

void TrainPage::slotRealtimeDetectBtnClicked()
{
    checkStepBtn(ID_REALTIME_DETECT);
    if (detectComponent&&!isInitOscillograph)
    {
        isInitOscillograph = true;
        detectComponent->initOscillograph();
    }
}

void TrainPage::slotTreatmentCfgBtnClicked()
{
    checkStepBtn(ID_TREATMENT_CONFIG);
    isInitOscillograph = false;
}

void TrainPage::slotBindingBtnClicked()
{
    //checkStepBtn(ID_BINDING_DEVICE);
}

void TrainPage::slotTrainRecordBtnClicked()
{
    checkStepBtn(ID_TRAIN_RECORD);
    isInitOscillograph = false;
}

void TrainPage::slotRealtimeDetectJumpToPrevious()
{
    checkStepBtn(preComponentID);
    isInitOscillograph = false;
}

void TrainPage::slotStartTraining()
{
    st.IsChecked = 0;
    Patient patientInfo;
    service->getPatientInfo(currentPatientUid, patientInfo);
    IdentityInfo identityInfo;
    service->getPatientIdentifyInfo(QString::fromStdString(patientInfo.IdentifyFK), identityInfo);
    auto standardTreatmentList = standardTreatmentService->getStandardTreatment(patientInfo.IllType - 1, patientInfo.IllStage);
    if (standardTreatmentList.empty())
    {
        standardTreatmentService->currentPatientUid = "123";
        standardTreatmentList = standardTreatmentService->getStandardTreatment(patientInfo.IllType, patientInfo.IllStage);
    }
    for (int i = 0; i < standardTreatmentList.size(); i++)
    {
        if (standardTreatmentList[i].IsChecked)
        {
            st = standardTreatmentList[i];
            break;
        }
    }

    webSocketService->initGame(identityInfo, patientInfo, st);
	if (st.IsChecked)
	{
        const QString processName = "NxMotionTraining.exe";
   QProcess process;
   QString command = QString("tasklist /FI \"IMAGENAME eq %1\"").arg("NxMotionTraining.exe");
   process.start(command);
   process.waitForFinished(-1); // 等待进程完成
   QString output = QString::fromLocal8Bit(process.readAllStandardOutput());
   if (output.contains("NxMotionTraining.exe")) {
       QString killCommand = QString("taskkill /F /IM %1").arg(processName);
       process.start(killCommand);
       process.waitForFinished(-1); // 等待进程完成
   }
   process.setProgram(QString::fromStdString(systemConfig.GetSystemCfg().Common.GamePath)+"NxMotionTraining.exe");
   process.setWorkingDirectory(QString::fromStdString(systemConfig.GetSystemCfg().Common.GamePath)); // 设置工作路径
   process.startDetached();
	}
   

}

void TrainPage::slotEditPatient()
{
    if (!patientInfo)
    {
        patientInfo = new CPatientInfo(tr("Train"), false, currentPatientUid, this);
        connect(patientInfo, SIGNAL(signalEditPatientExit(QString)), SLOT(slotEditPatientExit(QString)));
        connect(patientInfo, SIGNAL(signalPatientDeleted()), SLOT(slotPatientDeleted()));

        ui->patientLayout->addWidget(patientInfo);
    }
    ui->mainStackedWidget->setCurrentWidget(ui->patientPage);
}

void TrainPage::slotEditPatientExit(const QString&)
{
    updatePatientInfo();
    ui->mainStackedWidget->setCurrentWidget(ui->trainPage);
    patientInfo->deleteLater();
    patientInfo = nullptr;
}

void TrainPage::checkStepBtn(TrainCommponentID id)
{
    LOG_INFO("checkStepBtn ..........................start");

    if (id == currentComponentID)
    {
        return;
    }

    // 正在录制过程中，退出前给出提示
    if (currentComponentID == ID_REALTIME_DETECT && detectComponent && detectComponent->isRecording()) {
        CNxMessageBox msgBox(tr("Realtime Detect"), tr("Recording, are you sure you want to quit?"));
        msgBox.exec();
        if (msgBox.isConfirmed()) {
            detectComponent->stopRecord();
        }
        else {
            switch (id)
            {
            case ID_CHANNEL_PLAN:
                ui->channelPlanBtn->setChecked(false);
                break;
            case ID_TREATMENT_CONFIG:
                ui->treatmentCfgBtn->setChecked(false);
                break;
            case ID_TRAIN_RECORD:
                ui->trainRecordBtn->setChecked(false);
                break;
            case ID_REALTIME_DETECT:
                break;
            case ID_UNKNOWN:
                break;
            }
            return;
        }
    }

    preComponentID = currentComponentID;
    currentComponentID = id;

    // 清除所有按钮选中状态
    const auto btns = {
       ui->channelPlanBtn,
       ui->realtimeDetectBtn,
       ui->treatmentCfgBtn,
       ui->trainRecordBtn,

       ui->channelPlanFoldBtn,
       ui->detectFoldBtn,
       ui->treatmentCfgFoldBtn,
       ui->recordFoldBtn
    };

    for (const auto btn : btns)
    {
        btn->setChecked(false);
    }

    // 销毁旧的组件
    switch (preComponentID)
    {
    case ID_CHANNEL_PLAN:
        ui->channelPlanContainer->removeWidget(channelPlanComponent);
        delete channelPlanComponent;
        channelPlanComponent = nullptr;
        break;
    case ID_REALTIME_DETECT:
        ui->detectContainer->removeWidget(detectComponent);
        delete detectComponent;
        detectComponent = nullptr;
        break;
    case ID_TREATMENT_CONFIG:
        ui->treatmentCfgContainer->removeWidget(treatmentCfgComponent);
        delete treatmentCfgComponent;
        treatmentCfgComponent = nullptr;
        break;
    case ID_TRAIN_RECORD:
        ui->recordContainer->removeWidget(recordComponent);
        delete recordComponent;
        recordComponent = nullptr;
        break;
    case ID_UNKNOWN:
        break;
    }
    componentIdIndexMap.remove(preComponentID);

    // 添加新的组件
    switch (currentComponentID)
    {
    case ID_CHANNEL_PLAN:
        if (!componentIdIndexMap.contains(id)) {
            channelPlanComponent = new CChannelPlanWidget(currentPatientUid, this);
            connect(channelPlanComponent, &CChannelPlanWidget::channelMapShow, this, &TrainPage::showChannelMap);
            connect(channelPlanComponent, &CChannelPlanWidget::signalChannelPlanChanged, this, &TrainPage::slotSetComponentStatus);
            ui->channelPlanContainer->addWidget(channelPlanComponent);
        }
        ui->channelPlanBtn->setChecked(true);
        ui->channelPlanFoldBtn->setChecked(true);
        ui->stackedWidgetRight->setCurrentWidget(ui->channelPlanPage);
        if (channelMapWidget)
        {
            channelMapWidget->hide();
        }
        break;
    case ID_REALTIME_DETECT:
        if (!componentIdIndexMap.contains(id)) {
            detectComponent = new CDetect(currentPatientUid, this);
            ui->detectContainer->addWidget(detectComponent);
            connect(detectComponent, SIGNAL(signalExitOscillograph()), this, SLOT(slotRealtimeDetectJumpToPrevious()));
        }
        ui->realtimeDetectBtn->setChecked(true);
        ui->detectFoldBtn->setChecked(true);
        ui->stackedWidgetRight->setCurrentWidget(ui->detectPage);
        break;
    case ID_TREATMENT_CONFIG:
        if (!componentIdIndexMap.contains(id)) {
            treatmentCfgComponent = new CTreatmentCfg(this);
            ui->treatmentCfgContainer->addWidget(treatmentCfgComponent);
            Patient patientInfo;
            service->getPatientInfo(currentPatientUid, patientInfo);
            treatmentCfgComponent->setPatientInfo(currentPatientUid, patientInfo.IllType - 1, patientInfo.IllStage);
            treatmentCfgComponent->initWidget();
        }
        ui->treatmentCfgBtn->setChecked(true);
        ui->treatmentCfgFoldBtn->setChecked(true);
        ui->stackedWidgetRight->setCurrentWidget(ui->treatmentCfgPage);
        break;
    case ID_TRAIN_RECORD:
        if (!componentIdIndexMap.contains(id)) {
            recordComponent = new CTrainRecord(currentPatientUid, this);
            ui->recordContainer->addWidget(recordComponent);
        }
        ui->trainRecordBtn->setChecked(true);
        ui->recordFoldBtn->setChecked(true);
        ui->stackedWidgetRight->setCurrentWidget(ui->recordPage);
        break;
    case ID_UNKNOWN:
        break;
    }

    componentIdIndexMap.insert(currentComponentID, ui->stackedWidgetRight->currentIndex());

    LOG_INFO("SlotJumpToPage ..........................finished");
}
void TrainPage::hideChannelMap() const
{
    channelPlanComponent->show();
    channelMapWidget->hide();
    ui->channelPlanContainer->removeWidget(channelMapWidget);
    ui->channelPlanContainer->addWidget(channelPlanComponent);
}

void TrainPage::slotSetComponentStatus() const
{
    const auto channelPlans = service->getChannelPlans(currentPatientUid);
    if (channelPlans.empty())
    {
        ui->realtimeDetectBtn->setEnabled(false);
        ui->detectFoldBtn->setEnabled(false);
        ui->startBtn->setEnabled(false);
        ui->startFoldBtn->setEnabled(false);
    }
    else
    {
        ui->realtimeDetectBtn->setEnabled(true);
        ui->detectFoldBtn->setEnabled(true);
        ui->startBtn->setEnabled(true);
        ui->startFoldBtn->setEnabled(true);
    }
}

void TrainPage::showChannelMap(const int currentRow)
{
    LOG_INFO("start..................channelMapShow.");
    channelPlanComponent->hide();
    ui->channelPlanContainer->removeWidget(channelPlanComponent);
    LOG_INFO("delete channelMapWidget ");
    delete channelMapWidget;
    LOG_INFO("new  channelMapWidget ");
    channelMapWidget = new CChannelMapWidget(this);
    connect(channelMapWidget, &CChannelMapWidget::hideChannelMapWidget, this, &TrainPage::hideChannelMap);
    ui->channelPlanContainer->addWidget(channelMapWidget);
    channelMapWidget->channelMap = channelPlanComponent->channelMap;
    channelMapWidget->patientFK = channelPlanComponent->currentPatientUid;
    channelMapWidget->currentTab = currentRow;
    LOG_INFO("init..................channelMapShow.");
    channelMapWidget->initChannelMap(channelPlanComponent->channelPlanFK);
}

void TrainPage::updatePatientInfo()
{
    Patient patientInfo;
    service->getPatientInfo(currentPatientUid, patientInfo);

    IdentityInfo identityInfo;
    service->getPatientIdentifyInfo(QString::fromStdString(patientInfo.IdentifyFK), identityInfo);

    ui->patientName->setText(QString::fromStdString(patientInfo.PatientName));
    ui->age->setText(QString::number(patientInfo.getAge()) + tr("Years"));
    ui->caseId->setText(QString::fromStdString(patientInfo.CaseID));

    showAvatar(identityInfo.Avatar, patientInfo.Gender);
    showIllStage(static_cast<TypeOfIllness>(patientInfo.IllType), patientInfo.IllStage);
    showGender(static_cast<GenderEmu>(patientInfo.Gender));
}

void TrainPage::showAvatar(const std::vector<char>& avatar, const char gender) const
{
    if (!avatar.empty()) {
        QByteArray ba;
        for (const char i : avatar)
        {
            ba.append(i);
        }

        QImage img;
        img.loadFromData(ba, "jpg");
        ba.clear();
        constexpr auto borderWidth = 2;
        auto showImg = QPixmap::fromImage(img.scaled(ui->avator->width() - borderWidth * 2, ui->avator->height() - borderWidth * 2, Qt::KeepAspectRatio));
        const auto radius = ui->avator->width() / 2 - borderWidth;
        const QSize size(ui->avator->width() - borderWidth * 2, ui->avator->height() - borderWidth * 2);
        QBitmap mask(size);
        QPainter painter(&mask);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::HighQualityAntialiasing);
        painter.fillRect(0, 0, size.width(), size.height(), Qt::white);
        painter.setBrush(QColor(0, 0, 0));
        painter.drawEllipse(QPoint(radius, radius), radius, radius);
        showImg.setMask(mask);
        ui->avator->setPixmap(showImg);
        ui->avatorFold->setPixmap(showImg.scaled(ui->avatorFold->width() - borderWidth, ui->avatorFold->height() - borderWidth));
    }
    else {
        QString style, foldStyle;
        if (gender == male) {
            style = "QLabel { \
                                image: url(:/Optimus/images/common/man-avator.svg); \
                                background-color: #ffffff; \
                                border-radius: 47px; \
                                border: 2px solid gray; \
                            } ";
            foldStyle = "QLabel { \
                                image: url(:/Optimus/images/common/man-avator.svg); \
                                background-color: #ffffff; \
                                border-radius: 17px; \
                                border: 2px solid gray; \
                            } ";
        }
        else {
            style = "QLabel { \
                                image: url(:/Optimus/images/common/woman-avator.svg); \
                                background-color: #ffffff; \
                                border-radius: 47px; \
                                border: 2px solid gray; \
                            } ";

            foldStyle = "QLabel { \
                                image: url(:/Optimus/images/common/woman-avator.svg); \
                                background-color: #ffffff; \
                                border-radius: 17px; \
                                border: 2px solid gray; \
                            } ";
        }
        ui->avator->setStyleSheet(style);
        ui->avatorFold->setStyleSheet(foldStyle);
    }
}

void TrainPage::showIllStage(TypeOfIllness illType, char illStage) const
{
    QString illStageStr;
    if (illType == UNKOWN) {
        illStageStr = tr("Unselected");
    }
    else if (illType == ALS) {
        const QString levels[] = { "1", "2", "2", "3", "4", "4", "5" };
        const QString suffix[] = { "", "A", "B", "", "A", "B", "" };
        illStageStr = tr("ALS") + levels[illStage] + tr("Phase") + suffix[illStage];
    }
    else if (illType == Seizures) {
        const QString levels[] = { "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX" };
        illStageStr = tr("Seizures") + levels[illStage] + tr("Level");
    }
    else if (illType == Depression) {
        const QString levels[] = { "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX" };
        illStageStr = tr("Depression") + levels[illStage] + tr("Level");
    }
    else if (illType == SCI) {
        const QString levels[] = { "A", "B", "C", "D", "E", "F", "G", "H", "I" };
        illStageStr = tr("SCI") + levels[illStage] + tr("Level");
    }
    else if (illType == PD) {
        const QString levels[] = { "0", "1", "1.5", "2", "2.5", "3", "4", "5" };
        illStageStr = tr("PD") + levels[illStage] + tr("Phase");
    }
    else if (illType == Blindness) {
        const QString levels[] = { "1", "2", "3", "4", "5", "6", "7", "8" };
        illStageStr = tr("Blindness") + levels[illStage] + tr("Level");
    }
    ui->stage->setText(illStageStr);
}

void TrainPage::showGender(const GenderEmu gender) const
{
    QString genderLabelStyle;
    if (gender == male) {
        genderLabelStyle = "image: url(:/Optimus/images/common/male.svg);";
    }
    else {
        genderLabelStyle = "image: url(:/Optimus/images/common/female.svg);";
    }
    ui->gender->setStyleSheet(genderLabelStyle);
}

END_NX_NAMESPACE

