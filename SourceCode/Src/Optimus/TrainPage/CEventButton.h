#ifndef CEVENTBUTTON_H
#define CEVENTBUTTON_H
#include <QPushButton>
#include <QColor>
#include <QTimer>
#include <QMouseEvent>
#include <QGraphicsOpacityEffect>
class CEventButton : public QPushButton
{
	Q_OBJECT
public:
    explicit CEventButton(const QString& name, const QColor& color, QWidget* parent = nullptr);
	QString getName() const;
	void setButtonColor(const QColor& color);
	void setButtonEnabled(bool type);
public:
	int status=0;
signals:
	void longPressed(const QString& name);
	void initEvent(const QString& name, const QString& color);
	void buttonClick(const QString& name, int count);
protected:
	void mousePressEvent(QMouseEvent* event) override;
private slots:
	void onLongPress();
	void onButtonReleased();
	void onButtonClick();
private:
	QString buttonName;
	QColor buttonColor;
	QTimer longPressTimer;
	int clickCount = 1;
	QGraphicsOpacityEffect* opacityEffect;
};
#endif // CEVENTBUTTON_H
