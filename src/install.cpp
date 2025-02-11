#include <windows.h>
#include <shlobj.h>
#include <wininet.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include "../include/install.h"

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "advapi32.lib")

using namespace std;


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