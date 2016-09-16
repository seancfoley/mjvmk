"c:\program files\nasm\nasmw.exe" -f bin -o lib\out\x86\psBoot.bin lib\src\ps\x86\psBoot.asm
"c:\program files\win98ddk\bin\win98\ml.exe" /c /coff /Fllib\out\x86\psCpu.lst /Sa /Sc /Folib\out\x86\psCpu.obj lib\src\ps\x86\psCpu.asm
"c:\program files\win98ddk\bin\win98\ml.exe" /c /coff /Fllib\out\x86\psEntry.lst /Sa /Sc /Folib\out\x86\psEntry.obj lib\src\ps\x86\psEntry.asm
