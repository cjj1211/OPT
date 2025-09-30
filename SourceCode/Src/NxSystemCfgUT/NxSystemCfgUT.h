#pragma once
#include "nxsystemcfgut_global.h"
#include <QtTest/QtTest>

using namespace NX;

class NxSystemCfgUT:public QObject
{
    Q_OBJECT

private slots:
    void CASE_TEST_READ();
    void CASE_TEST_WRITE();
};
