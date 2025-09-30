#pragma once
#include "Global.h"
#include <QWidget>
#include "ui_CResultDisplayWidget.h"
BEGIN_NX_NAMESPACE
class CResultDisplayWidget : public QWidget
{
	Q_OBJECT

public:
	CResultDisplayWidget(QWidget *parent = nullptr);
	~CResultDisplayWidget();
	// 根据score来初始化背景图片
	void setBackground(int score);

private:
	Ui::CResultDisplayWidgetClass ui;
};
END_NX_NAMESPACE
