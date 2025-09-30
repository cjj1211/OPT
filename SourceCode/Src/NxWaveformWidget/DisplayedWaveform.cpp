#include "DisplayedWaveform.h"

WaveformType DisplayedWaveform::translateSignalTypeToWaveformType(UiSignalType signalType)
{
    switch (signalType) {
    case UiSignalType::AmplifierSignal:
        return WaveformType::AmplifierWaveform;
    default:
        return WaveformType::UnknownWaveform;
    }
}
