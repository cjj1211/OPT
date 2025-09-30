/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/

#ifndef __C_COMMUNICATION_DTO_H
#define __C_COMMUNICATION_DTO_H

#include "nxwaveformwidget_global.h"
#include <map>

BEGIN_NX_NAMESPACE

struct SignalSegment
{
    QString wavename;
    std::vector<float> yMinData;
    std::vector<float> yMaxData;
    std::vector<float> yData;
};

// Data transfer , from recorder server to Recorder UI Widget;
class UiDisplayDto
{
public:
    UiDisplayDto()
        : signalSegments({})
    {
    }

    ~UiDisplayDto()
    {
        for (const auto segment : signalSegments | std::views::values)
        {
            delete segment;
        }
    }

    void addSignalSegment(SignalSegment* segment) {
        if (signalSegments.contains(segment->wavename)) {
            signalSegments[segment->wavename] = segment;
        }
        else {
            signalSegments.emplace(segment->wavename, segment);
        }
    };

    SignalSegment* getSignalSegment(const QString& wavename) {
        if (signalSegments.contains(wavename))
        {
            return signalSegments[wavename];
        }
        return nullptr;
    }

    // 判断当前通道是否存在waveformSegment
    bool isContainsWave(const QString& wavename) const {
        return signalSegments.contains(wavename);
    };

private:
    std::map<QString, SignalSegment*> signalSegments;
};

END_NX_NAMESPACE
#endif
