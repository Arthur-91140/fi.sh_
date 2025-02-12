#ifndef core_H_INCLUDED
#define core_H_INCLUDED

#include <windows.h>
#include <shlobj.h>
#include <wininet.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "advapi32.lib")

using namespace std;

// Request UAC
void RequestAdminPrivileges();

// Return user folder
string GetUserFolderPath();

#endif
