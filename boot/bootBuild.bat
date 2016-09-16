del a:\*.img
"c:\program files\nasm\nasmw.exe" -f bin -o ..\lib\out\x86\psBoot.bin ..\lib\src\ps\x86\psBoot.asm
debug ..\lib\out\x86\psBoot.bin < debugCommand.txt
