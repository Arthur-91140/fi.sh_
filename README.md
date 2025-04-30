# fi.sh_

A simple Command & Control solution

For compiling "main" : `g++ .\src\main.cpp .\src\command.cpp .\src\core.cpp .\src\system-info.cpp .\src\uuid.cpp -o fish.exe -static -mwindows -lwininet -ladvapi32 -lole32 -loleaut32 -liphlpapi -lrpcrt4 -luuid -lshlwapi -lwbemuuid`  
For compiling "installer" : `g++ -o .\installer.exe installer.cpp ../src/core.cpp -static -lwininet -ladvapi32 -lurlmon`  
For compiling "updater" : `g++ -o .\updater.exe updater.cpp -mwindows -lwininet`  
For compiling "uninstaller" : `g++ -o uninstaller.exe uninstaller.cpp -static -municode -Wl,--subsystem,windows`