#ifndef _DETECTOR_CONFIGURATION_H
#define _DETECTOR_CONFIGURATION_H

#include "nxbrainareawidget_global.h"
#include <string>
#include <iguana/reflection.hpp>
#include <ylt/struct_json/json_reader.h>

struct DetectorConfig
{
    std::vector<double> DetectorCenter;
    std::vector<double> DetectorNormal;
    std::vector<double> ProbeCenter;
    std::vector<double> ProbeNormal;
    std::vector<int> DetectorColor;
    std::vector<int> ProbeColor;
};
REFLECTION(DetectorConfig, DetectorCenter, DetectorNormal, DetectorColor, ProbeCenter, ProbeNormal, ProbeColor);

class CDetectorConfig
{
public:
    static DetectorConfig getConfig(const std::string& filepath)
    {
        DetectorConfig cfg;
        struct_json::from_json_file(cfg, filepath);
        return cfg;
    }
};
#endif
