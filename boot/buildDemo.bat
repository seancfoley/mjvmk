erase ..\demo\out\x86\*.img
..\..\ExeToBin\Release\ExeToBin ..\demo\out\x86\demo.exe ..\demo\out\x86\demo.img
erase a:\*.img
copy ..\demo\out\x86\demo.img a:\
