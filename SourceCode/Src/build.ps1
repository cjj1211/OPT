$PSDefaultParameterValues['*:Encoding'] = 'utf8'
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
$Env:QtMsBuild="C:\Users\Administrator\AppData\Local\QtMsBuild"
$Env:PATH="$Env:MSBUILD_PATH;$Env:PATH"
#$Env:QT_PATH="D:\BuildTools\Qt\5.15.1\msvc2019_64"
$Env:QT_PATH="C:\Qt\Qt5.12.12\5.12.12\msvc2017_64"
$Env:QtToolsPath="$Env:QT_PATH\bin" #Support QtMSBuild , At desktop is done by the plug in
$Env:PATH="$Env:QtMsBuild;$Env:PATH"
$Env:PATH="$Env:QT_PATH\bin;$Env:PATH"
$Env:PATH="C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\amd64;$Env:PATH"
dotnet nuget list source;
#dotnet nuget add source "https://tfs.neuroxess.com/EEGRecorder/_packaging/NeuroXess/nuget/v3/index.json" -n nx-tfs --username "guobao.li" --password "nhlgb@0228" --store-password-in-clear-text  
# $RecorderServerPath="C:\Users\admin\workspace\RecorderServer"
# $BaseFilePath="$RecorderServerPath\Base"
# Get-Item $BaseFilePath\*
# Copy-Item -Recurse -Force "$BaseFilePath\SDL" .\MindExplorer\AppService\
# Get-Item .\MindExplorer\AppService\SDL\*
# Copy-Item -Recurse -Force "$BaseFilePath\ThirdParty\lib" .\MindExplorer\ThirdParty\
# Copy-Item -Recurse -Force "$BaseFilePath\packages" .\MindExplorer\
# Get-Item .\MindExplorer\*
# Write-Host "cp -r $BaseFilePath\ThirdParty\lib .\MindExplorer\ThirdParty finished" 
# Get-Item .\MindExplorer\ThirdParty\*

Copy-Item -Recurse -Force E:\AzureDevOpsPool\QtMsBuild  .\
msbuild Optimus/Optimus.vcxproj /t:Rebuild /p:Configuration=Release /p:Platform=x64
#dotnet nuget remore source -n nx-tfs
# Get-Item .\Output\MindExplorer\bin\*
