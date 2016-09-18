SET NASM_DIR=C:\Program Files (x86)\nasm-2.12.02-win32\nasm-2.12.02
SET VCINSTALLDIR=C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\
SET PROJECT_DIR=c:\vstudio\git\mjvmk

cd %NASM_DIR%

nasm -f bin -o %PROJECT_DIR%\lib\out\x86\psBoot.bin %PROJECT_DIR%\lib\src\ps\x86\psBoot.asm

cd %VCINSTALLDIR%bin

ml /c /coff "/Fl%PROJECT_DIR%\lib\out\x86\psCpu.lst" /Sa /Fo%PROJECT_DIR%\lib\out\x86\psCpu.obj %PROJECT_DIR%\lib\src\ps\x86\psCpu.asm

ml /c /coff /Fl%PROJECT_DIR%\lib\out\x86\psEntry.lst /Sa /Fo%PROJECT_DIR%\lib\out\x86\psEntry.obj %PROJECT_DIR%\lib\src\ps\x86\psEntry.asm


