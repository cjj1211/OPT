#include "CPatientInfo.h"
#include "CNxMessageBox.h"
#include "CRecordFace.h"
#include "NxEntity/Patient.h"
#include "NxEntity/IdentityInfo.h"
#include "NxAppService/CPatientInfoService.h"
#include <QButtonGroup>
#include <QRegExpValidator>
#include <QDebug>
#include <QBuffer>
#include <QPainter>
#include <QBitmap>
#include <vector>


BEGIN_NX_NAMESPACE

CPatientInfo::CPatientInfo(const QString& prePageName, bool canBeDelete, const QString& patientUid, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::CPatientInfo())
    , service(new CPatientInfoService(this))
    , identityInfo(new IdentityInfo)
    , patientInfo(new Patient())
    , recordFace(nullptr)
    , curPatientUid(patientUid)
    , gender(Male)
    , genderGroup(new QButtonGroup(this))
    , alsGroup(new QButtonGroup(this))
    , seizuresGroup(new QButtonGroup(this))
    , depressionGroup(new QButtonGroup(this))
    , sciGroup(new QButtonGroup(this))
    , pdGroup(new QButtonGroup(this))
    , blindnessGroup(new QButtonGroup(this))
    , stateGroup(new QButtonGroup(this))
    , alsState(0)
    , seizuresState(0)
    , depressionState(0)
    , sciState(0)
    , pdState(0)
    , blindnessState(0)
    , illType(0)
    , isEditPatient(!patientUid.isEmpty())
{
    ui->setupUi(this);
    ui->deleteBtn->setVisible(canBeDelete);

    ui->prePageName->setText(prePageName);
    if (isEditPatient) {
        connect(ui->prePageName, &QPushButton::clicked, [this, patientUid]() {
            emit signalEditPatientExit(patientUid);
            });
    }
    else {
        connect(ui->prePageName, SIGNAL(clicked()), SIGNAL(signalExitCreatePatient()));
    }

    ui->nameErrIcon->hide();
    ui->nameErrMsg->hide();
    ui->caseIdErrorIcon->hide();
    ui->caseIdErrorMsg->hide();

    genderGroup->addButton(ui->maleRadio, Male);
    genderGroup->addButton(ui->femaleRadio, Female);
    connect(genderGroup, SIGNAL(buttonToggled(int, bool)), SLOT(slotGenderSelected(int, bool)));

    alsGroup->addButton(ui->alsP1, 0);
    alsGroup->addButton(ui->alsP2A, 1);
    alsGroup->addButton(ui->alsP2B, 2);
    alsGroup->addButton(ui->alsP3, 3);
    alsGroup->addButton(ui->alsP4A, 4);
    alsGroup->addButton(ui->alsP4B, 5);
    alsGroup->addButton(ui->alsP5, 6);
    connect(alsGroup, SIGNAL(buttonToggled(int, bool)), SLOT(slotALSSelected(int, bool)));

    seizuresGroup->addButton(ui->seizuresL1, 0);
    seizuresGroup->addButton(ui->seizuresL2, 1);
    seizuresGroup->addButton(ui->seizuresL3, 2);
    seizuresGroup->addButton(ui->seizuresL4, 3);
    connect(seizuresGroup, SIGNAL(buttonToggled(int, bool)), SLOT(slotSeizureRadioSlected(int, bool)));

    depressionGroup->addButton(ui->depressionL1, 0);
    depressionGroup->addButton(ui->depressionL2, 1);
    depressionGroup->addButton(ui->depressionL3, 2);
    connect(depressionGroup, SIGNAL(buttonToggled(int, bool)), SLOT(slotDepressionRadioSelected(int, bool)));

    sciGroup->addButton(ui->SCIA, 0);
    sciGroup->addButton(ui->SCIB, 1);
    sciGroup->addButton(ui->SCIC, 2);
    sciGroup->addButton(ui->SCID, 3);
    connect(sciGroup, SIGNAL(buttonToggled(int, bool)), SLOT(slotSCIRadioSelected(int, bool)));

    pdGroup->addButton(ui->PD0, 0);
    pdGroup->addButton(ui->PD1, 1);
    pdGroup->addButton(ui->PD1_5, 2);
    pdGroup->addButton(ui->PD2, 3);
    pdGroup->addButton(ui->PD2_5, 4);
    pdGroup->addButton(ui->PD3, 5);
    pdGroup->addButton(ui->PD4, 6);
    pdGroup->addButton(ui->PD5, 7);
    connect(pdGroup, SIGNAL(buttonToggled(int, bool)), SLOT(slotPDRadioSelected(int, bool)));

    blindnessGroup->addButton(ui->Bindness1, 0);
    blindnessGroup->addButton(ui->Bindness2, 1);
    blindnessGroup->addButton(ui->Bindness3, 2);
    blindnessGroup->addButton(ui->Bindness4, 3);
    blindnessGroup->addButton(ui->Bindness5, 4);
    blindnessGroup->addButton(ui->Bindness6, 5);
    blindnessGroup->addButton(ui->Bindness7, 6);
    blindnessGroup->addButton(ui->Bindness8, 7);
    connect(blindnessGroup, SIGNAL(buttonToggled(int, bool)), SLOT(slotBlindnessRadioSelected(int, bool)));

    stateGroup->addButton(ui->alsBtn, ALS);
    stateGroup->addButton(ui->seizuresBtn, Seizures);
    stateGroup->addButton(ui->depressionBtn, Depression);
    stateGroup->addButton(ui->sciBtn, SCI);
    stateGroup->addButton(ui->pdBtn, PD);
    stateGroup->addButton(ui->blindnessBtn, Blindness);
    connect(stateGroup, SIGNAL(buttonClicked(int)), SLOT(slotSickStateChanged(int)));

    nameValidator = new QRegExpValidator(QRegExp("[\u4e00-\u9fa5a-zA-Z]{1,20}"), this);
    caseIDValidator = new QRegExpValidator(QRegExp("[a-zA-Z0-9]+$"), this);
    ui->name->setValidator(nameValidator);
    ui->caseId->setValidator(caseIDValidator);
    ui->birthday->setMaximumDate(QDate::currentDate());

    connect(ui->caseId, SIGNAL(textChanged(QString)), SLOT(slotCaseIDChanged(QString)));
    connect(ui->name, SIGNAL(textChanged(QString)), SLOT(slotNameChanged(QString)));
    connect(ui->birthday, SIGNAL(dateChanged(QDate)), SLOT(slotBirthdayChanged(QDate)));
    connect(ui->saveBtn, SIGNAL(clicked()), SLOT(slotSave()));
    connect(ui->deleteBtn, SIGNAL(clicked()), SLOT(slotDelete()));
    connect(ui->openCamBtn, SIGNAL(clicked()), SLOT(slotCapture()));
    connect(ui->diseaseCourse, SIGNAL(textChanged()), SLOT(slotDiseaseEditorChanged()));
    connect(ui->insertDiseaseBtn, SIGNAL(clicked()), SLOT(slotInsertDiseaseInfo()));
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), SLOT(slotTabChanged(int)));

    if (isEditPatient) {
        connect(ui->cancelBtn, &QPushButton::clicked, [this, patientUid]() {
            emit signalEditPatientExit(patientUid);
            });
    }
    else {
        ui->alsBtn->click();
        connect(ui->cancelBtn, SIGNAL(clicked()), SIGNAL(signalExitCreatePatient()));
    }

    if (!isEditPatient)
    {
        ui->deleteBtn->hide();
    }
    else {
        service->getPatientByUid(patientUid, *patientInfo);
        service->getPatientIdentifyInfo(QString::fromStdString(patientInfo->IdentifyFK), *identityInfo);
        showAvatar();
        ui->name->setText(QString::fromStdString(patientInfo->PatientName));
        ui->caseId->setText(QString::fromStdString(patientInfo->CaseID));
        ui->birthday->setDate(QDate::fromString(QString::fromStdString(patientInfo->Birthday).split(' ')[0], "yyyy-MM-dd"));
        ui->note->setPlainText(QString::fromStdString(patientInfo->Note));
        ui->deviceSN->setText(QString::fromStdString(patientInfo->DeviceID));
        if (patientInfo->Gender == male)
        {
            ui->maleRadio->setChecked(true);
        }
        else
        {
            ui->femaleRadio->setChecked(true);
        }
        if (patientInfo->IllType != UNKOWN) {
            stateGroup->button(patientInfo->IllType)->click();
            switch (static_cast<TypeOfIllness>(patientInfo->IllType)) {
            case     UNKOWN:
                break;
            case     ALS:        // ½¥¶³Ö¢
                alsGroup->button(patientInfo->IllStage)->setChecked(true);
                break;
            case     Seizures:   // ñ²ðï
                seizuresGroup->button(patientInfo->IllStage)->setChecked(true);
                break;
            case     Depression: // ÒÖÓôÖ¢
                depressionGroup->button(patientInfo->IllStage)->setChecked(true);
                break;
            case     SCI:        // ¼¹ËèËðÉË
                sciGroup->button(patientInfo->IllStage)->setChecked(true);
                break;
            case     PD:         // ÅÁ½ðÉ­
                pdGroup->button(patientInfo->IllStage)->setChecked(true);
                break;
            case     Blindness:   // Ê§Ã÷
                blindnessGroup->button(patientInfo->IllStage)->setChecked(true);
                break;
            }
        }
    }

    ui->diseaseCourse->setHtml(QString::fromStdString(patientInfo->DiseaseCourse));
}

CPatientInfo::~CPatientInfo()
{
    qDebug() << "CPatientInfo Released.";
    service->deleteLater();
    delete ui;
}

void CPatientInfo::resizeEvent(QResizeEvent* event) {
    auto tabRegion = ui->courseTab->geometry();
    tabRegion.adjust(10, 10, -10, -10);
    ui->diseaseCourse->setGeometry(tabRegion);
    ui->insertDiseaseBtn->setGeometry(QRect(tabRegion.right() - ui->insertDiseaseBtn->width() - 10, tabRegion.top() + 10,
        ui->insertDiseaseBtn->width(), ui->insertDiseaseBtn->height()));
    ui->insertDiseaseBtn->raise();
}

void CPatientInfo::showEvent(QShowEvent* event)
{
    auto tabRegion = ui->courseTab->geometry();
    tabRegion.adjust(10, 10, -10, -10);
    ui->diseaseCourse->setGeometry(tabRegion);
    ui->insertDiseaseBtn->setGeometry(QRect(tabRegion.right() - ui->insertDiseaseBtn->width() - 10, tabRegion.top() + 10,
        ui->insertDiseaseBtn->width(), ui->insertDiseaseBtn->height()));
    ui->insertDiseaseBtn->raise();
}

void CPatientInfo::showAvatar() const
{
    if (!identityInfo->Avatar.empty()) {
        QByteArray ba;
        for (const char i : identityInfo->Avatar)
        {
            ba.append(i);
        }

        QImage img;
        img.loadFromData(ba, "jpg");
        ba.clear();
        auto showImg = QPixmap::fromImage(img.scaled(ui->avator->width(), ui->avator->height(), Qt::KeepAspectRatio));

        const auto radius = ui->avator->width() / 2;
        const QSize size(ui->avator->width(), ui->avator->height());
        QBitmap mask(size);
        QPainter painter(&mask);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::HighQualityAntialiasing);
        painter.fillRect(0, 0, size.width(), size.height(), Qt::white);
        painter.setBrush(QColor(220, 0, 0));
        painter.drawEllipse(QPoint(radius, radius), radius, radius);
        showImg.setMask(mask);
        ui->avator->setPixmap(showImg);
    }
}

void CPatientInfo::slotGenderSelected(int id, bool status) {
    if (status)
    {
        gender = static_cast<BG_Gender>(id);
        QString style;
        if (gender == male)
        {
            style = "QLabel{\
                            background-color: rgba(146, 156, 192, 1);\
                            border-radius: 42px;\
                            background-image: url(:/Optimus/images/common/man-avator.svg);\
                            background-repeat: no-repeat;\
                            background-position: center;\
                        }";
            ui->avator->setStyleSheet(style);
        }
        else
        {
            style = "QLabel{\
                            background-color: rgba(146, 156, 192, 1);\
                            border-radius: 42px;\
                            background-image: url(:/Optimus/images/common/woman-avator.svg);\
                            background-repeat: no-repeat;\
                            background-position: center;\
                        }";
            ui->avator->setStyleSheet(style);
        }
    }
}

void CPatientInfo::slotALSSelected(int id, bool status) {
    if (status) alsState = id;
}

void CPatientInfo::slotSeizureRadioSlected(int id, bool status) {
    if (status) seizuresState = id;
}

void CPatientInfo::slotDepressionRadioSelected(int id, bool status) {
    if (status) depressionState = id;
}

void CPatientInfo::slotSCIRadioSelected(int id, bool status) {
    if (status) sciState = id;
}

void CPatientInfo::slotPDRadioSelected(int id, bool status) {
    if (status) pdState = id;
}

void CPatientInfo::slotBlindnessRadioSelected(int id, bool status) {
    if (status) blindnessState = id;
}

void CPatientInfo::slotSickStateChanged(int id) {
    illType = id;
}

void CPatientInfo::slotNameChanged(QString name) const
{
    name.replace(QString::fromLocal8Bit("¡¾"), "");
    name.replace(QString::fromLocal8Bit("¡¿"), "");
    name.replace(QString::fromLocal8Bit("£¿"), "");
    name.replace(QString::fromLocal8Bit("£¡"), "");
    name.replace(QString::fromLocal8Bit("¡¤"), "");
    name.replace(QString::fromLocal8Bit("£¤"), "");
    name.replace(QString::fromLocal8Bit("¡­¡­"), "");
    name.replace(QString::fromLocal8Bit("£¨"), "");
    name.replace(QString::fromLocal8Bit("£©"), "");
    name.replace(QString::fromLocal8Bit("¡ª¡ª"), "");
    name.replace(QString::fromLocal8Bit("¡¢"), "");
    name.replace(QString::fromLocal8Bit("£º"), "");
    name.replace(QString::fromLocal8Bit("£»"), "");
    name.replace(QString::fromLocal8Bit("¡°"), "");    //ÒýºÅÁ½±ß·Ö¿ª¹ýÂË
    name.replace(QString::fromLocal8Bit("¡±"), "");
    name.replace(QString::fromLocal8Bit("¡¯"), "");    //ÒýºÅÁ½±ß·Ö¿ª¹ýÂË
    name.replace(QString::fromLocal8Bit("¡®"), "");
    name.replace(QString::fromLocal8Bit("¡¶"), "");
    name.replace(QString::fromLocal8Bit("¡·"), "");
    name.replace(QString::fromLocal8Bit("£¬"), "");
    name.replace(QString::fromLocal8Bit("¡£"), "");
    ui->name->setText(name);

    if (name.isEmpty()) {
        ui->nameErrIcon->show();
        ui->nameErrMsg->setText(tr("Patient name can not be empty."));
        ui->nameErrMsg->show();
        return;
    }
    if (name.size() > 20) {
        ui->nameErrIcon->show();
        ui->nameErrMsg->setText(tr("The character length must be between 1~20."));
        ui->nameErrMsg->show();
        return;
    }

    ui->nameErrIcon->hide();
    ui->nameErrMsg->hide();
}

void CPatientInfo::slotCaseIDChanged(QString id) {
    if (id.isEmpty()) {
        ui->caseIdErrorIcon->show();
        ui->caseIdErrorMsg->setText(tr("Case ID can not be empty."));
        ui->caseIdErrorMsg->show();
        return;
    }
    else if (id.size() > 20) {
        ui->caseIdErrorIcon->show();
        ui->caseIdErrorMsg->setText(tr("The character length must be between 1~20."));
        ui->caseIdErrorMsg->show();
    }
    else if (service->isCaseIdDuplicate(id, QString::fromStdString(patientInfo->UID))) {
        ui->caseIdErrorIcon->show();
        ui->caseIdErrorMsg->setText(tr("Case ID already exists."));
        ui->caseIdErrorMsg->show();
        return;
    }

    ui->caseIdErrorIcon->hide();
    ui->caseIdErrorMsg->hide();
}

void CPatientInfo::slotBirthdayChanged(QDate date) {
    birthday = date;
}

void CPatientInfo::slotDelete() {
    if (isEditPatient)
    {
        CNxMessageBox* msgBox = new CNxMessageBox(tr("Patient Info"), tr("The user and user associated information will be deleted, and cannot be restored after deletion."), this);
        msgBox->exec();
        if (msgBox->isConfirmed())
        {
            if (service->deletePatientByUid(curPatientUid)) {
                emit signalPatientDeleted();
            }
        }
    }
}

void CPatientInfo::slotSave() {
    if (ui->name->text().isEmpty())
    {
        ui->nameErrIcon->show();
        ui->nameErrMsg->setText(tr("Patient name can not be empty."));
        ui->nameErrMsg->show();
        return;
    }
    if (ui->name->text().size() > 20)
    {
        ui->nameErrIcon->show();
        ui->nameErrMsg->setText(tr("The character length must be between 1~20."));
        ui->nameErrMsg->show();
        return;
    }

    if (ui->caseId->text().isEmpty())
    {
        ui->caseIdErrorIcon->show();
        ui->caseIdErrorMsg->setText(tr("Case ID can not be empty."));
        ui->caseIdErrorMsg->show();
        return;
    }
    if (ui->caseId->text().size() > 20)
    {
        ui->caseIdErrorIcon->show();
        ui->caseIdErrorMsg->setText(tr("The character length must be between 1~20."));
        ui->caseIdErrorMsg->show();
        return;
    }
    if (service->isCaseIdDuplicate(ui->caseId->text(), QString::fromStdString(patientInfo->UID))) {
        ui->caseIdErrorIcon->show();
        ui->caseIdErrorMsg->setText(tr("Case ID already exists."));
        ui->caseIdErrorMsg->show();
        return;
    }

    identityInfo->Icon = std::vector<char>();
    identityInfo->Password = identityInfo->UID;
    identityInfo->setRole(Role_Patient);

    patientInfo->Birthday = ui->birthday->date().toString("yyyy-MM-dd").toStdString();
    patientInfo->CaseID = ui->caseId->text().toStdString();
    patientInfo->PatientName = ui->name->text().toStdString();
    patientInfo->Gender = static_cast<GenderEmu>(gender);
    patientInfo->Note = ui->note->toPlainText().toStdString();
    patientInfo->DeviceID = ui->deviceSN->text().toStdString();
    patientInfo->IllType = illType;
    if (ALS == illType) {
        patientInfo->IllStage = alsState;
    }
    else if (Seizures == illType) {
        patientInfo->IllStage = seizuresState;
    }
    else if (Depression == illType) {
        patientInfo->IllStage = depressionState;
    }
    else if (SCI == illType) {
        patientInfo->IllStage = sciState;
    }
    else if (PD == illType) {
        patientInfo->IllStage = pdState;
    }
    else if (Blindness == illType) {
        patientInfo->IllStage = blindnessState;
    }

    patientInfo->DiseaseCourse = ui->diseaseCourse->toHtml().toStdString();

    if (isEditPatient) {
        if (service->updatePatient(*identityInfo, *patientInfo))
        {
            LOG_INFO("Update patient " + patientInfo->PatientName + " SUCCESS.");
            emit signalEditPatientExit(curPatientUid);
        }
        else
        {
            LOG_ERR("Update patient " + patientInfo->PatientName + " FAIL.");
        }
    }
    else {
        if (service->createPatient(*identityInfo, *patientInfo))
        {
            LOG_INFO("Create patient " + patientInfo->PatientName + " SUCCESS.");
            emit signalExitCreatePatient();
        }
        else
        {
            LOG_ERR("Create patient " + patientInfo->PatientName + " FAIL.");
        }
    }
}

void CPatientInfo::slotBinding()
{

}

void CPatientInfo::slotCapture()
{
    if (recordFace == nullptr) {
        recordFace = new CRecordFace(service, curPatientUid, this);
        connect(recordFace, SIGNAL(signalUpdateAvator(QImage)), SLOT(slotUpdateAvator(QImage)));
    }

    recordFace->show();
}

void CPatientInfo::slotUpdateAvator(QImage img)
{
    QByteArray ba;
    QBuffer imgBuffer(&ba);
    imgBuffer.open(QIODevice::WriteOnly);
    img.save(&imgBuffer, "jpg");

    identityInfo->Avatar = std::vector<char>(ba.begin(), ba.end());
    const auto feature = NX::CPatientInfoService::extractFeature(img);
    identityInfo->Feature.resize(1024 * sizeof(float));
    memcpy(identityInfo->Feature.data(), feature.data(), 1024 * sizeof(float));
    ba.clear();
    showAvatar();
}

void CPatientInfo::slotDiseaseEditorChanged()
{
}

void CPatientInfo::slotInsertDiseaseInfo()
{
    QString info = ui->diseaseCourse->toHtml();

    auto date = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss ");
    //auto promte = tr("<p style=\"color:rgba(255,255,255,0.5);\">Please enter a description of your condition</p> ");
    auto promte = tr("<p>Please enter a description of your condition</p> ");
    info = date + promte + info;
    ui->diseaseCourse->setHtml(info);
}

void CPatientInfo::slotTabChanged(int)
{
    auto tabRegion = ui->courseTab->geometry();
    tabRegion.adjust(10, 10, -10, -10);
    ui->diseaseCourse->setGeometry(tabRegion);
    ui->insertDiseaseBtn->setGeometry(QRect(tabRegion.right() - ui->insertDiseaseBtn->width() - 10, tabRegion.top() + 10,
        ui->insertDiseaseBtn->width(), ui->insertDiseaseBtn->height()));
    ui->insertDiseaseBtn->raise();
}

END_NX_NAMESPACE
