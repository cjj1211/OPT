#ifndef SWITCHBUTTON_H
#define SWITCHBUTTON_H

#include <QWidget>
#include <QTimer>
#include <QColor>
#include "../Global.h"
BEGIN_NX_NAMESPACE
class CSwitchButton : public QWidget
{
    Q_OBJECT


public:
    explicit CSwitchButton(QWidget *parent = nullptr);
    ~CSwitchButton(){}
public:
    bool Switchchecked;             //是否选中
    void setChecked(bool checked);
signals:
    void statusChanged(bool checked);

public slots:

private slots:
    void updateValue();

private:
    void drawBackGround(QPainter *painter);
    void drawSlider(QPainter *painter);

protected:
    void paintEvent(QPaintEvent *ev);
    void mousePressEvent(QMouseEvent *ev);

private:
    int Switchspace;                //滑块距离边界距离
    int Switchradius;               //圆角角度


    bool showText;            //是否显示文字
    bool showCircle;          //是否显示圆圈
    bool animation;           //是否使用动画

    QColor bgColorOn;         //打开时候的背景色
    QColor bgColorOff;        //关闭时候的背景色
    QColor sliderColorOn;     //打开时候滑块颜色
    QColor sliderColorOff;    //关闭时候滑块颜色
    QColor textColor;         //文字颜色

    QString textOn;           //打开时候的文字
    QString textOff;          //关闭时候的文字

    QTimer  *timer;            //动画定时器
    int     step;             //动画步长
    int     startX;           //滑块开始X轴坐标
    int     endX;             //滑块结束X轴坐标

public:
    int SwitchButtonspace()                 const;
    int SwitchButtonradius()                const;
    bool SwitchButtonchecked()              const;
    bool SwitchButtonshowText()             const;
    bool showCircel()           const;
    bool SwitchButtonanimation()            const;

    QColor SwitchButtonbgColorOn()          const;
    QColor SwitchButtonbgColorOff()         const;
    QColor SwitchButtonsliderColorOn()      const;
    QColor SwitchButtonsliderColorOff()     const;
    QColor SwitchButtontextColor()          const;

    QString SwitchButtontextOn()            const;
    QString SwitchButtontextOff()           const;

    int SwitchButtonstep()                  const;
    int SwitchButtonstartX()                const;
    int SwitchButtonendX()                  const;


public Q_SLOTS:
    void setSpace(int space);
    void setRadius(int radius);

    void setShowText(bool show);
    void setShowCircle(bool show);
    void setAnimation(bool ok);

    void setBgColorOn(const QColor &color);
    void setBgColorOff(const QColor &color);
    void setSliderColorOn(const QColor &color);
    void setSliderColorOff(const QColor &color);
    void setTextColor(const QColor &color);

    void setTextOn(const QString &text);
    void setTextOff(const QString &text);



};



#endif // SWITCHBUTTON_H
END_NX_NAMESPACE