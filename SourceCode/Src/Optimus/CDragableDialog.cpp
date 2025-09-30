#include "CDragableDialog.h"
#include <QMouseEvent>

BEGIN_NX_NAMESPACE

CDragableDialog::CDragableDialog(QWidget *parent)
    : QDialog(parent)
    , mousePress(false)
{}

CDragableDialog::~CDragableDialog() = default;

void CDragableDialog::mouseMoveEvent(QMouseEvent * event)
{
    if (mousePress) {
        move(event->globalPos() - mousePoint);
    }
    QDialog::mouseMoveEvent(event);
}

void CDragableDialog::mousePressEvent(QMouseEvent* event)
{
    if ((event->button() == Qt::LeftButton)) {
        mousePress = true;
        mousePoint = event->globalPos() - this->pos();
    }
    QDialog::mousePressEvent(event);
}

void CDragableDialog::mouseReleaseEvent(QMouseEvent* event)
{
    mousePress = false;
    QDialog::mouseReleaseEvent(event);
}

END_NX_NAMESPACE