#include "CEventWidget.h"
#include "ui_CEventWidget.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QColorDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QScrollBar>
CEventWidget::CEventWidget(QWidget* parent)
	: QWidget(parent)
    , ui(new Ui::CEventWidget)
{
	ui->setupUi(this);
    qRegisterMetaType<EventButtonData>("EventButtonData");
	//setWindowFlags(Qt::FramelessWindowHint);
	//ui->widget->setWindowFlags( Qt::FramelessWindowHint);
	createEvent = new CCreateEvent(ui->widget);
	connect(createEvent, &CCreateEvent::editButton, this, &CEventWidget::editButton);
	connect(createEvent, &CCreateEvent::deleteButtonClicked, this, &CEventWidget::deleteButtonClicked);
	connect(createEvent, &CCreateEvent::setButton, this, &CEventWidget::addNewButton);
	createEvent->hide();
	createEvent->hide();
	loadJsonData();
	createButtons();
    connect(createEvent, &CCreateEvent::setButton, this, &CEventWidget::addNewButton);

	
}
void CEventWidget::loadJsonData() {
	QFile file(R"(..\config\Event.json)");
	if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QByteArray jsonData = file.readAll();
		QJsonDocument doc = QJsonDocument::fromJson(jsonData);
		jsonDataArray = doc.array();
		file.close();
	}
}
void CEventWidget::saveJsonData() {
	QFile file(R"(..\config\Event.json)");
	if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QJsonDocument doc(jsonDataArray);
		file.write(doc.toJson());
		file.close();
	}
}
void CEventWidget::createButtons() {

	for (CEventButton* button : buttons) {
		button->hide();
	/*	delete button;*/
	}
	createEvent->hide();
	buttons.clear();
	buttonOrgName.clear();
	for (const QJsonValue& value : jsonDataArray) {
		QJsonObject obj = value.toObject();
		QString name = obj["name"].toString();
		QString colorString = obj["color"].toString();
		QColor color(colorString);
        CEventButton* button = new CEventButton(name, color, ui->widget);
		button->setButtonColor(color); // 设置按钮的背景颜色
        connect(button, &CEventButton::longPressed, this, &CEventWidget::onButtonLongPressed);
        connect(button, &CEventButton::initEvent, this, &CEventWidget::initEvent);
		buttons.append(button);
		buttonOrgName.append(name);
	}
	col = 0; row = 0;
    CEventButton* addButton = new CEventButton("+", "#EC4F9A", ui->widget);
	addButton->setButtonColor("#EC4F9A"); // 设置按钮的背景颜色
    connect(addButton, &QPushButton::clicked, this, &CEventWidget::addButtonClicked);
	buttons.push_back(addButton);
    int x = 20; // 设置起始 x 坐标
	int y = 20; // 设置起始 y 坐标
	int buttonWidth = 90; // 设置按钮宽度
	int buttonHeight = 40; // 设置按钮高度
	int horizontalSpacing = 10; // 设置按钮之间的水平间距
	int verticalSpacing = 20; // 设置两行按钮之间的垂直间距
	// 计算按钮布局大小
	int maxX = 0; // 最大 x 坐标
	int maxY = 0; // 最大 y 坐标
	// 将起始坐标转换为窗口坐标
	
	x += this->x();
	y += this->y();

    for (CEventButton* button : buttons)
	{
        EventButtonData customData;
		customData.posCol = col;
		customData.posRow = row;
        button->setProperty("EventButtonData", QVariant::fromValue(customData));
		button->setGeometry(x, y, buttonWidth, buttonHeight); // 设置按钮位置和大小
		if (x > maxX) {
			maxX = x;
		}
		if (y > maxY) {
			maxY = y;
		}
		col++;
		x += buttonWidth + horizontalSpacing; // 按钮之间的水平间距
		if (col == 2) {
			x = 20; // 重置 x 坐标
			y += buttonHeight + verticalSpacing; // 两行按钮之间的垂直间距
			col = 0;
			++row;
		}

		if (button != addButton)
		{
            connect(button, &CEventButton::buttonClick, this, &CEventWidget::slotButtonClick);
		}
	}

	if (maxY + buttonHeight + 30<600)
	{
		this->setFixedSize(maxX + buttonWidth + 20, maxY + buttonHeight + 30); // 20 是一些额外空间用于边缘和调整
		ui->widget->setFixedHeight(maxY + buttonHeight + 30); // 20 是一些额外空间用于边缘和调整
	}
	else
	{

	
		ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
		// 设置滚动条的滑动步长
		ui->scrollArea->verticalScrollBar()->setSingleStep(20); // 设置垂直滚动条步长为 20
		ui->widget->setFixedHeight( maxY + buttonHeight + 30); // 20 是一些额外空间用于边缘和调整
		this->setFixedSize(maxX + buttonWidth + 20, 600); // 20 是一些额外空间用于边缘和调整
	}
	// 显示按钮
	for (CEventButton* button : buttons) {
		button->show(); 
	}

	status = 0;
}
void CEventWidget::deleteButtonClicked(const QString& name) {
	for (int i = 0; i < jsonDataArray.size(); ++i)
	{
		QJsonObject obj = jsonDataArray[i].toObject();
		if (obj["name"].toString() == name)
		{
			jsonDataArray.removeAt(i);
			saveJsonData();
			break;
		}
	}
	createButtons();
}
void CEventWidget::addButtonClicked() {
	if (status == 0)
	{
		QPushButton* senderButton = qobject_cast<QPushButton*>(sender());
		if (senderButton)
		{
			createEvent->buttonOrgName = buttonOrgName;

			createEvent->changeModel(0);
			senderButton->hide();
			QPoint pos = senderButton->mapTo(this, QPoint(0, 0));
			createEvent->raise();
			createEvent->move(pos);
			createEvent->setText("", "#EC4F9A");
			createEvent->show();
			createEvent->setFouce();
			status = 1;
			ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
			
		}
	}
}
void CEventWidget::onButtonLongPressed(const QString& name) {

}
bool CEventWidget::isButtonNameExists(const QString& name) const {
	for (const QJsonValue& value : jsonDataArray)
	{
		QJsonObject obj = value.toObject();
		if (obj["name"].toString() == name)
		{
			return true;
		}
	}
	return false;
}
void CEventWidget::addNewButton(const QString& name, const QString& color)
{
	if (!name.isEmpty() && !isButtonNameExists(name))
	{
		QJsonObject obj;
		obj["name"] = name;
		obj["color"] = color;
		jsonDataArray.append(obj);
		saveJsonData();
	}
	createButtons();
}
void CEventWidget::editButton(const QString& name, const QString& color, const QString& orgColor, const QString& orgName)
{
	QJsonObject obj;
	obj["name"] = name;
	obj["color"] = color;
	for (int i = 0; i < jsonDataArray.size(); i++)
	{
		if (orgName == jsonDataArray[i].toObject().value("name").toString())
		{
			if (name == orgName)
			{
				jsonDataArray.replace(i, obj);
				saveJsonData();
				break;
			}
			else
			{
				if (!name.isEmpty() && !isButtonNameExists(name))
				{
					jsonDataArray.replace(i, obj);
					saveJsonData();
					break;
				}
			}
		}
	}
	status = 0;
	createButtons();
	
}
void CEventWidget::initEvent(const QString& name, const QString& color)
{
	if (status == 0)
	{
        CEventButton* clickedButton = qobject_cast<CEventButton*>(sender());
		for (CEventButton* button : buttons)
		{
			button->setButtonEnabled(false);
		}

		createEvent->buttonOrgName = buttonOrgName;
		createEvent->changeModel(1);
		clickedButton->hide();
		QPoint pos = clickedButton->mapTo(this, QPoint(0, 0));
		createEvent->raise();
		createEvent->move(pos);
		createEvent->show();
		createEvent->setFouce();
		createEvent->setText(name, color);
		status = 1;
		//ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	}
}
void CEventWidget::slotButtonClick(const QString& name, int count)
{
	emit buttonClick(name, count);
}
