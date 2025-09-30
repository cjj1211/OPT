#pragma once
#include "Global.h"
#include "ui_CHistoryDataWidget.h"
BEGIN_NX_NAMESPACE
class CHistoryDataWidget : public QWidget
{
	Q_OBJECT

public:
	CHistoryDataWidget(QWidget *parent = nullptr);
	~CHistoryDataWidget();
	void showData(QMap<QString, QString>data);
private:
	Ui::CHistoryDataWidgetClass ui;
};
END_NX_NAMESPACE