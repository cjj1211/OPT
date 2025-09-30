#include "CEventButton.h"

CEventButton::CEventButton(const QString& name, const QColor& color, QWidget* parent)
	: QPushButton(name, parent), buttonName(name), buttonColor(color) {
	longPressTimer.setSingleShot(true);
	longPressTimer.setInterval(1000); // 设置长按时长为1秒（1000毫秒）
    connect(&longPressTimer, &QTimer::timeout, this, &CEventButton::onLongPress);
    connect(this, &QPushButton::released, this, &CEventButton::onButtonReleased);
    connect(this, &QPushButton::clicked, this, &CEventButton::onButtonClick);
	setToolTip(QString::fromLocal8Bit("长按进行编辑、删除")); // 设置悬停提示文本
	setFixedSize(70, 20);
    opacityEffect = new QGraphicsOpacityEffect(this);
	opacityEffect->setOpacity(0.95); // 设置透明度值，范围从0（完全透明）到1（完全不透明）
	this->setGraphicsEffect(opacityEffect);
}
QString CEventButton::getName() const {
	return buttonName;
}
void CEventButton::setButtonColor(const QColor& color) {
	buttonColor = color;
	QPalette palette = this->palette();
	palette.setColor(QPalette::Button, buttonColor);
	QString colors = "background-color:%1;border: none;QToolTip { color: white; }";
	this->setStyleSheet(colors.arg(buttonColor.name()));
}
void CEventButton::setButtonEnabled(bool type)
{
	setEnabled(type);
}
void CEventButton::mousePressEvent(QMouseEvent* event) {
	longPressTimer.start();
	QPushButton::mousePressEvent(event);
}
void CEventButton::onLongPress() {
	emit longPressed(buttonName);
	emit initEvent(buttonName, buttonColor.name());
}
void CEventButton::onButtonReleased() {
	longPressTimer.stop();
}
void CEventButton::onButtonClick()
{
	
	if (clickCount == 1)
	{
		clickCount++;
		opacityEffect->setOpacity(1); // 设置透明度值，范围从0（完全透明）到1（完全不透明）
		this->setGraphicsEffect(opacityEffect);
		emit buttonClick(buttonName, 1);
	}
	else
	{
		if (clickCount == 2)
		{
			opacityEffect->setOpacity(0.95); // 设置透明度值，范围从0（完全透明）到1（完全不透明）
			this->setGraphicsEffect(opacityEffect);
			clickCount = 1;
			emit buttonClick(buttonName, 2);
		}
	}
}
