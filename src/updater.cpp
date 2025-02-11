#include <windows.h>
#include <shlobj.h>
#include <wininet.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include "../include/install.h"
#include "../include/command.h"

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "advapi32.lib")

using namespace std;

void killProcess(const char* processName) {
    string command = "taskkill /IM ";
    command += processName;
    command += " /F";
    system(command.c_str());
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

    killProcess("fish.exe");

    return 0;
}