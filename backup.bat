REM move big files to secondary location
move src\run\unlinkedStaticClassFiles.c unlinkedStaticClassFiles.c
move jvm\src\resolution\file\staticClassFiles.c staticClassFiles.c
xcopy src a:\src /i /f /s /a
xcopy lib\src a:\lib\src /i /f /s /a
xcopy romize\src a:\romize\src /i /f /s /a
xcopy jvm\src a:\jvm\src /i /f /s /a
xcopy demo\src a:\demo\src /i /f /s /a
xcopy os\src a:\os\src /i /f /s /a
REM move big files back to original location
move unlinkedStaticClassFiles.c src\run\unlinkedStaticClassFiles.c
move staticClassFiles.c jvm\src\resolution\file\staticClassFiles.c
erase jvm\api\java\lang\*.class
erase jvm\api\java\io\*.class
erase jvm\api\java\util\*.class
erase jvm\api\javax\microedition\io\*.class
erase jvm\api\com\mjvmk\*.class
erase jvm\api\com\mjvmk\util\*.class
xcopy jvm\api a:\jvm\api /i /f /s /a
copy jvm\mjvmk.pxv a:\jvm

copy romize\romize.dsp a:\romize
copy jvm\jvm.dsp a:\jvm
copy mjvmk.dsp a:\
copy mjvmk.dsw a:\
copy demo\demo.dsp a:\demo
copy demo\demo.dsw a:\demo
mkdir a:\demo\demo2
copy demo\demo2\demo.dsw a:\demo\demo2
copy demo\demo2\demo.dsp a:\demo\demo2
copy demo\demoContents.txt a:\demo
copy lib\lib.dsp a:\lib
copy os\os.dsp a:\os

copy notes.txt a:\
copy buildAsm.bat a:\
copy buildListing.bat a:\
copy jvm\romizeApi.txt a:\jvm
copy jvm\demoApi.txt a:\jvm
copy jvm\extraRomizeApi.txt a:\jvm
xcopy c:\projects\mjvmk\boot a:\boot /i /f /s /a
REM move big files to temp location
move site\demo2.zip demo2.zip
move site\demo1.zip demo1.zip
xcopy c:\projects\mjvmk\site a:\site /i /f /s /a
REM move big files back from temp location
move demo2.zip site\demo2.zip
move demo1.zip site\demo1.zip
mkdir a:\ExeToBin
copy ..\ExeToBin\main.c a:\ExeToBin
copy ..\ExeToBin\ExeToBin.dsw a:\ExeToBin
copy ..\ExeToBin\ExeToBin.dsp a:\ExeToBin 
copy backup.bat a:\
