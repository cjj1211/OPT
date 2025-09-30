#ifndef CEVENTWIDGET_H
#define CEVENTWIDGET_H
#include <QWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QJsonArray>
#include "CEventButton.h"
#include "CCreateEvent.h"
#include <QScrollArea>
#include <QVBoxLayout>
#include <QMetaType>
#include <QStackedWidget>
struct EventButtonData
{
	int  posRow;
	int  posCol;
};
Q_DECLARE_METATYPE(EventButtonData);
QT_BEGIN_NAMESPACE
namespace Ui { class CEventWidget; }
QT_END_NAMESPACE
class CEventWidget : public QWidget
{
	Q_OBJECT
public:
    explicit CEventWidget(QWidget* parent = nullptr);
	void loadJsonData();
	void saveJsonData();
	void createButtons();

public:
	QList<CEventButton*> buttons;
	int status = 0;
signals:
	void buttonClick(const QString& name, int count);
public slots:
	void deleteButtonClicked(const QString& name);
	void addButtonClicked();
private slots:
	void onButtonLongPressed(const QString& name);
	bool isButtonNameExists(const QString& name) const;
	void addNewButton(const QString& name, const QString& color);
	void editButton(const QString& name, const QString& color, const QString& orgColor, const QString& orgName);
	void initEvent(const QString& name, const QString& color);
	void slotButtonClick(const QString& name, int count);
private:
	QGridLayout* layout;
	int scrollBarValue = 0;
	QJsonArray jsonDataArray;
	CCreateEvent* createEvent;
	int col = 0;
	int row = 0;
	QStringList buttonOrgName;


    Ui::CEventWidget* ui;
};
#endif // CEVENTWIDGET_H
