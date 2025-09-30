#ifndef CCREATEEVENT_H
#define CCREATEEVENT_H
#include <QWidget>
#include <QLineEdit>
#include <QFocusEvent>
#include "CEventButton.h"
QT_BEGIN_NAMESPACE
namespace Ui { class CCreateEvent; }
QT_END_NAMESPACE
class CCreateEvent : public QWidget
{
    Q_OBJECT
public:
    CCreateEvent(QWidget *parent = nullptr);
    ~CCreateEvent();
    void setText(const QString& buttonName, const QString& buttoncolor);
    void changeModel( int Model);
    void setFouce();
signals:
    void setButton(const QString& buttonName,const QString& buttonColor);
    void editButton(const QString& buttonName, const QString& buttonColor, const QString& orgButtonColor, const QString& orgName);
    void deleteButtonClicked(const QString& name);
    void focusLost();

public :
    QStringList buttonOrgName;
protected:
    void keyPressEvent(QKeyEvent* event);
    void lineEditInputCheck();
private slots:
    void redCliked();
    void greenCliked();
    void blueCliked();
    void yellowCliked();
    void purpleCliked();
    void cyanCliked();
    void magentaCliked();
    void grayCliked();
    void editFinished();
private:
    Ui::CCreateEvent *ui;
    int  model=0;
    QString orgButtonColor;
    QString orgButtonName;
    QString buttonColor="#EC4F9A";
    QString styelSheet="background-color:%1;border:none;border-radius:0px;";
};
#endif // CCREATEEVENT_H
