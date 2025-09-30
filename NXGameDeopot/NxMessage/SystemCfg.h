#ifndef SYSTEM_CFG_H
#define SYSTEM_CFG_H

#include <iguana/json_reader.hpp>
#include <string>
struct CommonCfg {
    int BackGroundVolume;
    int GameVolume;
    int CountDownTime;
    std::string Language;
    std::string ClientAddress;
    int HeartBeatTime;
    int MissedHeartbeatsThreshold;
    int Master;
    int Outboard;
};
REFLECTION(CommonCfg, BackGroundVolume, GameVolume, CountDownTime, Language, ClientAddress, HeartBeatTime, MissedHeartbeatsThreshold, Master, Outboard)
struct SystemCfg {
   
    CommonCfg Common;
   
};
REFLECTION(SystemCfg,Common)
#endif
