#ifndef SYSTEM_CFG_H
#define SYSTEM_CFG_H

#include <iguana/json_reader.hpp>
#include <string>

struct HospitalCfg {
    std::string HospitalName;
    std::string Department;
};

REFLECTION(HospitalCfg, HospitalName, Department)

enum CommunicationMode {
    CM_USB = 0,
    CM_BLUE_TOOTH
};
struct DropboxCfg {
    std::string Adress;
    int  Port;

};
REFLECTION(DropboxCfg, Adress, Port)
struct HardwareCfg {
    bool EnableRecord;
    bool EnableFilter;
    bool EnableGPU;
    bool EnableCompress;
    bool SyntheticMode;
    int MultiNotchCount{ 1 };
    int ImpedanceFreq{ 1000 };
    std::string SaveFileType{ "rhd" };
    CommunicationMode CommunicationMode;
};

REFLECTION(HardwareCfg, EnableRecord, EnableFilter, EnableGPU, EnableCompress, SyntheticMode, MultiNotchCount, ImpedanceFreq, CommunicationMode, SaveFileType)



struct DisplayCfg {
    std::string DisplayMode;
};

REFLECTION(DisplayCfg, DisplayMode)

struct CommonCfg {
    std::string Version;
    std::string Language;
    bool EnableTestMode;
    std::string DBIp;
    std::string DBUser;
    std::string DBPassword;
    std::string DBName;
    int DBPort;
    std::vector<std::string>FPGAPort;
    std::string StoragePath;
    double  LogoutTime;
    std::string BackupPath;
    std::string RecorderAddress;
    std::string MasterAddress;
    int BackupDate;
    std::string AtlasType; // ÄÔÍ¼Æ×ÀàÐÍ
    int TrainAddress;
    std::string GamePath;
};
REFLECTION(CommonCfg, Version, Language, EnableTestMode, DBIp, DBUser, DBPassword, DBName, DBPort, FPGAPort, StoragePath, LogoutTime, RecorderAddress, MasterAddress, BackupPath, BackupDate, TrainAddress, GamePath,AtlasType)


struct SpectrogramCfg {
    bool ShowFreqMarker;
    int FreqMarkerValue;
    //int FftSize; //     256 512 1024 2048 4096 8192 16384
    int TScale; // 2s 5s 10s
    int MinFreq;
    int MaxFreq;
    int HarmonicsNum;
    int SpectrogramMode; // 0:Spectrogram 1:Spectrum
};
REFLECTION(SpectrogramCfg, ShowFreqMarker, FreqMarkerValue, TScale, MinFreq, MaxFreq, HarmonicsNum, SpectrogramMode);

struct SystemCfg {
    HospitalCfg Hospital;
    HardwareCfg Hardware;
    DisplayCfg Display;
    CommonCfg Common;
    SpectrogramCfg Spectrogram;
    DropboxCfg  Dropbox;
};

REFLECTION(SystemCfg, Hospital, Hardware, Display,Common, Spectrogram, Dropbox)
#endif
