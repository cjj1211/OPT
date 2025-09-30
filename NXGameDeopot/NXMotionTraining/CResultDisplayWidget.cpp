#include "CResultDisplayWidget.h"
BEGIN_NX_NAMESPACE
CResultDisplayWidget::CResultDisplayWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	
}

CResultDisplayWidget::~CResultDisplayWidget()
{
	
}

void CResultDisplayWidget::setBackground(int score)
{

	ui.score->setText(QString::number(score));
}

END_NX_NAMESPACE
