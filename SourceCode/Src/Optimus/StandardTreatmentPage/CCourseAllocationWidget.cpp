#include "CCourseAllocationWidget.h"
#include "ui_CCourseAllocationWidget.h"
#include "NxAppService/CStandardTreatmentService.h"
#include "NxEntity/StandardTreatment.h"
#include <QUuid>
#include <QDateTime>
#include <QScrollBar>
#include "../CNxMessageBox.h"
BEGIN_NX_NAMESPACE
CCourseAllocationWidget::CCourseAllocationWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CCourseAllocationWidget)
    , standardTreatmentService(new CStandardTreatmentService(currentPatientUid))
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    connectSignal();
    //ui->scrollArea->verticalScrollBar()->setFixedWidth(8);

    ui->scrollArea->verticalScrollBar()->setStyleSheet(
        "QScrollBar:vertical {"
        "    width: 8px;"
        "    background: #122963;"
        "    margin: 0px, 0px, 0px, 0px;"
        "    padding-top: 9px;"
        "    padding-bottom: 9px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    width: 8px;"
        "    background: rgb(77, 112, 155);"
        "    border-radius: 4px;"
        "    min-height: 20;"
        "}"
        "QScrollBar::add-line:vertical {"
        "    height: 9px; width: 8px;"
        "    border-image: url(:/images/a/3.png);"
        "    subcontrol-position: bottom;"
        "}"
        "QScrollBar::sub-line:vertical {"
        "    height: 9px; width: 8px;"
        "    border-image: url(:/images/a/1.png);"
        "    subcontrol-position: top;"
        "}"
        "QScrollBar::add-line:vertical:hover {"
        "    height: 9px; width: 8px;"
        "    border-image: url(:/images/a/4.png);"
        "    subcontrol-position: bottom;"
        "}"
        "QScrollBar::sub-line:vertical:hover {"
        "    height: 9px; width: 8px;"
        "    border-image: url(:/images/a/2.png);"
        "    subcontrol-position: top;"
        "}"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
        "    background: rgba(0, 0, 0, 10%);"
        "    border-radius: 4px;"
        "}"
    );



}

CCourseAllocationWidget::~CCourseAllocationWidget()
{
    delete ui;
}

void CCourseAllocationWidget::initWidget()
{

   standardTreatmentService->currentPatientUid = currentPatientUid;
   auto currentstandardTreatment= standardTreatmentService->getStandardTreatment(currentDiseaseName, currentStaging);
   standardTreatment = currentstandardTreatment;
   if (currentstandardTreatment.empty())
   {
       standardTreatmentService->currentPatientUid = "123";
       currentstandardTreatment= standardTreatmentService->getStandardTreatment(currentDiseaseName, currentStaging);
       standardTreatment = currentstandardTreatment;
   }
   for (auto standardTreatment: currentstandardTreatment)
   {
       switch (standardTreatment.TrainType)
       {
       case 0:
           if (standardTreatment.IsChecked)
           {
               ui->checkBox->setChecked(true);
           }       
           else
           {
               ui->checkBox->setChecked(false);
           }
           ui->actionsignalduration->setValue(standardTreatment.TrialTime);
           ui->actioncountsduration->setValue(standardTreatment.TrialCounts);
           ui->actiontimedruantion->setValue(standardTreatment.CountDown);
           ui->actionMin->setCurrentText(QString::number(standardTreatment.MinScore));
           ui->actionMax->setCurrentText(QString::number(standardTreatment.MaxScore));
           setAction(QString::fromStdString(standardTreatment.Action));
           break;
       case 1:
           if (standardTreatment.IsChecked)
           {
               ui->click->setChecked(true);
           }
           else
           {
               ui->click->setChecked(false);
           }
           ui->clickduration->setValue(standardTreatment.TrialTime);
           ui->clickcountsduration->setValue(standardTreatment.TrialCounts);
           ui->clickMin->setCurrentText(QString::number(standardTreatment.MinScore));
           ui->clickMax->setCurrentText(QString::number(standardTreatment.MaxScore));
           ui->clickDiffMin->setCurrentText(QString::number(standardTreatment.Difficult));
           ui->clickDiffMax->setCurrentText(QString::number(standardTreatment.MaxDifficult));
           break;
       case 2:
           if (standardTreatment.IsChecked)
           {
               ui->direction->setChecked(true);
           }
           else
           {
               ui->direction->setChecked(false);
           }
           ui->directionsignalduration->setValue(standardTreatment.TrialTime);
           ui->directionountsduration->setValue(standardTreatment.TrialCounts);
           ui->directionMin->setCurrentText(QString::number(standardTreatment.MinScore));
           ui->directionMax->setCurrentText(QString::number(standardTreatment.MaxScore));
           ui->directiontimedruantion->setValue(standardTreatment.CountDown);
           ui->directionDiffMin->setCurrentText(QString::number(standardTreatment.Difficult));
           ui->directionDiffMax->setCurrentText(QString::number(standardTreatment.MaxDifficult));
         
           break;
       case 3:
           if (standardTreatment.IsChecked)
           {
               ui->word->setChecked(true);
           }
           else
           {
               ui->word->setChecked(false);
           }
           ui->wordsignalduration->setValue(standardTreatment.TrialTime);
           ui->wordcountsduration->setValue(standardTreatment.TrialCounts);
           ui->wordMin->setCurrentText(QString::number(standardTreatment.MinScore));
           ui->wordMax->setCurrentText(QString::number(standardTreatment.MaxScore));
           ui->wordtimedruantion->setValue(standardTreatment.CountDown);

           break;
       case 4:
           if (standardTreatment.IsChecked)
           {
               ui->software->setChecked(true);
           }
           else
           {
               ui->software->setChecked(false);
           }
           ui->softwaresignalduration->setValue(standardTreatment.TrialTime);
           ui->softwarecountsduration->setValue(standardTreatment.TrialCounts);
           ui->softwareMin->setCurrentText(QString::number(standardTreatment.MinScore));
           ui->softwareMax->setCurrentText(QString::number(standardTreatment.MaxScore));
           ui->softwaretimedruantion->setValue(standardTreatment.CountDown);

           break;
       default:
           break;
       }
   }
   isUpdate();
   ui->save->setEnabled(false);
}

void CCourseAllocationWidget::setAction(QString  action)
{
    QStringList actionList = action.split(",");
    if (actionList.contains("1"))
    {
        ui->left1->setChecked(true);
    }
    else
    {
        ui->left1->setChecked(false);
    }
    if (actionList.contains("2"))
    {
        ui->left2->setChecked(true);
    }
    else
    {
        ui->left2->setChecked(false);
    }
    if (actionList.contains("3"))
    {
        ui->left3->setChecked(true);
    }
    else
    {
        ui->left3->setChecked(false);
    }
    if (actionList.contains("4"))
    {
        ui->left4->setChecked(true);
    }
    else
    {
        ui->left4->setChecked(false);
    }
    if (actionList.contains("5"))
    {
        ui->left5->setChecked(true);
    }
    else
    {
        ui->left5->setChecked(false);
    }
    if (actionList.contains("6"))
    {
        ui->left6->setChecked(true);
    }
    else
    {
        ui->left6->setChecked(false);
    }
    if (actionList.contains("7"))
    {
        ui->left7->setChecked(true);
    }
    else
    {
        ui->left7->setChecked(false);
    }
    if (actionList.contains("8"))
    {
        ui->left8->setChecked(true);
    }
    else
    {
        ui->left8->setChecked(false);
    }
    if (actionList.contains("9"))
    {
        ui->left9->setChecked(true);
    }
    else
    {
        ui->left9->setChecked(false);
    }
    if (actionList.contains("10"))
    {
        ui->right1->setChecked(true);
    }
    else
    {
        ui->right1->setChecked(false);
    }
    if (actionList.contains("11"))
    {
        ui->rigth2->setChecked(true);
    }
    else
    {
        ui->rigth2->setChecked(false);
    }
    if (actionList.contains("12"))
    {
        ui->right3->setChecked(true);
    }
    else
    {
        ui->right3->setChecked(false);
    }
    if (actionList.contains("13"))
    {
        ui->right4->setChecked(true);
    }
    else
    {
        ui->right4->setChecked(false);
    }
    if (actionList.contains("14"))
    {
        ui->right5->setChecked(true);
    }
    else
    {
        ui->right5->setChecked(false);
    }
    if (actionList.contains("15"))
    {
        ui->right6->setChecked(true);
    }
    else
    {
        ui->right6->setChecked(false);
    }
    if (actionList.contains("16"))
    {
        ui->right7->setChecked(true);
    }
    else
    {
        ui->right7->setChecked(false);
    }
    if (actionList.contains("17"))
    {
        ui->right8->setChecked(true);
    }
    else
    {
        ui->right8->setChecked(false);
    }
    if (actionList.contains("18"))
    {
        ui->right9->setChecked(true);
    }
    else
    {
        ui->right9->setChecked(false);
    }
}

void CCourseAllocationWidget::updateAction(StandardTreatment& currentStandardTreatment)
{
    QString action;
    if (ui->left1->isChecked())
    {
        action.append("1,");
    }
    if (ui->left2->isChecked())
    {
        action.append("2,");
    }
    if (ui->left3->isChecked())
    {
        action.append("3,");
    }
    if (ui->left4->isChecked())
    {
        action.append("4,");
    }
    if (ui->left5->isChecked())
    {
        action.append("5,");
    }
    if (ui->left6->isChecked())
    {
        action.append("6,");
    }
    if (ui->left7->isChecked())
    {
        action.append("7,");
    }
    if (ui->left8->isChecked())
    {
        action.append("8,");
    }
    if (ui->left9->isChecked())
    {
        action.append("9,");
    }
    if (ui->right1->isChecked())
    {
        action.append("10,");
    }
    if (ui->rigth2->isChecked())
    {
        action.append("11,");
    }
    if (ui->right3->isChecked())
    {
        action.append("12,");
    }
    if (ui->right4->isChecked())
    {
        action.append("13,");
    }
    if (ui->right5->isChecked())
    {
        action.append("14,");
    }
    if (ui->right6->isChecked())
    {
        action.append("15,");
    }
    if (ui->right7->isChecked())
    {
        action.append("16,");
    }
    if (ui->right8->isChecked())
    {
        action.append("17,");
    }
    if (ui->right9->isChecked())
    {
        action.append("18");
    } 
    currentStandardTreatment.Action = action.toStdString();
}

void CCourseAllocationWidget::setPatientInfo(QString CurrentPatientUid, int CurrentDiseaseName, int CurrentStaging)
{
    currentPatientUid= CurrentPatientUid;
    currentDiseaseName= CurrentDiseaseName;
    currentStaging= CurrentStaging;
}

void CCourseAllocationWidget::setTitle(const QString& title)
{
    ui->title->setText(title);
}


void CCourseAllocationWidget::slotActionSignalDurationValueChanged(int value)
{
    ui->actionsignallb->setText(QString::number(value)+tr("min"));
    isUpdate();
}


void CCourseAllocationWidget::connectSignal()
{
    connect(ui->actionsignalduration, &QSlider::valueChanged, this, &CCourseAllocationWidget::slotActionSignalDurationValueChanged);
    connect(ui->actioncountsduration, &QSlider::valueChanged, this, &CCourseAllocationWidget::slotActionCountsDurationValueChanged);
    connect(ui->actiontimedruantion, &QSlider::valueChanged, this, &CCourseAllocationWidget::sloactionTimeDruantionValueChanged);
    connect(ui->clickduration, &QSlider::valueChanged, this, &CCourseAllocationWidget::slotClickDurationValueChanged);
    connect(ui->clickcountsduration, &QSlider::valueChanged, this, &CCourseAllocationWidget::slotClickCountsDuratiValueChanged);
    connect(ui->directionsignalduration, &QSlider::valueChanged, this, &CCourseAllocationWidget::slotDirectionSignalDuratiValueChanged);
    connect(ui->directionountsduration, &QSlider::valueChanged, this, &CCourseAllocationWidget::slotDirectionCountsDuratiValueChanged);
    connect(ui->directiontimedruantion, &QSlider::valueChanged, this, &CCourseAllocationWidget::slotDirectionTimeDruantiValueChanged);
    connect(ui->wordsignalduration, &QSlider::valueChanged, this, &CCourseAllocationWidget::slotWordSignalDuratiValueChanged);
    connect(ui->wordcountsduration, &QSlider::valueChanged, this, &CCourseAllocationWidget::slotWordCountsDuratiValueChanged);

    connect(ui->wordtimedruantion, &QSlider::valueChanged, this, &CCourseAllocationWidget::slotWordTimeDruantiValueChanged);
    connect(ui->softwaresignalduration, &QSlider::valueChanged, this, &CCourseAllocationWidget::slotSoftWareSignalDuratiValueChanged);
    connect(ui->softwarecountsduration, &QSlider::valueChanged, this, &CCourseAllocationWidget::slotSoftWareCountsDuratiValueChanged);
    connect(ui->softwaretimedruantion, &QSlider::valueChanged, this, &CCourseAllocationWidget::slotSoftWareTimeDruantiValueChanged);
    connect(ui->actionMin, &QComboBox::currentTextChanged, this, &CCourseAllocationWidget::slotActionMinTextChanged);
    connect(ui->actionMax, &QComboBox::currentTextChanged, this, &CCourseAllocationWidget::slotActionMaxTextChanged);
    connect(ui->clickDiffMin, &QComboBox::currentTextChanged, this, &CCourseAllocationWidget::slotClickDiffMinTextChanged);
    connect(ui->clickDiffMax, &QComboBox::currentTextChanged, this, &CCourseAllocationWidget::slotClickDiffMaxTextChanged);
    connect(ui->clickMin, &QComboBox::currentTextChanged, this, &CCourseAllocationWidget::slotClickMinTextChanged);
    connect(ui->clickMax, &QComboBox::currentTextChanged, this, &CCourseAllocationWidget::slotClickMaxTextChanged);
    connect(ui->directionMin, &QComboBox::currentTextChanged, this, &CCourseAllocationWidget::slotDirectionMinTextChanged);
    connect(ui->directionMax, &QComboBox::currentTextChanged, this, &CCourseAllocationWidget::slotDirectionMaxTextChanged);
    connect(ui->wordMin, &QComboBox::currentTextChanged, this, &CCourseAllocationWidget::slotWordMinTextChanged);
    connect(ui->directionDiffMin, &QComboBox::currentTextChanged, this, &CCourseAllocationWidget::isUpdate);
    connect(ui->directionDiffMax, &QComboBox::currentTextChanged, this, &CCourseAllocationWidget::isUpdate);
    connect(ui->wordMin, &QComboBox::currentTextChanged, this, &CCourseAllocationWidget::slotWordMinTextChanged);
    connect(ui->wordMax, &QComboBox::currentTextChanged, this, &CCourseAllocationWidget::slotWordMaxTextChanged);
    connect(ui->softwareMin, &QComboBox::currentTextChanged, this, &CCourseAllocationWidget::slotSoftWareMinTextChanged);
    connect(ui->softwareMax, &QComboBox::currentTextChanged, this, &CCourseAllocationWidget::slotSoftWareMaxTextChanged);
    connect(ui->checkBox, &QCheckBox::stateChanged, this, &CCourseAllocationWidget::slotActionCheackStateChanged);
    connect(ui->click, &QCheckBox::stateChanged, this, &CCourseAllocationWidget::slotClickCheackStateChanged);
    connect(ui->direction, &QCheckBox::stateChanged, this, &CCourseAllocationWidget::slotDirectionCheackStateChanged);
    connect(ui->word, &QCheckBox::stateChanged, this, &CCourseAllocationWidget::slotWordCheackStateChanged);
    connect(ui->software, &QCheckBox::stateChanged, this, &CCourseAllocationWidget::slotSoftWareCheackStateChanged);
    connect(ui->left1, &QCheckBox::stateChanged, this, &CCourseAllocationWidget::slotLeft1CheackStateChanged);
    connect(ui->left2, &QCheckBox::stateChanged, this, &CCourseAllocationWidget::slotLeft2CheackStateChanged);
    connect(ui->left3, &QCheckBox::stateChanged, this, &CCourseAllocationWidget::slotLeft3CheackStateChanged);
    connect(ui->left4, &QCheckBox::stateChanged, this, &CCourseAllocationWidget::slotLeft4CheackStateChanged);
    connect(ui->left5, &QCheckBox::stateChanged, this, &CCourseAllocationWidget::slotLeft5CheackStateChanged);
    connect(ui->left6, &QCheckBox::stateChanged, this, &CCourseAllocationWidget::slotLeft6CheackStateChanged);
    connect(ui->left7, &QCheckBox::stateChanged, this, &CCourseAllocationWidget::slotLeft7CheackStateChanged);
    connect(ui->left8, &QCheckBox::stateChanged, this, &CCourseAllocationWidget::slotLeft8CheackStateChanged);
    connect(ui->left9, &QCheckBox::stateChanged, this, &CCourseAllocationWidget::slotLeft9CheackStateChanged);
    connect(ui->right1, &QCheckBox::stateChanged, this, &CCourseAllocationWidget::slotRight1CheackStateChanged);
    connect(ui->rigth2, &QCheckBox::stateChanged, this, &CCourseAllocationWidget::slotRight2CheackStateChanged);
    connect(ui->right3, &QCheckBox::stateChanged, this, &CCourseAllocationWidget::slotRight3CheackStateChanged);
    connect(ui->right4, &QCheckBox::stateChanged, this, &CCourseAllocationWidget::slotRight4CheackStateChanged);
    connect(ui->right5, &QCheckBox::stateChanged, this, &CCourseAllocationWidget::slotRight5CheackStateChanged);
    connect(ui->right6, &QCheckBox::stateChanged, this, &CCourseAllocationWidget::slotRight6CheackStateChanged);
    connect(ui->right7, &QCheckBox::stateChanged, this, &CCourseAllocationWidget::slotRight7CheackStateChanged);
    connect(ui->right8, &QCheckBox::stateChanged, this, &CCourseAllocationWidget::slotRight8CheackStateChanged);
    connect(ui->right9, &QCheckBox::stateChanged, this, &CCourseAllocationWidget::slotRight9CheackStateChanged);
    connect(ui->save, &QPushButton::clicked, this, &CCourseAllocationWidget::slotSaveStandardTreatment);
    connect(ui->reset, &QPushButton::clicked, this, &CCourseAllocationWidget::slotResetStandardTreatment);
}

void CCourseAllocationWidget::actionIsUpdate()
{
    StandardTreatment treatment;
    for (auto standardtreatment : standardTreatment)
    {
        switch (standardtreatment.TrainType)
        {
        case 0:
    
            if (ui->checkBox->isChecked())
            {
                treatment.IsChecked = 1;
            }
            else
            {
                treatment.IsChecked = 0;
            }
            updateAction(treatment);
          
            if (standardtreatment.Action== treatment.Action&& standardtreatment.TrialTime==ui->actionsignalduration->value()&& standardtreatment.TrialCounts ==ui->actioncountsduration->value()&& standardtreatment.CountDown==ui->actiontimedruantion->value() && standardtreatment.MinScore == ui->actionMin->currentText().toInt() &&
                standardtreatment.MaxScore == ui->actionMax->currentText().toInt()&& treatment.IsChecked== standardtreatment.IsChecked)
            {
                actionUpdate = false;
            }
            else
            {
                actionUpdate = true;
            }

            break;
        default:
            break;
        }

    }
}

void CCourseAllocationWidget::clickIsUpdate()
{
    StandardTreatment treatment;
    for (auto standardtreatment : standardTreatment)
    {
        switch (standardtreatment.TrainType)
        {
        case 1:

            if (ui->click->isChecked())
            {
                treatment.IsChecked = 1;
            }
            else
            {
                treatment.IsChecked = 0;
            }
      
            if ( standardtreatment.TrialTime == ui->clickduration->value() && standardtreatment.TrialCounts == ui->clickcountsduration->value()  && standardtreatment.MinScore == ui->clickMin->currentText().toInt() &&
                standardtreatment.MaxScore == ui->clickMax->currentText().toInt() && treatment.IsChecked == standardtreatment.IsChecked&& standardtreatment.Difficult == ui->clickDiffMin->currentText().toInt()&&
                standardtreatment.MaxDifficult == ui->clickDiffMax->currentText().toInt())
            {
                clickUpdate = false;
            }
            else
            {
                clickUpdate = true;
            }
            break;
        default:
            break;
        }

    }
}

void CCourseAllocationWidget::directionIsUpdate()
{
    StandardTreatment treatment;
    for (auto standardtreatment : standardTreatment)
    {
        switch (standardtreatment.TrainType)
        {
        case 2:

            if (ui->direction->isChecked())
            {
                treatment.IsChecked = 1;
            }
            else
            {
                treatment.IsChecked = 0;
            }

            if (standardtreatment.TrialTime == ui->directionsignalduration->value() && standardtreatment.TrialCounts == ui->directionountsduration->value() && standardtreatment.CountDown == ui->directiontimedruantion->value() && standardtreatment.MinScore == ui->directionMin->currentText().toInt() &&
                standardtreatment.MaxScore == ui->directionMax->currentText().toInt() && treatment.IsChecked == standardtreatment.IsChecked&& standardtreatment.MaxDifficult==ui->directionDiffMax->currentText().toInt()&& standardtreatment.Difficult == ui->directionDiffMin->currentText().toInt())
            {
                directionUpdate = false;
            }
            else
            {
                directionUpdate = true;
            }
            break;
        default:
            break;
        }

    }
}

void CCourseAllocationWidget::wordIsUpdate()
{
    StandardTreatment treatment;
    for (auto standardtreatment : standardTreatment)
    {
        switch (standardtreatment.TrainType)
        {
        case 3:

            if (ui->word->isChecked())
            {
                treatment.IsChecked = 1;
            }
            else
            {
                treatment.IsChecked = 0;
            }

            if (standardtreatment.TrialTime == ui->wordsignalduration->value() && standardtreatment.TrialCounts == ui->wordcountsduration->value() && standardtreatment.CountDown == ui->wordtimedruantion->value() && standardtreatment.MinScore == ui->wordMin->currentText().toInt() &&
                standardtreatment.MaxScore == ui->wordMax->currentText().toInt() && treatment.IsChecked == standardtreatment.IsChecked)
            {
                wordUpdate = false;
            }
            else
            {
                wordUpdate = true;
            }
            break;
        default:
            break;
        }

    }


}

void CCourseAllocationWidget::softwareIsUpdate()
{
    StandardTreatment treatment;
  
 
    for (auto standardtreatment : standardTreatment)
    {
        switch (standardtreatment.TrainType)
        {
        case 4:

            if (ui->software->isChecked())
            {
                treatment.IsChecked = 1;
            }
            else
            {
                treatment.IsChecked = 0;
            }

            if (standardtreatment.TrialTime == ui->softwaresignalduration->value() && standardtreatment.TrialCounts == ui->softwarecountsduration->value() && standardtreatment.CountDown == ui->softwaretimedruantion->value() && standardtreatment.MinScore == ui->softwareMin->currentText().toInt() &&
                standardtreatment.MaxScore == ui->softwareMax->currentText().toInt() && treatment.IsChecked == standardtreatment.IsChecked)
            {
                softwareUpdate = false;
            }
            else
            {
               softwareUpdate = true;
            }
            break;
        default:
            break;
        }

    }
}

void CCourseAllocationWidget::isUpdate()
{
     softwareIsUpdate();
     actionIsUpdate();
     clickIsUpdate();
     directionIsUpdate();
     wordIsUpdate();
     if (softwareUpdate|| wordUpdate||directionUpdate || clickUpdate||actionUpdate)
     {
         ui->save->setEnabled(true);
     }
     else
     {
         ui->save->setEnabled(false);
     }

}

void CCourseAllocationWidget::slotActionCountsDurationValueChanged(int value)
{
    ui->actioncountslb->setText(QString::number(value)+QString::fromLocal8Bit("次"));
    isUpdate();
}


void CCourseAllocationWidget::sloactionTimeDruantionValueChanged(int value)
{
     ui->actiontimelb->setText(QString::number(value)+tr("s"));
     isUpdate();
}


void CCourseAllocationWidget::slotClickDurationValueChanged(int value)
{
    ui->clicksignallb->setText(QString::number(value)+tr("min"));
    isUpdate();
}



void CCourseAllocationWidget::slotClickCountsDuratiValueChanged(int value)
{
        ui->clickcountslb->setText(QString::number(value)+tr("count"));
        isUpdate();
}


void CCourseAllocationWidget::slotDirectionSignalDuratiValueChanged(int value)
{
     ui->directionssignallb->setText(QString::number(value)+tr("min"));
     isUpdate();
}


void CCourseAllocationWidget::slotDirectionCountsDuratiValueChanged(int value)
{
   ui->directionountslb->setText(QString::number(value)+tr("conut"));
   isUpdate();
}


void CCourseAllocationWidget::slotDirectionTimeDruantiValueChanged(int value)
{
       ui->directiontimelb->setText(QString::number(value)+tr("s"));
       isUpdate();
}


void CCourseAllocationWidget::slotWordSignalDuratiValueChanged(int value)
{
      ui->wordsignallb->setText(QString::number(value)+tr("min"));
      isUpdate();
}


void CCourseAllocationWidget::slotWordCountsDuratiValueChanged(int value)
{
     ui->wordcountslb->setText(QString::number(value)+tr("count"));
     isUpdate();
}


void CCourseAllocationWidget::slotWordTimeDruantiValueChanged(int value)
{
   ui->wordtimelb->setText(QString::number(value)+tr("s"));
   isUpdate();
}


void CCourseAllocationWidget::slotSoftWareSignalDuratiValueChanged(int value)
{
    ui->softwaresignallb->setText(QString::number(value)+tr("min"));
    isUpdate();
}


void CCourseAllocationWidget::slotSoftWareCountsDuratiValueChanged(int value)
{
    ui->softwarecountslb->setText(QString::number(value)+tr("count"));
    isUpdate();
}


void CCourseAllocationWidget::slotSoftWareTimeDruantiValueChanged(int value)
{
    ui->softwaretimelb->setText(QString::number(value)+tr("s"));
    isUpdate();
}

void CCourseAllocationWidget::slotActionMinTextChanged(QString value)
{
    isUpdate();
}

void CCourseAllocationWidget::slotActionMaxTextChanged(QString value)
{
    isUpdate();
}

void CCourseAllocationWidget::slotClickDiffMinTextChanged(QString value)
{
    isUpdate();
}

void CCourseAllocationWidget::slotClickDiffMaxTextChanged(QString value)
{
    isUpdate();
}

void CCourseAllocationWidget::slotClickMinTextChanged(QString value)
{
    isUpdate();
}

void CCourseAllocationWidget::slotClickMaxTextChanged(QString value)
{
    isUpdate();
}

void CCourseAllocationWidget::slotDirectionMinTextChanged(QString value)
{
    isUpdate();
}

void CCourseAllocationWidget::slotDirectionMaxTextChanged(QString value)
{
    isUpdate();
}

void CCourseAllocationWidget::slotWordMinTextChanged(QString value)
{
    isUpdate();
}

void CCourseAllocationWidget::slotWordMaxTextChanged(QString value)
{
    isUpdate();
}

void CCourseAllocationWidget::slotSoftWareMinTextChanged(QString value)
{
    isUpdate();
}

void CCourseAllocationWidget::slotSoftWareMaxTextChanged(QString value)
{
    isUpdate();
}

void CCourseAllocationWidget::slotActionCheackStateChanged()
{
    isUpdate();
}

void CCourseAllocationWidget::slotClickCheackStateChanged()
{
    isUpdate();
}

void CCourseAllocationWidget::slotDirectionCheackStateChanged()
{
    isUpdate();
}

void CCourseAllocationWidget::slotWordCheackStateChanged()
{
    isUpdate();
}

void CCourseAllocationWidget::slotSoftWareCheackStateChanged()
{
    isUpdate();
}

void CCourseAllocationWidget::slotLeft1CheackStateChanged()
{
    isUpdate();
}

void CCourseAllocationWidget::slotLeft2CheackStateChanged()
{
    isUpdate();
}

void CCourseAllocationWidget::slotLeft3CheackStateChanged()
{
    isUpdate();
}

void CCourseAllocationWidget::slotLeft4CheackStateChanged()
{
    isUpdate();
}

void CCourseAllocationWidget::slotLeft5CheackStateChanged()
{
    isUpdate();
}

void CCourseAllocationWidget::slotLeft6CheackStateChanged()
{
    isUpdate();
}

void CCourseAllocationWidget::slotLeft7CheackStateChanged()
{
    isUpdate();
}

void CCourseAllocationWidget::slotLeft8CheackStateChanged()
{
    isUpdate();
}

void CCourseAllocationWidget::slotLeft9CheackStateChanged()
{
    isUpdate();
}

void CCourseAllocationWidget::slotRight1CheackStateChanged()
{
    isUpdate();
}

void CCourseAllocationWidget::slotRight2CheackStateChanged()
{
    isUpdate();
}

void CCourseAllocationWidget::slotRight3CheackStateChanged()
{
    isUpdate();
}

void CCourseAllocationWidget::slotRight4CheackStateChanged()
{
    isUpdate();
}

void CCourseAllocationWidget::slotRight5CheackStateChanged()
{
    isUpdate();
}

void CCourseAllocationWidget::slotRight6CheackStateChanged()
{
    isUpdate();
}

void CCourseAllocationWidget::slotRight7CheackStateChanged()
{
    isUpdate();
}

void CCourseAllocationWidget::slotRight8CheackStateChanged()
{
    isUpdate();
}

void CCourseAllocationWidget::slotRight9CheackStateChanged()
{
    isUpdate();
}

void CCourseAllocationWidget::slotSaveStandardTreatment()
{
    standardTreatmentService->currentPatientUid = currentPatientUid;
    auto currentstandardTreatment = standardTreatmentService->getStandardTreatment(currentDiseaseName, currentStaging);
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString formattedDateTime = currentDateTime.toString("yyyy-MM-dd hh:mm:ss");
    if (currentstandardTreatment.empty())
    {
        for (int i=0;i<4;i++)
        {
            StandardTreatment currentStandardTreatment;
            currentStandardTreatment.UID= QUuid::createUuid().toString().replace("{", "").replace("}", "").toStdString();
            currentStandardTreatment.IsDeleted = 0;
            currentStandardTreatment.TrainType = i;
            currentStandardTreatment.CreateDateTime= formattedDateTime.toStdString();
            currentStandardTreatment.UpdateDateTime = formattedDateTime.toStdString();
            currentStandardTreatment.DiseaseName = currentDiseaseName;
            currentStandardTreatment.Staging = currentStaging;
            currentStandardTreatment.PatientFK = currentPatientUid.toStdString();
            currentStandardTreatment.Difficult = 60;
            currentStandardTreatment.MaxScore = 100;
            currentStandardTreatment.MaxDifficult = 100;
            currentStandardTreatment.MinScore = 60;
            currentstandardTreatment.push_back(currentStandardTreatment);
        }

    }
    for (auto standardTreatment : currentstandardTreatment)
    {
        switch (standardTreatment.TrainType)
        {
        case 0:
            if (ui->checkBox->isChecked())
            {
                standardTreatment.IsChecked = 1;
            }
     
            else
            {
                standardTreatment.IsChecked = 0;
            }
            standardTreatment.TrialTime = ui->actionsignalduration->value();
            standardTreatment.TrialCounts = ui->actioncountsduration->value();
            standardTreatment.CountDown = ui->actiontimedruantion->value();
            updateAction(standardTreatment);
            standardTreatment.MinScore = ui->actionMin->currentText().toInt();
            standardTreatment.MaxScore = ui->actionMax->currentText().toInt();

            standardTreatmentService->updateStandardTreatment(standardTreatment);
            break;
        case 1:

            if (ui->click->isChecked())
            {
                standardTreatment.IsChecked = 1;
            }

            else
            {
                standardTreatment.IsChecked = 0;
            }
            standardTreatment.TrialTime = ui->clickduration->value();
            standardTreatment.TrialCounts = ui->clickcountsduration->value();
            standardTreatment.MinScore = ui->clickMin->currentText().toInt();
            standardTreatment.MaxScore = ui->clickMax->currentText().toInt();
            standardTreatment.Difficult = ui->clickDiffMin->currentText().toInt();
            standardTreatment.MaxDifficult = ui->clickDiffMax->currentText().toInt();
            standardTreatmentService->updateStandardTreatment(standardTreatment);
            break;
        case 2:
            if (ui->direction->isChecked())
            {
                standardTreatment.IsChecked = 1;
            }

            else
            {
                standardTreatment.IsChecked = 0;
            }
            standardTreatment.TrialTime = ui->directionsignalduration->value();
            standardTreatment.TrialCounts = ui->directionountsduration->value();
            standardTreatment.MinScore = ui->directionMin->currentText().toInt();
            standardTreatment.MaxScore = ui->directionMax->currentText().toInt();
            standardTreatment.CountDown = ui->directiontimedruantion->value();
            standardTreatment.Difficult = ui->directionDiffMin->currentText().toInt();
            standardTreatment.MaxDifficult = ui->directionDiffMax->currentText().toInt();
            standardTreatmentService->updateStandardTreatment(standardTreatment);
            break;
        case 3:
            if (ui->word->isChecked())
            {
                standardTreatment.IsChecked = 1;
            }
            else
            {
                standardTreatment.IsChecked = 0;
            }
            standardTreatment.TrialTime = ui->wordsignalduration->value();
            standardTreatment.TrialCounts = ui->wordcountsduration->value();
            standardTreatment.MinScore = ui->wordMin->currentText().toInt();
            standardTreatment.MaxScore = ui->wordMax->currentText().toInt();
            standardTreatment.CountDown = ui->wordtimedruantion->value();
            standardTreatmentService->updateStandardTreatment(standardTreatment);
            break;
        case 4:

            if (ui->software->isChecked())
            {
                standardTreatment.IsChecked = 1;
            }
            else
            {
                standardTreatment.IsChecked = 0;
            }
            standardTreatment.TrialTime = ui->softwaresignalduration->value();
            standardTreatment.TrialCounts = ui->softwarecountsduration->value();
            standardTreatment.MinScore = ui->softwareMin->currentText().toInt();
            standardTreatment.MaxScore = ui->softwareMax->currentText().toInt();
            standardTreatment.CountDown = ui->softwaretimedruantion->value();
            standardTreatmentService->updateStandardTreatment(standardTreatment);
            break;
        default:
            break;
        }
    }
    initWidget();
}

void CCourseAllocationWidget::slotResetStandardTreatment()
{
    CNxMessageBox msgBox(QString::fromLocal8Bit(""), tr("Reset to standard treatment?"));
    msgBox.exec();
    bool confirmed = msgBox.isConfirmed();
    if (confirmed)
    {
        if (isStandStreatment)
        {
            standardTreatmentService->currentPatientUid = currentPatientUid;
            auto currentstandardTreatment = standardTreatmentService->getStandardTreatment(currentDiseaseName, currentStaging);
            for (auto standardTreatment : currentstandardTreatment)
            {
                switch (standardTreatment.TrainType)
                {
                case 0:
                    standardTreatment.IsChecked = 0;
                    standardTreatment.TrialTime = 10;
                    standardTreatment.TrialCounts = 10;
                    standardTreatment.CountDown = 5;
                    standardTreatment.MinScore = 60;
                    standardTreatment.MaxScore = 100;
                    standardTreatment.Action = "1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18";
                    standardTreatmentService->updateStandardTreatment(standardTreatment);
                    break;
                case 1:

                    standardTreatment.IsChecked = 0;
                    standardTreatment.TrialTime = 10;
                    standardTreatment.TrialCounts = 10;
                    standardTreatment.Difficult = 10;
                    standardTreatment.MinScore = 60;
                    standardTreatment.MaxScore = 100;
                    standardTreatment.MaxDifficult = 100;
                    standardTreatmentService->updateStandardTreatment(standardTreatment);
                    break;
                case 2:
                    standardTreatment.IsChecked = 0;
                    standardTreatment.TrialTime = 10;
                    standardTreatment.TrialCounts = 10;
                    standardTreatment.Difficult = 10;
                    standardTreatment.MinScore = 60;
                    standardTreatment.MaxScore = 100;
                    standardTreatment.MaxDifficult = 100;
                    standardTreatment.CountDown = 5;
                    standardTreatmentService->updateStandardTreatment(standardTreatment);
                    break;
                case 3:
                    standardTreatment.IsChecked = 0;
                    standardTreatment.TrialTime = 10;
                    standardTreatment.TrialCounts = 10;
                    standardTreatment.Difficult = 10;
                    standardTreatment.MinScore = 60;
                    standardTreatment.MaxScore = 100;
                    standardTreatment.MaxDifficult = 100;
                    standardTreatment.CountDown = 5;
                    standardTreatmentService->updateStandardTreatment(standardTreatment);
                    break;
                case 4:
                    standardTreatment.IsChecked = 0;
                    standardTreatment.TrialTime = 10;
                    standardTreatment.TrialCounts = 10;
                    standardTreatment.Difficult = 10;
                    standardTreatment.MinScore = 60;
                    standardTreatment.MaxScore = 100;
                    standardTreatment.MaxDifficult = 100;
                    standardTreatment.CountDown = 5;
                    standardTreatmentService->updateStandardTreatment(standardTreatment);
                    break;
                default:
                    break;
                }
            }
            initWidget();
        }
        else
        {
          
                  auto currentstandardTreatment = standardTreatmentService->getStandardTreatment(currentDiseaseName, currentStaging);
                  standardTreatment = currentstandardTreatment;
                  standardTreatmentService->currentPatientUid = "123";
                  currentstandardTreatment = standardTreatmentService->getStandardTreatment(currentDiseaseName, currentStaging);
                  standardTreatment = currentstandardTreatment;
        
            for (auto standardTreatment : currentstandardTreatment)
            {
                switch (standardTreatment.TrainType)
                {
                case 0:
                    if (standardTreatment.IsChecked)
                    {
                        ui->checkBox->setChecked(true);
                    }
                    else
                    {
                        ui->checkBox->setChecked(false);
                    }
                    ui->actionsignalduration->setValue(standardTreatment.TrialTime);
                    ui->actioncountsduration->setValue(standardTreatment.TrialCounts);
                    ui->actiontimedruantion->setValue(standardTreatment.CountDown);
                    ui->actionMin->setCurrentText(QString::number(standardTreatment.MinScore));
                    ui->actionMax->setCurrentText(QString::number(standardTreatment.MaxScore));
                    setAction(QString::fromStdString(standardTreatment.Action));
                    break;
                case 1:
                    if (standardTreatment.IsChecked)
                    {
                        ui->click->setChecked(true);
                    }
                    else
                    {
                        ui->click->setChecked(false);
                    }
                    ui->clickduration->setValue(standardTreatment.TrialTime);
                    ui->clickcountsduration->setValue(standardTreatment.TrialCounts);
                    ui->clickMin->setCurrentText(QString::number(standardTreatment.MinScore));
                    ui->clickMax->setCurrentText(QString::number(standardTreatment.MaxScore));
                    ui->clickDiffMin->setCurrentText(QString::number(standardTreatment.Difficult));
                    ui->clickDiffMax->setCurrentText(QString::number(standardTreatment.MaxDifficult));
                    break;
                case 2:
                    if (standardTreatment.IsChecked)
                    {
                        ui->direction->setChecked(true);
                    }
                    else
                    {
                        ui->direction->setChecked(false);
                    }
                    ui->directionsignalduration->setValue(standardTreatment.TrialTime);
                    ui->directionountsduration->setValue(standardTreatment.TrialCounts);
                    ui->directionMin->setCurrentText(QString::number(standardTreatment.MinScore));
                    ui->directionMax->setCurrentText(QString::number(standardTreatment.MaxScore));
                    ui->directiontimedruantion->setValue(standardTreatment.CountDown);

                    break;
                case 3:
                    if (standardTreatment.IsChecked)
                    {
                        ui->word->setChecked(true);
                    }
                    else
                    {
                        ui->word->setChecked(false);
                    }
                    ui->wordsignalduration->setValue(standardTreatment.TrialTime);
                    ui->wordcountsduration->setValue(standardTreatment.TrialCounts);
                    ui->wordMin->setCurrentText(QString::number(standardTreatment.MinScore));
                    ui->wordMax->setCurrentText(QString::number(standardTreatment.MaxScore));
                    ui->wordtimedruantion->setValue(standardTreatment.CountDown);

                    break;
                case 4:
                    if (standardTreatment.IsChecked)
                    {
                        ui->software->setChecked(true);
                    }
                    else
                    {
                        ui->software->setChecked(false);
                    }
                    ui->softwaresignalduration->setValue(standardTreatment.TrialTime);
                    ui->softwarecountsduration->setValue(standardTreatment.TrialCounts);
                    ui->softwareMin->setCurrentText(QString::number(standardTreatment.MinScore));
                    ui->softwareMax->setCurrentText(QString::number(standardTreatment.MaxScore));
                    ui->softwaretimedruantion->setValue(standardTreatment.CountDown);

                    break;
                default:
                    break;
                }
            }
            slotSaveStandardTreatment();

        }
     
    }
    else
    {
        return;
    }
}

END_NX_NAMESPACE