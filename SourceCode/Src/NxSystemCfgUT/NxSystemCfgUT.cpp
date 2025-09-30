#include "NxSystemCfgUT.h"
#include "../NxSystemCfg/CSystemCfg.h"

const char filepath[] = "E:/SourceCode/Output/config/SystemConfig.json";

#define testCfg CSystemCfg::Instance(filepath)

void NxSystemCfgUT::CASE_TEST_READ()
{
    auto cfg = testCfg.GetSystemCfg();
    QVERIFY(cfg.Hospital.HospitalName == "TestName");
    QVERIFY(cfg.Hospital.Department == "TestDepartment");

    QVERIFY(cfg.Hardware.EnableRecord == true);
    QVERIFY(cfg.Hardware.EnableFilter == false);
    QVERIFY(cfg.Hardware.EnableGPU == true);
    QVERIFY(cfg.Hardware.EnableCompress == false);
    QVERIFY(cfg.Hardware.CommunicationMode == CommunicationMode::CM_USB);


    QVERIFY(cfg.Display.DisplayMode == "light");
}


void NxSystemCfgUT::CASE_TEST_WRITE()
{
    auto cfg = testCfg.GetSystemCfg();
    cfg.Hospital.HospitalName = "xxxx";
    cfg.Hardware.EnableRecord = true;
    cfg.Hardware.EnableFilter = false;
    cfg.Hardware.EnableGPU = true;
    cfg.Hardware.EnableCompress = false;
    cfg.Hardware.CommunicationMode = CommunicationMode::CM_USB;


    cfg.Display.DisplayMode = "light";
    cfg.Common.Language = "zh-CN";
    QVERIFY(testCfg.UpdateSystemCfg(cfg)==true);
}




