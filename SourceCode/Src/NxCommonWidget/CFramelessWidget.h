// /******************************************************************************
// *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
// *  作者 : 宋志杰 zhijie.song@neuroxess.com
// *  创建日期:   1 2024
// *  修改记录:
// *  <修改者姓名><邮件地址>   <修改内容>
// ******************************************************************************/

#ifndef C_FRAMELESS_WIDGET
#define C_FRAMELESS_WIDGET
#include <QDialog>

#include "nx_common_widget_global.h"
#include <QWidget>

class NX_COMMON_WIDGET_EXPORT FramelessWidget: public QDialog
{
    Q_OBJECT
public:
    enum EAreaType
    {
        Area_Invalid,   // 窗口范围外

        Area_Middle,    // 窗口中间区域
        Area_Left,      // 窗口左边拉伸区域
        Area_Right,     // 窗口右边拉伸区域

        Area_Top,       // 窗口上方拉伸区域
        Area_TopLeft,   // 窗口左上角拉伸区域
        Area_TopRight,  // 窗口右上角拉伸区域

        Area_Bottom,        // 窗口底部拉伸区域
        Area_BottomLeft,    // 窗口左下角拉伸区域
        Area_BottomRight,   // 窗口右下角拉伸区域
    };
    explicit FramelessWidget(QWidget* parent = nullptr);
    virtual ~FramelessWidget() override;

    void setStrengthDelta(int delta);// 设置拉伸区域大小（距离窗口边缘多少个像素）

protected:
    void enterEvent(QEvent* e) override;// 重写鼠标移入事件，记录鼠标原始形状
    void leaveEvent(QEvent* e) override;// 重写鼠标移出事件，回复鼠标原始形状

    void mousePressEvent(QMouseEvent* e) override;  // 重写鼠标按下事件
    void mouseReleaseEvent(QMouseEvent* e) override;// 重写鼠释放事件
    void mouseMoveEvent(QMouseEvent* e) override;   // 重写鼠标移动事件

    virtual bool isEnabledDrag(QMouseEvent* e);

private:
    EAreaType getAreaType(QPoint) const;// 获取位置所在的鼠标区域类型

private:
    int strengthArea{};// 在离窗口边缘多少个像素会触发拉伸
    QPoint clickPos;// 鼠标左键点击位置
    Qt::CursorShape cursorShape;// 记录原始鼠标类型
    EAreaType areaType{ Area_Invalid };// 鼠标当前所在区域类型
    bool moving{ false }; 
};

#endif
