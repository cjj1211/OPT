#include "CDisplayStatus.h"


// 获取显示状态

CDisplayStatus& CDisplayStatus::instance() {
    static CDisplayStatus _instance;
    return _instance;
}

void CDisplayStatus::reset()
{
    // TODO
}

// 灵敏度

inline float CDisplayStatus::getYScale() const { return yScale->getNumericValue(); }

inline float CDisplayStatus::getYScale(int index) const { return yScale->getNumericValue(index); }

int NX::CDisplayStatus::getYScaleIndex() const
{
    return yScale->getIndex();
}

inline void CDisplayStatus::shiftYScale(int index) const { yScale->shiftIndex(index); }

inline int CDisplayStatus::getYScalerNumOfItems() const { return yScale->numberOfItems(); }

std::string NX::CDisplayStatus::getYScaleName(int index) const
{
    return yScale->items[index].displayedValueName;
}

// 走纸速度

inline double CDisplayStatus::getPaperSpeed() const { return tPaperSpeed->getNumericValue(); }

// sweep模式为绘制区域波形的最右侧像素x坐标， roll模式为波形最左侧位置
inline int CDisplayStatus::getValidDataIndex() const {
    return validDataIndex;
}

CDisplayStatus::CDisplayStatus()
    : sweepFirstTime(false),
    showRuler(true),
    rollMode(false),
    clipWaveforms(false),
    // 显示禁用的通道
    showDisabledChannels(false),
    // 显示aux通道
    showVerticalTimeLines(true)
{
    tPaperSpeed = new CDiscreteList();
    tPaperSpeed->addItem("15", "15 mm/s", 15.0); // 数值对应的是mm/s
    tPaperSpeed->addItem("30", "30 mm/s", 30.0); // 数值对应的是mm/s
    tPaperSpeed->addItem("40", "40 mm/s", 40.0);
    tPaperSpeed->addItem("50", "50 mm/s", 50.0);
    tPaperSpeed->addItem("60", "60 mm/s", 60.0);
    tPaperSpeed->addItem("120", "120 mm/s", 120.0);
    tPaperSpeed->addItem("240", "240 mm/s", 240.0);
    tPaperSpeed->addItem("250", "250 mm/s", 250.0);
    // 测试用走纸速度
    tPaperSpeed->addItem("500", "500 mm/s", 500.0);
    tPaperSpeed->addItem("1000", "1000 mm/s", 1000.0);
    tPaperSpeed->addItem("5000", "5000 mm/s", 5000.0);
    tPaperSpeed->addItem("10000", "10000 mm/s", 10000.0);
    //tPaperSpeed->addItem("20000", "20000 mm/s", 20000.0);
    //tPaperSpeed->addItem("30000", "30000 mm/s", 30000.0);
    tPaperSpeed->setValue("50");

    yScale = new CDiscreteList();
    yScale->addItem("0.1", "0.1 " + MicroVoltsSymbol.toStdString() + "/mm", 0.1);
    yScale->addItem("0.5", "0.5 " + MicroVoltsSymbol.toStdString() + "/mm", 0.5);
    yScale->addItem("1", "1 " + MicroVoltsSymbol.toStdString() + "/mm", 1.0);
    yScale->addItem("2", "2 " + MicroVoltsSymbol.toStdString() + "/mm", 2.0);
    yScale->addItem("5", "5 " + MicroVoltsSymbol.toStdString() + "/mm", 5.0);
    yScale->addItem("7", "7 " + MicroVoltsSymbol.toStdString() + "/mm", 7.0);
    yScale->addItem("10", "10 " + MicroVoltsSymbol.toStdString() + "/mm", 10.0);
    yScale->addItem("15", "15 " + MicroVoltsSymbol.toStdString() + "/mm", 15.0);
    yScale->addItem("20", "20 " + MicroVoltsSymbol.toStdString() + "/mm", 20.0);
    yScale->addItem("50", "50 " + MicroVoltsSymbol.toStdString() + "/mm", 50.0);
    yScale->addItem("100", "100 " + MicroVoltsSymbol.toStdString() + "/mm", 100.0);
    yScale->addItem("200", "200 " + MicroVoltsSymbol.toStdString() + "/mm", 200.0);
    yScale->addItem("500", "500 " + MicroVoltsSymbol.toStdString() + "/mm", 500.0);
    yScale->addItem("1000", "1000 " + MicroVoltsSymbol.toStdString() + "/mm", 1000.0);
    yScale->addItem("2000", "2000 " + MicroVoltsSymbol.toStdString() + "/mm", 2000.0);
    yScale->addItem("5000", "5000 " + MicroVoltsSymbol.toStdString() + "/mm", 5000.0);
    yScale->setValue("50");

}

void CDisplayStatus::updateDisplayWaveformNames(const QStringList& pinnedWaves, const QStringList& displayWaves)
{
    visibleWaveNames.clear();
    for (auto& name : pinnedWaves)
    {
        visibleWaveNames.insert(name);
    }

    for (auto& name : displayWaves)
    {
        visibleWaveNames.insert(name);
    }

    qDebug() << visibleWaveNames.count();
}

QSet<QString> CDisplayStatus::getDisplayWaveformNames() const
{
    return visibleWaveNames;
}