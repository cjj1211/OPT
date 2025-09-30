cd ./bin

::set deployer=C:\Qt\Qt5.12.12\5.12.12\msvc2017_64\bin\windeployqt.exe 

set deployer=%1

%deployer% Optimus.exe
%deployer% NxBackupService.exe

%deployer% NxAppService.dll
%deployer% NxBrainAreaWidget.dll
%deployer% NxCommunication.dll
%deployer% NxDBManager.dll
%deployer% NxElectrodeMap.dll
%deployer% NxEntity.dll
%deployer% NxLogger.dll
%deployer% NxSpectrogram.dll
%deployer% NxSystemCfg.dll
%deployer% NxWaveformWidget.dll

cd ..