#include "CSystemSettings.h"
#include "../UserManage/CUserInfo.h"
#include "NxAppService/CUserService.h"
#include "../CNxMessageBox.h"
#include "NxSystemCfg/CSystemCfg.h"
#include "CSwitchButton.h"
#include "../CProgressDialog.h"


#include "NxAppService/COscillographService.h"

#include <QButtonGroup>
#include <QProcess>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfoList>
#include <QDebug>
#include <QStorageInfo>
#include <QDateTime>
BEGIN_NX_NAMESPACE
CSystemSettings::CSystemSettings(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CSystemSettings())
    , userInfo(nullptr)
    , userService(new CUserService())
    , progressDialog(nullptr)
	, oscillographService(new COscillographService("127.0.0.1:50051"))
{
    ui->setupUi(this);
    // 创建两个按钮组
    QButtonGroup* Waveformbackgroundcolor = new QButtonGroup(this);

    QButtonGroup* Language = new QButtonGroup(this);
    gpuSwitchButton = new CSwitchButton(this);
    compressSwitchButton = new CSwitchButton(this);
    systemCfg = systemConfig.GetSystemCfg();
    initSysConfig();
    mysql_init(&mysqlInstance);
    mysql_real_connect(&mysqlInstance, systemCfg.Common.DBIp.c_str(), systemCfg.Common.DBUser.c_str(), systemCfg.Common.DBPassword.c_str(),systemCfg.Common.DBName.c_str(), systemCfg.Common.DBPort, 0, 0);
    Waveformbackgroundcolor->addButton(ui->dark);
    Waveformbackgroundcolor->addButton(ui->light);


    Language->addButton(ui->chinese);
    Language->addButton(ui->english);
    ui->userTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->userTable->setFocusPolicy(Qt::NoFocus);
    // 设置水平表头文本对齐方式为左对齐
    ui->userTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    ui->gpuhor->addWidget(gpuSwitchButton);
    ui->compresshor->addWidget(compressSwitchButton);
    connect(ui->backup, &QPushButton::clicked, this, &CSystemSettings::slotBackupClick);
    connect(ui->Restore, &QPushButton::clicked, this, &CSystemSettings::slotRestoreClick);
    connect(ui->UserManage, &QPushButton::clicked, this, &CSystemSettings::slotUserManageClick);
    connect(ui->HospitalInfo, &QPushButton::clicked, this, &CSystemSettings::slotHospitalInfoClick);
    connect(ui->Dropbox, &QPushButton::clicked, this, &CSystemSettings::slotDropboxClick);
    connect(ui->BackupRestore, &QPushButton::clicked, this, &CSystemSettings::slotBackupRestoreClick);
    connect(ui->Create, &QPushButton::clicked, this, &CSystemSettings::slotCreateUser);
    connect(ui->Edit, &QPushButton::clicked, this, &CSystemSettings::slotEditUser);
    connect(ui->Delete, &QPushButton::clicked, this, &CSystemSettings::slotDeleteUser);
    connect(ui->HospitalName, &QLineEdit::editingFinished, this, &CSystemSettings::updateSysConfig);
    connect(ui->DepartmentsName, &QLineEdit::editingFinished, this, &CSystemSettings::updateSysConfig);
    connect(ui->BackupAdress, &QLineEdit::editingFinished, this, &CSystemSettings::updateSysConfig);
    connect(ui->address, &QLineEdit::editingFinished, this, &CSystemSettings::updateSysConfig);
    connect(ui->port, &QLineEdit::editingFinished, this, &CSystemSettings::updateSysConfig);
    connect(ui->chinese, &QRadioButton::toggled, this, &CSystemSettings::updateSysConfig);
    connect(ui->english, &QRadioButton::toggled, this, &CSystemSettings::updateSysConfig);
    connect(ui->dark, &QRadioButton::toggled, this, &CSystemSettings::updateSysConfig);
    connect(ui->light, &QRadioButton::toggled, this, &CSystemSettings::updateSysConfig);
    connect(gpuSwitchButton, &CSwitchButton::statusChanged, this, &CSystemSettings::updateSysConfig);
    connect(compressSwitchButton, &CSwitchButton::statusChanged, this, &CSystemSettings::updateSysConfig);
    ui->UserManage->setCheckable(true);
    ui->HospitalInfo->setCheckable(true);   
    ui->BackupRestore->setCheckable(true);
    ui->Dropbox->setCheckable(true);
    ui->UserManage->click();
    ui->Delete->setIcon(QIcon(":/Optimus/images/common/delete.svg"));
    ui->Create->setIcon(QIcon(":/Optimus/images/common/create.svg"));

}

CSystemSettings::~CSystemSettings()
{
    delete userInfo;

}
void CSystemSettings::setCheck()
{
    auto pageBtns = { ui->UserManage, ui->HospitalInfo,ui->Dropbox,ui->BackupRestore};
    for (auto btn : pageBtns) {
        btn->setChecked(false);
    }
}
void CSystemSettings::initWidget()
{
    ui->userTable->setRowCount(0);
   auto allUser= userService->getAllUser();

   for (int i = 0; i < allUser.size(); i++)
   {
       auto rowCounts = ui->userTable->rowCount();
       ui->userTable->insertRow(rowCounts);
       auto currentInfo = userService->getUserIdentityInfo(QString::fromStdString(allUser[i].IdentifyFK));
       ui->userTable->setItem(rowCounts, 0, new QTableWidgetItem(QString::fromStdString(currentInfo.LoginName)));
       ui->userTable->setItem(rowCounts, 1, new QTableWidgetItem(QString::fromStdString(allUser[i].UserName)));
       ui->userTable->setItem(rowCounts, 2, new QTableWidgetItem(QString::fromStdString("******")));
       if (currentInfo.Role==0)
       {
           ui->userTable->setItem(rowCounts, 3, new QTableWidgetItem(tr("Doctor")));
       }
       else if(currentInfo.Role == 1)
       {
           ui->userTable->setItem(rowCounts, 3, new QTableWidgetItem(tr("Admin")));
       }
       else
       {
           ui->userTable->setItem(rowCounts, 3, new QTableWidgetItem(tr("ServiceEngineer")));
       }
     
       ui->userTable->setItem(rowCounts, 4, new QTableWidgetItem(QString::fromStdString(allUser[i].Departments)));

   }
   if (ui->userTable->rowCount() > 0)
   {
       ui->userTable->selectRow(0);
   }

}
void CSystemSettings::initSysConfig()
{
    ui->HospitalName->setText(QString::fromStdString(systemCfg.Hospital.HospitalName));
    ui->DepartmentsName->setText(QString::fromStdString(systemCfg.Hospital.Department));
    ui->address->setText(QString::fromStdString(systemCfg.Dropbox.Adress));
    ui->port->setText(QString::number(systemCfg.Dropbox.Port));
    ui->BackupAdress->setText(QString::fromStdString(systemCfg.Common.BackupPath));
    getStorageInfoList();
    if (systemCfg.Display.DisplayMode=="dark")
    {
        ui->dark->setChecked(true);
    }
    else
    {
        ui->light->setChecked(true);
    }
    if (systemCfg.Common.Language=="EN")
    {
        ui->english->setChecked(true);

    }
    else
    {
        ui->chinese->setChecked(true);
    }
    if (systemCfg.Hardware.EnableGPU)
    {
        gpuSwitchButton->setChecked(true);
    }
    else
    {
        gpuSwitchButton->setChecked(false);
    }
    if (systemCfg.Hardware.EnableCompress)
    {
        compressSwitchButton->setChecked(true);
    }
    else
    {
        compressSwitchButton->setChecked(false);
    }
}
void CSystemSettings::slotHospitalInfoClick()
{
    ui->stackedWidget->setCurrentIndex(1);
    setCheck();
    ui->HospitalInfo->setChecked(true);
    ui->Delete->setVisible(false);
    ui->Create->setVisible(false);
    ui->Edit->setVisible(false);
    initSysConfig();
}


void CSystemSettings::slotDropboxClick()
{
    ui->stackedWidget->setCurrentIndex(2);
    setCheck();
    ui->Dropbox->setChecked(true);
    ui->Delete->setVisible(false);
    ui->Create->setVisible(false);
    ui->Edit->setVisible(false);
    initSysConfig();
}
void CSystemSettings::slotBackupRestoreClick()
{
    ui->stackedWidget->setCurrentIndex(3);
    setCheck();
    ui->BackupRestore->setChecked(true);
    ui->Delete->setVisible(false);
    ui->Create->setVisible(false);
    ui->Edit->setVisible(false);
    initSysConfig();
}

void CSystemSettings::slotCreateUser()
{
    if(userInfo == nullptr)
    {
        userInfo = new CUserInfo(userService, "");
        userInfo->isCreate = true;
        connect(userInfo, &CUserInfo::initTableWidget, this, &CSystemSettings::initWidget);
        connect(userInfo, &CUserInfo::signalUserInfoClose, this, &CSystemSettings::slotCloseUserInfo);
    }
    userInfo->show();
}

void CSystemSettings::slotEditUser()
{
    const auto currentRow = ui->userTable->currentRow();
    const auto loginName = ui->userTable->item(currentRow, 0)->text();
    auto userList = userService->getUserByLoginName(loginName);
    if (userInfo == nullptr)
    {
        userInfo = new CUserInfo(userService,loginName);
        userInfo->isCreate = false;
        connect(userInfo, &CUserInfo::initTableWidget, this, &CSystemSettings::initWidget);
        connect(userInfo, &CUserInfo::signalUserInfoClose, this, &CSystemSettings::slotCloseUserInfo);
    }
    userInfo->show();

}
void CSystemSettings::slotDeleteUser()
{
    const auto currentRow = ui->userTable->currentRow();
    if (currentRow<0)
    {
        return;
    }

    CNxMessageBox msgBox(tr(""), tr("Are you sure to delete?"));
    msgBox.exec();
    if (msgBox.isConfirmed())
    {
        if (ui->userTable->item(currentRow, 0)->text()=="Sysuser")
        {
            CNxMessageBox msgBox(tr(""), tr("Built-in administrator user, cannot be deleted!"));
            msgBox.exec();
            return;
        }
        else
        {
            auto currentInfo = userService->getUserByLoginName(ui->userTable->item(currentRow, 0)->text());
            if (currentInfo.empty())
            {
                return;
            }
            auto currentuser = userService->getUserByIdentifyFK(QString::fromStdString(currentInfo[0].UID));
            userService->deleteUser(QString::fromStdString(currentInfo[0].UID), QString::fromStdString(currentuser[0].UID));
            initWidget();
        }
    }
    else 
    {
        return ;
    }
   
}

void CSystemSettings::slotCloseUserInfo()
{
    userInfo->close();
    delete userInfo;
    userInfo = nullptr;
}

void CSystemSettings::updateSysConfig()
{
	if (ui->HospitalName->text().isEmpty())
	{
		return;
	}
    if (ui->DepartmentsName->text().isEmpty())
    {
        return;
    }
    if (ui->address->text().isEmpty())
    {
        return;
    }
    if (ui->port->text().isEmpty())
    {
        return;
    }
    if (ui->BackupAdress->text().isEmpty())
    {
        return;
    }

    systemCfg.Hospital.HospitalName = ui->HospitalName->text().toStdString();
    systemCfg.Hospital.Department = ui->DepartmentsName->text().toStdString();
    systemCfg.Dropbox.Adress = ui->address->text().toStdString();
    systemCfg.Dropbox.Port = ui->port->text().toInt();
    systemCfg.Common.BackupPath = ui->BackupAdress->text().toStdString();
    if (gpuSwitchButton->Switchchecked)
    {
        systemCfg.Hardware.EnableGPU = true;
    }
    else
    {
        systemCfg.Hardware.EnableGPU = false;
    }
    if (compressSwitchButton->Switchchecked==true)
    {
        systemCfg.Hardware.EnableCompress = true;
    }
    else
    {
        systemCfg.Hardware.EnableCompress = false;
    }
    if (ui->chinese->isChecked())
    {
        systemCfg.Common.Language = "zh-CN";
    }
    else
    {
        systemCfg.Common.Language = "EN";
    }
    if (ui->dark->isChecked())
    {
        systemCfg.Display.DisplayMode = "dark";
    }
    else
    {
        systemCfg.Display.DisplayMode = "light";
    }
    systemConfig.UpdateSystemCfg(systemCfg);
    systemCfg = systemConfig.GetSystemCfg();
    initSysConfig();
}
void CSystemSettings::slotBackupClick()
{
    
    QString filePath = QFileDialog::getExistingDirectory(this, tr("Select Path"), "", QFileDialog::ShowDirsOnly);
    filePath=filePath+"/";
    if(!filePath.isEmpty())
    {
        ui->BackupAdress->setText(filePath);
    }
    else
    {
	    return;
    }

    
}
QString CSystemSettings :: formatBytes(qint64 bytes)
{
    QStringList units = { "B", "KB", "MB", "GB", "TB" };

    int i = 0;
    double size = bytes;

    while (size > 1024.0 && i < units.size() - 1)
    {
        size /= 1024.0;
        i++;
    }

    return QString("%1 %2").arg(size, 0, 'f', 1).arg(units[i]);
}
void  CSystemSettings::analyticalSql(const QString& fileName)
{
     QFile file(fileName);
     QDateTime qureUpdatetime;
     QDateTime sqlUpdatetime;
     if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
     {
        LOG_INFO("Could not open the file.");
        return;
     }
     QTextStream in(&file);
     QString content = in.readAll();
     file.close();
     // 定义正则表达式模式，用于匹配 INSERT 语句
     QRegularExpression regex("INSERT\\s+INTO\\s+[^;]+;");
     // 使用正则表达式匹配
     QRegularExpressionMatchIterator matchIterator = regex.globalMatch(content);
     while (matchIterator.hasNext())
     {
     	QRegularExpressionMatch match = matchIterator.next();
        QString  sql = match.captured();

        getSqlInfo(sql);
        QString burlesque= QString("SELECT   UpdateDateTime 	FROM  %1  WHERE uid = '%2'; ");
        burlesque = burlesque.arg(sqlInfo.tableName).arg(sqlInfo.uid);
        int queryResult = mysql_query(&mysqlInstance, burlesque.toLatin1());
        if (queryResult != 0) {
          
   
            LOG_INFO("Query execution failed: " + QString(mysql_error(&mysqlInstance)).toStdString()) ;
            return;
           
        }

        result = mysql_store_result(&mysqlInstance);
        MYSQL_ROW row;
        while (row = mysql_fetch_row(result))
        {
            qureUpdatetime= QDateTime::fromString(QString::fromStdString(row[0]), "yyyy-MM-dd hh:mm:ss");
        }
        sqlUpdatetime = QDateTime::fromString(sqlInfo.updateTime, "yyyy-MM-dd hh:mm:ss");
        if (sqlUpdatetime> qureUpdatetime)
        {
           
            sql.replace("INSERT", "REPLACE ");
            mysql_query(&mysqlInstance, sql.toLatin1());
        }
     }
}
void CSystemSettings::getSqlInfo(const QString& sqlStatement)
{
    // 使用正则表达式提取表名和所有字段值
    QRegularExpression regex("INSERT INTO `([^`]+)` VALUES \\(([^)]+)\\)");
    QRegularExpressionMatch match = regex.match(sqlStatement);

    if (match.hasMatch()) {
        QString tableName = match.captured(1);
        QString allFieldValues = match.captured(2);

        sqlInfo.tableName= tableName;

        // 分割字段值
        QStringList fieldValues = allFieldValues.split(',');
        if (fieldValues.size()>=3)
        {
            sqlInfo.uid = fieldValues[0].trimmed().remove(QRegularExpression("'"));
            sqlInfo.updateTime = fieldValues[2].trimmed().remove(QRegularExpression("'"));
        }
    }
  
}
void CSystemSettings::getStorageInfoList()
{
    QList<QStorageInfo> storageInfoList = QStorageInfo::mountedVolumes();//获取磁盘信息
    QString text = "";
    QDir dir(QString::fromStdString(systemCfg.Common.StoragePath));
    QFileInfo studyPath = QFileInfo(QString::fromStdString(systemCfg.Common.StoragePath));
    QString diskName = studyPath.absoluteFilePath().left(1);
    auto path = QString("%1:/").arg(diskName);
    QStorageInfo storage(path);
    if (storage.isValid() && storage.isReady()) {

        double rate = ((storage.bytesAvailable() / 1.0) / (storage.bytesTotal() / 1.0)) * 100;     
        text += formatBytes(storage.bytesAvailable()) +tr(" abailable ")+","+tr(" total ")+ formatBytes(storage.bytesTotal());
        ui->label_10->setText(text);
        ui->progressBar->setValue(rate);

    }
    else
    {
        if (systemCfg.Common.Language == "zh-CN")
        {
            text += QString::fromLocal8Bit("存储磁盘不存在,请检查!") + "\n";
        }
        else
        {
            text += QString::fromLocal8Bit("The storage disk does not exist. Check it!") + "\n";
        }
        ui->label_10->setText(text);
    }
}
void CSystemSettings::slotRestoreClick()
{
    // 弹出文件选择对话框
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::ExistingFiles); 
    dialog.setNameFilter("sql files (*.sql)");
    QStringList fileNames;
    QVector<QString> restorePathVector;
    QString  restorePath;
    if (dialog.exec())
    {
        fileNames = dialog.selectedFiles();
    }

    for (auto zipFile: fileNames)
    {
        analyticalSql(zipFile);
        QFileInfo fileInfo(zipFile);
        // 使用fileName()函数获取文件名
        restorePathVector.append(fileInfo.baseName());
    }
    oscillographService->setRestore(restorePathVector);
}

void CSystemSettings::slotUserManageClick()
{
    ui->stackedWidget->setCurrentIndex(0);
    setCheck();
    ui->UserManage->setChecked(true);
    ui->Delete->setVisible(true);
    ui->Create->setVisible(true);
    ui->Edit->setVisible(true);
    initWidget();
}

END_NX_NAMESPACE