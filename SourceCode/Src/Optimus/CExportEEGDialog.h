#ifndef C_EXPORT_EEG_DIALOG
#define C_EXPORT_EEG_DIALOG
#include "Global.h"
#include <QDialog>
#include <QTimer>
#include <memory>
#include "CDragableDialog.h"
#include "ui_CExportEEGDialog.h"


BEGIN_NX_NAMESPACE
class CProgressBarDialog;
class COscillographService;

class CExportEEGDialog  : public CDragableDialog
{
    Q_OBJECT
    QT_TRANSLATE_HANDLER
public:
    explicit CExportEEGDialog(COscillographService* service_, QWidget *parent_);
    ~CExportEEGDialog() override;

private slots:
    void slotOpenDialog();
    void slotExport();
    void slotUpdateProgress();
    void slotTargetPathChanged(QString text);

private:
    std::unique_ptr<Ui::CExportEEGDialog> ui;
    COscillographService* service;
    CProgressBarDialog* progressBar;
    QTimer exportTimer;
};

END_NX_NAMESPACE

#endif
