
set srcpath=%1%
echo srcpath

set outpath=%2%
echo %outpath%

7z a "%outpath%" "%srcpath%\" -r -mx -mf=BCJ2
::copy /b 7zSD.sfx + config.txt + Optimus.7z "%outpath%"