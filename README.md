# fi.sh_

A simple Command & Control solution

For compiling "main" : `g++ .\main.cpp -o fish.exe -mwindows -lwininet`  
For compiling "installer" : `g++ -o .\installer.exe installer.cpp -static -lwininet`  
For compiling "updater" : `g++ -o .\updater.exe updater.cpp -mwindows -lwininet`  
For compiling "uninstaller" : `g++ -o uninstaller.exe uninstaller.cpp -static -municode -Wl,--subsystem,windows`