#include "CIllnessInfo.h"

BEGIN_NX_NAMESPACE

CIllnessInfo::CIllnessInfo(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CIllnessInfo())
{
    ui->setupUi(this);
}

CIllnessInfo::~CIllnessInfo()
{
    delete ui;
}

void CIllnessInfo::setInfo(const QString& info, const QString& date)
{
    ui->info->setText(info);
    ui->date->setText(date);
}

END_NX_NAMESPACE
