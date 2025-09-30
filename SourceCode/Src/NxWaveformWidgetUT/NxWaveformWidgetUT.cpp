#include "NxWaveformWidgetUT.h"
#include <QApplication>

NxWaveformWidgetUT::NxWaveformWidgetUT()
{
    widget = new MultiColumnDisplay(this);
}

NxWaveformWidgetUT::~NxWaveformWidgetUT()
{
    delete widget;
    widget = nullptr;
}

void NxWaveformWidgetUT::CASE_TEST_CREATE_WIDGET() {

    widget->show();
    
}
