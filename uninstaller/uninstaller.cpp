#include <windows.h>
#include <tlhelp32.h>
#include <shlobj.h>
#include <filesystem>
#include <iostream>
#include <string>
#include "../include/core.h"

using namespace std;
namespace fs = filesystem;

// Fonction pour arreter un processus par son nom
bool KillProcessByName(const wstring& processName) {
    cout << "[INFO] Tentative d'arr�t du processus: " << string(processName.begin(), processName.end()) << endl;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) {
        cout << "[ERROR] Impossible de cr�er un instantan� des processus." << endl;
        return false;
    }

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(PROCESSENTRY32W);

    if (Process32FirstW(hSnap, &pe)) {
        do {
            if (processName == pe.szExeFile) {
                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                if (hProcess) {
                    cout << "[INFO] Processus trouve, arret en cours..." << endl;
                    TerminateProcess(hProcess, 0);
                    CloseHandle(hProcess);
                    cout << "[INFO] Processus arrete avec succes." << endl;
                }
                else {
                    cout << "[WARN] Impossible d'ouvrir le processus." << endl;
                }
            }
        } while (Process32NextW(hSnap, &pe));
    }
    CloseHandle(hSnap);
    return true;
}

// Fonction pour executer une commande systeme
void ExecuteCommand(const wstring& command) {
    cout << "[INFO] Execution de la commande: " << string(command.begin(), command.end()) << endl;
    system(string(command.begin(), command.end()).c_str());
}

// Fonction qui retourne le dossier utilisateur
string GetUserFolderPath() {
    char path[MAX_PATH];
    if (SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, 0, path) == S_OK) {
        return string(path);
    }
    return "";
}

// Fonction pour supprimer un dossier et son contenu
bool supprimerDossier(const string& chemin) {
    try {
        if (fs::exists(chemin) && fs::is_directory(chemin)) {
            fs::remove_all(chemin); // Supprime le dossier et son contenu
            return true;
        }
        else {
            cerr << "Le dossier n'existe pas ou ce n'est pas un r�pertoire." << endl;
            return false;
        }
    }
    catch (const fs::filesystem_error& e) {
        cerr << "Erreur lors de la suppression : " << e.what() << endl;
        return false;
    }
}

// Fonction pour supprimer la cl� de registre
bool DeleteRunKeyFish() {
    HKEY hKey;
    LONG result;

    // Ouvrir la cle de registre
    result = RegOpenKeyExA(
        HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0,
        KEY_SET_VALUE,
        &hKey
    );

    if (result != ERROR_SUCCESS) {
        return false;
    }

    // Supprimer la valeur "fish"
    result = RegDeleteValueA(hKey, "fish");

    // Fermer la cle
    RegCloseKey(hKey);

    // Retourner true si la suppression a r�ussi
    return (result == ERROR_SUCCESS);
}

int main() {
    cout << "[INFO] Verification des privileges administrateur..." << endl;
    BOOL isAdmin = FALSE;

    // Verifier et demander les privileges administrateur
    RequestAdminPrivileges();

    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;
    if (AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0, &AdministratorsGroup)) {
        CheckTokenMembership(NULL, AdministratorsGroup, &isAdmin);
        FreeSid(AdministratorsGroup);
    }

    if (!isAdmin) {
        cout << "[ERROR] Ce programme doit etre execute en mode administrateur." << endl;

        return 1;
    }

    cout << "[INFO] Debut de la suppression du programme fish." << endl;

    // Arreter le processus fish.exe
    KillProcessByName(L"fish.exe");

    // Executer la commande 'sc delete fish'
    ExecuteCommand(L"sc delete fish");

    // Supprimer la cle de registre
    DeleteRunKeyFish();

    // Recuperer le chemin du dossier utilisateur et supprimer le dossier 'fish'
    string userFolder = GetUserFolderPath();
    if (userFolder.empty()) {
        MessageBox(NULL, "Impossible d'obtenir le dossier utilisateur.", "Erreur", MB_OK | MB_ICONERROR);
        return 1;
    }

    string fishFolder = userFolder + "\\fish";

    supprimerDossier(fishFolder);

    cout << "[INFO] Le programme fish a ete supprime avec succes." << endl;

    return 0;
}
