/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#include "CPatientAdminPage.h"
#include "../CPatientInfoCard.h"
#include "../CPatientInfo.h"
#include"../../NxAppService/CPatientAdminService.h"
#include <QListWidget>
#include <QSpacerItem>

BEGIN_NX_NAMESPACE
CPatientAdminPage::CPatientAdminPage(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::CPatientAdminPage)
    , service(new CPatientAdminService(this))
    , createPatienComonent(nullptr)
    , editPatientComponent(nullptr) {

    ui->setupUi(this);

    connect(ui->searchEdit, SIGNAL(textChanged(QString)), SLOT(slotFilterChanged(QString)));
    connect(ui->selectAll, SIGNAL(clicked()), SLOT(slotSelectAll()));
    connect(ui->searchBtn, SIGNAL(clicked()), SLOT(slotSearch()));
    connect(ui->createBtn, SIGNAL(clicked()), SLOT(slotCreatePatient()));
    connect(ui->importBtn, SIGNAL(clicked()), SLOT(slotImport()));
    connect(ui->exportBtn, SIGNAL(clicked()), SLOT(slotExport()));
    connect(ui->forward, SIGNAL(clicked()), SLOT(slotForward()));
    connect(ui->back, SIGNAL(clicked()), SLOT(slotBack()));
    connect(ui->fastForward, SIGNAL(clicked()), SLOT(slotFastForward()));
    connect(ui->fastBack, SIGNAL(clicked()), SLOT(slotFastBack()));

    showPage(1);
}

CPatientAdminPage::~CPatientAdminPage() = default;

void CPatientAdminPage::showPage(int pageIndex) {
    const auto patientCnt = service->getPatientCount(filter);
    constexpr int patientCntPerPage = 12;

    auto allPageCnt = patientCnt / patientCntPerPage;
    if (patientCnt % patientCntPerPage > 0 || patientCnt == 0)
    {
        allPageCnt++;
    }

    if (pageIndex > allPageCnt)
    {
        pageIndex = allPageCnt;
    }

    ui->allPage->setText(QString::number(allPageCnt));
    ui->curPage->setText(QString::number(pageIndex));
    const auto patients = service->getPatientCurrentPage(pageIndex, patientCntPerPage, filter);
    for (const auto& uid : patientCardMap.keys()) {
        const auto card = patientCardMap[uid];
        ui->patientGrid->removeWidget(card);
        patientCardMap.remove(uid);
        card->deleteLater();
    }

    for (auto index : spacerMap.keys())
    {
        const auto spacer = spacerMap[index];
        ui->patientGrid->removeItem(spacer);
        spacerMap.remove(index);
        delete spacer;
    }
    CPatientInfoCard* firstCard = nullptr;
    for (int i = 0; i < 12; ++i)
    {
        if (i < patients.size()) {
            auto card = new CPatientInfoCard(service, QString::fromStdString(patients[i].UID), this);
            ui->patientGrid->addWidget(card, i / 4, i % 4);
            patientCardMap.insert(QString::fromStdString(patients[i].UID), card);
            connect(card, SIGNAL(signalPressed(QString)), SLOT(slotPatientCardPressed(QString)));
            connect(card, SIGNAL(signalDoubleClicked(QString)), SLOT(slotPatientCardDoubleClicked(QString)));
            connect(card, SIGNAL(signalJumpToTrainPage(QString)), SIGNAL(signalJumpToTrainPage(QString)));
            if (i == 0) firstCard = card;
        }
        else {
            auto placeHolder = new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Expanding);
            ui->patientGrid->addItem(placeHolder, i / 4, i % 4);
            spacerMap.insert(i, placeHolder);
        }
    }
    if (firstCard)
    {
        firstCard->setSelected(true);
    }
    ui->patientGrid->setRowStretch(0, 1);
    ui->patientGrid->setRowStretch(1, 1);
    ui->patientGrid->setRowStretch(2, 1);
    ui->patientGrid->setColumnStretch(0, 1);
    ui->patientGrid->setColumnStretch(1, 1);
    ui->patientGrid->setColumnStretch(2, 1);
    ui->patientGrid->setColumnStretch(3, 1);

    const auto curPage = ui->curPage->text().toInt();
    const auto allPage = ui->allPage->text().toInt();
    if (curPage >= allPage) {
        ui->forward->setEnabled(false);
    }
    else {
        ui->forward->setEnabled(true);
    }

    if (curPage == allPage) {
        ui->fastForward->setEnabled(false);
    }
    else {
        ui->fastForward->setEnabled(true);
    }

    if (curPage <= 1) {
        ui->back->setEnabled(false);
    }
    else {
        ui->back->setEnabled(true);
    }

    if (curPage == 1) {
        ui->fastBack->setEnabled(false);
    }
    else {
        ui->fastBack->setEnabled(true);
    }
}

void CPatientAdminPage::slotFilterChanged(QString text) {
    filter = text;
    showPage(1);
}

void CPatientAdminPage::slotCreatePatient() {
    if (createPatienComonent == nullptr) {
        createPatienComonent = new CPatientInfo(tr("Patient Admin"), false, "", this);
        connect(createPatienComonent, SIGNAL(signalExitCreatePatient()), SLOT(slotCreatePatientExit()));
    }
    ui->createPatientLayout->addWidget(createPatienComonent);
    ui->stackedWidget->setCurrentWidget(ui->createPatientPage);
}

void CPatientAdminPage::slotCreatePatientExit() {
    ui->stackedWidget->setCurrentWidget(ui->patientInfoPage);
    ui->createPatientLayout->removeWidget(createPatienComonent);
    delete createPatienComonent;
    createPatienComonent = nullptr;
    showPage(1);
}

void CPatientAdminPage::slotImport() {
}

void CPatientAdminPage::slotExport() {

}

void CPatientAdminPage::slotSearch() {
}

void CPatientAdminPage::slotForward() {
    const auto curPage = ui->curPage->text().toInt();
    const auto allPage = ui->allPage->text().toInt();
    if (curPage >= allPage) return;

    showPage(curPage + 1);
}

void CPatientAdminPage::slotBack() {
    const auto curPage = ui->curPage->text().toInt();
    auto allPage = ui->allPage->text().toInt();
    if (curPage <= 1) return;

    showPage(curPage - 1);
}

void CPatientAdminPage::slotFastForward() {
    const auto curPage = ui->curPage->text().toInt();
    const auto allPage = ui->allPage->text().toInt();
    if (curPage == allPage) return;

    showPage(allPage);
}

void CPatientAdminPage::slotFastBack() {
    const auto curPage = ui->curPage->text().toInt();
    const auto allPage = ui->allPage->text().toInt();
    if (curPage == 1) return;

    showPage(1);
}

void CPatientAdminPage::slotPatientCardPressed(QString pressedPatientUid)
{
    ui->selectAll->setChecked(false);
    for (const auto& uid : patientCardMap.keys())
    {
        if (pressedPatientUid != uid)
        {
            patientCardMap[uid]->setSelected(false);
        }
    }
}

void CPatientAdminPage::slotPatientCardDoubleClicked(QString uid)
{
    if (editPatientComponent == nullptr) {
        editPatientComponent = new CPatientInfo(tr("Patient Admin"), true, uid, this);
        connect(editPatientComponent, SIGNAL(signalEditPatientExit(QString)), SLOT(slotEditPatientExit(QString)));
        connect(editPatientComponent, SIGNAL(signalPatientDeleted()), SLOT(slotPatientDeleted()));
    }
    ui->editPatientLayout->addWidget(editPatientComponent);
    ui->stackedWidget->setCurrentWidget(ui->editPatientpage);
}

void CPatientAdminPage::slotEditPatientExit(QString uid)
{
    if (patientCardMap.contains(uid))
    {
        const auto card = patientCardMap[uid];
        card->updatePatientInfo();

        ui->stackedWidget->setCurrentWidget(ui->patientInfoPage);
        ui->editPatientLayout->removeWidget(editPatientComponent);
        delete editPatientComponent;
        editPatientComponent = nullptr;
    }
}

void CPatientAdminPage::slotPatientDeleted()
{
    ui->stackedWidget->setCurrentWidget(ui->patientInfoPage);
    ui->editPatientLayout->removeWidget(editPatientComponent);
    delete editPatientComponent;
    editPatientComponent = nullptr;

    const auto curPage = ui->curPage->text().toInt();
    showPage(curPage);
}

void CPatientAdminPage::slotSelectAll()
{
    const bool select = ui->selectAll->isChecked();
    for (auto uid : patientCardMap.keys())
    {
        patientCardMap[uid]->setSelected(select);
    }
}
END_NX_NAMESPACE


