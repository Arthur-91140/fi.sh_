#include <windows.h>
#include <shlobj.h>
#include <wininet.h>
#include <cstdlib>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include "../include/core.h"

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "advapi32.lib")

using namespace std;

// =============================================================
// Fonction du coeur du programme (fonction primaire du syst�me)
// De pr�f�rence �viter de toucher aux diff�rentes fonctions
// =============================================================

void RequestAdminPrivileges() {
    BOOL isAdmin = FALSE;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID pAdminGroup;
    if (AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pAdminGroup)) {
        CheckTokenMembership(NULL, pAdminGroup, &isAdmin);
        FreeSid(pAdminGroup);
    }

    if (!isAdmin) {
        // Relance le programme avec les droits admin
        TCHAR szPath[MAX_PATH];
        GetModuleFileName(NULL, szPath, MAX_PATH);

        SHELLEXECUTEINFO sei = { sizeof(sei) };
        sei.lpVerb = TEXT("runas");
        sei.lpFile = szPath;
        sei.hwnd = NULL;
        sei.nShow = SW_SHOWDEFAULT;
        sei.fMask = SEE_MASK_NOCLOSEPROCESS;

        if (!ShellExecuteEx(&sei)) {
            MessageBox(NULL, TEXT("�chec de l'obtention des droits administrateur !"), TEXT("Erreur"), MB_OK);
            exit(1);
        }
        exit(0);
    }
}

// Fonction pour obtenir le dossier utilisateur
string GetUserFolderPath() {
    char path[MAX_PATH];
    if (SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, 0, path) == S_OK) {
        return string(path);
    }
    return "";
}

// Fonction pour tuer un processus (requiert UAC)
void killProcess(const char* processName) {
    RequestAdminPrivileges();
    string command = "taskkill /IM ";
    command += processName;
    command += " /F";
    system(command.c_str());
}

// Fonction pour t�l�charger un fichier depuis une URL
bool DownloadFile(const string& url, const string& localPath) {
    HINTERNET hInternet = InternetOpen("Installer", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) return false;

    HINTERNET hUrl = InternetOpenUrl(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (!hUrl) {
        InternetCloseHandle(hInternet);
        return false;
    }

    char buffer[4096];
    DWORD bytesRead;
    HANDLE hFile = CreateFile(localPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        InternetCloseHandle(hUrl);
        InternetCloseHandle(hInternet);
        return false;
    }

    while (InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        DWORD bytesWritten;
        WriteFile(hFile, buffer, bytesRead, &bytesWritten, NULL);
    }

    CloseHandle(hFile);
    InternetCloseHandle(hUrl);
    InternetCloseHandle(hInternet);
    return true;
}

// Fonction pour lire la liste des fichiers depuis files.txt
vector<string> GetFileList(const string& filePath) {
    vector<string> fileList;
    ifstream file(filePath);
    string line;
    while (getline(file, line)) {
        fileList.push_back(line);
    }
    return fileList;
}
