#include "CCreateEvent.h"
#include "ui_CCreateEvent.h"
CCreateEvent::CCreateEvent(QWidget* parent)
	: QWidget(parent)
	, ui(new Ui::CCreateEvent)
{
	ui->setupUi(this);
	connect(ui->red, &QPushButton::clicked, this, &CCreateEvent::redCliked);
	connect(ui->green, &QPushButton::clicked, this, &CCreateEvent::greenCliked);
	connect(ui->blue, &QPushButton::clicked, this, &CCreateEvent::blueCliked);
	connect(ui->yellow, &QPushButton::clicked, this, &CCreateEvent::yellowCliked);
	connect(ui->purple, &QPushButton::clicked, this, &CCreateEvent::purpleCliked);
	connect(ui->cyan, &QPushButton::clicked, this, &CCreateEvent::cyanCliked);
	connect(ui->magenta, &QPushButton::clicked, this, &CCreateEvent::magentaCliked);
	connect(ui->gray, &QPushButton::clicked, this, &CCreateEvent::grayCliked);
	connect(ui->lineEdit, &QLineEdit::textEdited, this, &CCreateEvent::lineEditInputCheck);
	connect(ui->lineEdit, &QLineEdit::editingFinished, this, &CCreateEvent::editFinished);
	ui->deleteButton->setIcon(QIcon(":/Optimus/images/trainPage/close.svg"));
	ui->deleteButton->setStyleSheet("background: transparent; border: none;");
	ui->deleteButton->hide();

}
CCreateEvent::~CCreateEvent()
{
	delete ui;
}
void CCreateEvent::setText(const QString& buttonName, const QString& buttoncolor)
{
	orgButtonName = buttonName;
	orgButtonColor = buttoncolor;
	ui->lineEdit->setText(buttonName);
	buttonColor = buttoncolor;
	ui->lineEdit->setStyleSheet(styelSheet.arg(buttonColor));
	if (model == 1)
	{
		ui->deleteButton->show();
		connect(ui->deleteButton, &QPushButton::clicked, [=]() {
			emit deleteButtonClicked(orgButtonName);
			});
	}
}
void CCreateEvent::changeModel(int Model)
{
	model = Model;
}
void CCreateEvent::setFouce()
{
	ui->lineEdit->setFocus();
}
void CCreateEvent::redCliked()
{
	buttonColor = "#EC4F9A";
	ui->lineEdit->setStyleSheet(styelSheet.arg(buttonColor));
}

void CCreateEvent::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
	{
		if (model == 0)
		{
			if (buttonOrgName.contains(ui->lineEdit->text()) && ui->lineEdit->text() != orgButtonName && !ui->lineEdit->text().isEmpty())
			{
				return;
			}
			emit setButton(ui->lineEdit->text(), buttonColor);
		}
		else
		{
			if (buttonOrgName.contains(ui->lineEdit->text()) && ui->lineEdit->text() != orgButtonName && !ui->lineEdit->text().isEmpty())
			{
				return;
			}
			emit editButton(ui->lineEdit->text(), buttonColor, orgButtonColor, orgButtonName);
		}
	}
}
void CCreateEvent::greenCliked()
{
	buttonColor = "#C280FF";
	ui->lineEdit->setStyleSheet(styelSheet.arg(buttonColor));
	setFouce();
}
void CCreateEvent::blueCliked()
{
	buttonColor = "#BFBF00";
	ui->lineEdit->setStyleSheet(styelSheet.arg(buttonColor));
	setFouce();
}
void CCreateEvent::yellowCliked()
{
	buttonColor = "#9FA4EF";
	ui->lineEdit->setStyleSheet(styelSheet.arg(buttonColor));
	setFouce();
}
void CCreateEvent::lineEditInputCheck()
{

	if (buttonOrgName.contains(ui->lineEdit->text()) && ui->lineEdit->text() != orgButtonName && !ui->lineEdit->text().isEmpty())
	{
		ui->lineEdit->setStyleSheet("border: 1px solid red;");
	}
	else
	{
		ui->lineEdit->setStyleSheet(styelSheet.arg(buttonColor));
	}
}


void CCreateEvent::purpleCliked()
{
	buttonColor = "#F9941C";
	ui->lineEdit->setStyleSheet(styelSheet.arg(buttonColor));
	setFouce();
}
void CCreateEvent::cyanCliked()
{
	buttonColor = "#F65B64";
	ui->lineEdit->setStyleSheet(styelSheet.arg(buttonColor));
	setFouce();
}
void CCreateEvent::magentaCliked()
{
	buttonColor = "#51CBFF";
	ui->lineEdit->setStyleSheet(styelSheet.arg(buttonColor));
	setFouce();
}
void CCreateEvent::grayCliked()
{
	buttonColor = "#24C4BB";
	ui->lineEdit->setStyleSheet(styelSheet.arg(buttonColor));
	setFouce();
}

void CCreateEvent::editFinished()
{
	bool isMouseOver = this->underMouse();
	if (!isMouseOver)
	{
		if (model == 0)
		{
			if (buttonOrgName.contains(ui->lineEdit->text()) && ui->lineEdit->text() != orgButtonName && !ui->lineEdit->text().isEmpty())
			{
				return;
			}
			emit setButton(ui->lineEdit->text(), buttonColor);
		}
		else
		{
			if (buttonOrgName.contains(ui->lineEdit->text()) && ui->lineEdit->text() != orgButtonName && !ui->lineEdit->text().isEmpty())
			{
				return;
			}
			emit editButton(ui->lineEdit->text(), buttonColor, orgButtonColor, orgButtonName);
		}
	}

}
