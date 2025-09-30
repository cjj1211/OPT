#pragma once
#include "NxWaveformWidget/MultiColumnDisplay.h"
#include "ui_NxWaveformWidgetUT.h"


class NxWaveformWidgetUT : public QWidget
{
    Q_OBJECT

public:
    NxWaveformWidgetUT();
    ~NxWaveformWidgetUT();

private slots:
    void CASE_TEST_CREATE_WIDGET();

private:
    MultiColumnDisplay* widget;
};
