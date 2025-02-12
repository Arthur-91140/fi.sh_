#include <windows.h>
#include <shlobj.h>
#include <wininet.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include "../include/core.h"
#include "../include/command.h"

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "advapi32.lib")

using namespace std;



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

    killProcess("fish.exe");

    return 0;
}