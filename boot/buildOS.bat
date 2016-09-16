erase ..\os\out\x86\os.img
..\..\ExeToBin\Release\ExeToBin ..\os\out\x86\os.exe ..\os\out\x86\os.img
erase a:\*.img
copy ..\os\out\x86\os.img a:\
