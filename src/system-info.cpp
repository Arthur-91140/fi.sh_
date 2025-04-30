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

#include "../include/system-info.h"
#include "../include/uuid.h"
#include "../include/core.h"

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "rpcrt4.lib")
#pragma comment(lib, "uuid.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")

using namespace std;

string globalUUID;

// Fonction pour récupérer le nom du GPU via WMI
string getGPUName() {
    string gpuName = "Inconnu";
    HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres))
        return gpuName;

    hres = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT,
                                RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
    if (FAILED(hres)) {
        CoUninitialize();
        return gpuName;
    }

    IWbemLocator* pLoc = NULL;
    hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
                            IID_IWbemLocator, (LPVOID*)&pLoc);
    if (FAILED(hres)) {
        CoUninitialize();
        return gpuName;
    }

    // Création du namespace WMI
    BSTR ns = SysAllocString(L"ROOT\\CIMV2");
    IWbemServices* pSvc = NULL;
    hres = pLoc->ConnectServer(ns, NULL, NULL, NULL, 0, NULL, NULL, &pSvc);
    SysFreeString(ns);
    if (FAILED(hres)) {
        pLoc->Release();
        CoUninitialize();
        return gpuName;
    }

    // Préparer et exécuter la requête WQL
    BSTR queryLanguage = SysAllocString(L"WQL");
    BSTR query = SysAllocString(L"SELECT * FROM Win32_VideoController");
    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(queryLanguage, query, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);
    SysFreeString(queryLanguage);
    SysFreeString(query);
    if (FAILED(hres)) {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return gpuName;
    }

    IWbemClassObject* pClassObject = NULL;
    ULONG uReturn = 0;
    if (pEnumerator) {
        hres = pEnumerator->Next(WBEM_INFINITE, 1, &pClassObject, &uReturn);
        if (uReturn != 0 && pClassObject) {
            VARIANT vtProp;
            hres = pClassObject->Get(L"Name", 0, &vtProp, 0, 0);
            if (SUCCEEDED(hres) && vtProp.vt == VT_BSTR && vtProp.bstrVal != NULL) {
                // Conversion du BSTR (UTF-16) en chaîne UTF-8
                int len = WideCharToMultiByte(CP_UTF8, 0, vtProp.bstrVal, -1, NULL, 0, NULL, NULL);
                if (len > 0) {
                    char* buffer = new char[len];
                    WideCharToMultiByte(CP_UTF8, 0, vtProp.bstrVal, -1, buffer, len, NULL, NULL);
                    gpuName = buffer;
                    delete[] buffer;
                }
            }
            VariantClear(&vtProp);
            pClassObject->Release();
        }
        pEnumerator->Release();
    }

    pSvc->Release();
    pLoc->Release();
    CoUninitialize();

    return gpuName;
}

// Fonction pour créer le fichier system_info.dat et y écrire toutes les infos
void createSystemInfoFile() {
    string userFolder = GetUserFolderPath();
    string fishFolder = userFolder + "\\fish";
    string filePath = fishFolder + "\\system_info.dat";

    ofstream file(filePath);
    if (!file.is_open()) return;

    // Récupérer l'UUID
    globalUUID = getUUID();

    // Récupérer le nom du PC
    char computerName[MAX_COMPUTERNAME_LENGTH + 1] = {0};
    DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
    GetComputerNameA(computerName, &size);

    // Récupérer le nom de l'utilisateur
    char userName[UNLEN + 1] = {0};
    size = UNLEN + 1;
    GetUserNameA(userName, &size);

    // Récupérer le nom du CPU depuis le registre
    char cpuName[128] = "Inconnu";
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD cpuNameSize = sizeof(cpuName);
        RegQueryValueExA(hKey, "ProcessorNameString", NULL, NULL, (LPBYTE)cpuName, &cpuNameSize);
        RegCloseKey(hKey);
    }

    // Récupérer les informations système (architecture, nombre de processeurs, etc.)
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    // Récupérer la mémoire physique totale
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    GlobalMemoryStatusEx(&memStatus);

    // Récupérer le dossier Windows
    char windowsDir[MAX_PATH] = {0};
    GetWindowsDirectoryA(windowsDir, MAX_PATH);

    // Récupérer la version de Windows
    OSVERSIONINFO osvi;
    ZeroMemory(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionExA(&osvi);

    // Récupérer l'adresse IP
    char ipAddress[16] = "Non détectée";
    PIP_ADAPTER_INFO adapterInfo = NULL;
    ULONG outBufLen = sizeof(IP_ADAPTER_INFO);
    adapterInfo = (PIP_ADAPTER_INFO)malloc(outBufLen);
    if (GetAdaptersInfo(adapterInfo, &outBufLen) == ERROR_BUFFER_OVERFLOW) {
        free(adapterInfo);
        adapterInfo = (PIP_ADAPTER_INFO)malloc(outBufLen);
    }
    if (GetAdaptersInfo(adapterInfo, &outBufLen) == NO_ERROR) {
        strcpy_s(ipAddress, adapterInfo->IpAddressList.IpAddress.String);
    }
    free(adapterInfo);

    // Récupérer le nom du GPU
    string gpuName = getGPUName();

    // Écriture des informations dans le fichier INI
    file << "[OTHER]" << endl;
    file << "UUID=" << globalUUID << endl;
    file << "[General]" << endl;
    file << "Nom_PC=" << computerName << endl;
    file << "Nom_Utilisateur=" << userName << endl;
    file << "[Processeur]" << endl;
    file << "Nom=" << cpuName << endl;
    file << "Architecture=" << sysInfo.wProcessorArchitecture << endl;
    file << "[Memoire]" << endl;
    file << "RAM_Totale=" << (memStatus.ullTotalPhys / (1024 * 1024)) << " MB" << endl;
    file << "[Windows]" << endl;
    file << "Dossier_Windows=" << windowsDir << endl;
    file << "Version=" << osvi.dwMajorVersion << "." << osvi.dwMinorVersion << endl;
    file << "Build=" << osvi.dwBuildNumber << endl;
    file << "[Reseau]" << endl;
    file << "Adresse_IP=" << ipAddress << endl;
    file << "[GPU]" << endl;
    file << "Nom_GPU=" << gpuName << endl;

    file.close();
}