#define system_info_H_INCLUDED
#ifndef SYSTEM_INFO_H_INCLUDED

#include <windows.h>
#include <wininet.h>
#include <wbemidl.h>
#include <fstream>
#include <iostream>
#include <shlwapi.h>
#include <iphlpapi.h>
#include <rpc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "../include/uuid.h"

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "rpcrt4.lib")
#pragma comment(lib, "uuid.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")

using namespace std;

extern string globalUUID;

// Fonction pour récupérer le nom du GPU via WMI
string getGPUName();

// Fonction pour créer le fichier system_info.dat et y écrire toutes les infos
void createSystemInfoFile();

#endif // SYSTEM_INFO_H_INCLUDED