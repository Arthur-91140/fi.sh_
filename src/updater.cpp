#include <windows.h>
#include <shlobj.h>
#include <wininet.h>
#include <urlmon.h>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <iostream>
#include "../include/core.h"

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "urlmon.lib")

namespace fs = std::filesystem;
using namespace std;

const string SERVER_URL = "https://arthur.xn--pruvost-rivire-6jb.fr/cdn/";
const string FILE_LIST = "files.txt";

/*
// Fonction qui retourne le dossier utilisateur
string GetUserFolderPath() {
    char path[MAX_PATH];
    if (SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, 0, path) == S_OK) {
        return string(path);
    }
    return "";
}
*/

/*
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
*/

// Créer une sauvegarde des fichiers
bool backupFiles(const string& fishFolder, const string& backupFolder) {
    try {
        fs::create_directory(backupFolder);
        for (const auto& entry : fs::directory_iterator(fishFolder)) {
            if (entry.path().filename() != "updater.exe") {
                fs::copy(entry.path(), backupFolder / entry.path().filename(),
                    fs::copy_options::overwrite_existing);
            }
        }
        return true;
    }
    catch (const fs::filesystem_error& e) {
        cerr << "Erreur backup : " << e.what() << endl;
        return false;
    }
}

// Supprimer les fichiers sauf updater.exe
bool cleanFolder(const string& fishFolder, const string& backupFolder) {
    try {
        for (const auto& entry : fs::directory_iterator(fishFolder)) {
            string filename = entry.path().filename().string();
            if (filename != "updater.exe" &&
                entry.path().string().find(backupFolder) != 0) {
                fs::remove_all(entry.path());
            }
        }
        return true;
    }
    catch (const fs::filesystem_error& e) {
        cerr << "Erreur nettoyage : " << e.what() << endl;
        return false;
    }
}

// Fonction pour lancer fish.exe
void ExecuteFile(const string& filePath) {
    ShellExecute(NULL, "open", filePath.c_str(), NULL, NULL, SW_HIDE);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

    // Attente de 10s le temps de l'arrêt de fish.exe
    this_thread::sleep_for(chrono::seconds(10));

    string userFolder = GetUserFolderPath();
    if (userFolder.empty()) {
        MessageBox(NULL, "Impossible d'obtenir le dossier utilisateur.", "Erreur", MB_OK | MB_ICONERROR);
        return 1;
    }

    string fishFolder = userFolder + "\\fish";
    string backupFolder = fishFolder + "\\backup_" + to_string(time(nullptr));

    // Créer la sauvegarde
    if (!backupFiles(fishFolder, backupFolder)) {
        MessageBox(NULL, "Erreur lors de la sauvegarde.", "Erreur", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Nettoyer le dossier
    if (!cleanFolder(fishFolder, backupFolder)) {
        MessageBox(NULL, "Erreur lors du nettoyage.", "Erreur", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Télécharger et lire la liste des fichiers
    string fileListPath = fishFolder + "\\" + FILE_LIST;
    if (!DownloadFile(SERVER_URL + FILE_LIST, fileListPath)) {
        MessageBox(NULL, "Échec du téléchargement de files.txt", "Erreur", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Télécharger chaque fichier de la liste
    vector<string> filesToDownload = GetFileList(fileListPath);

        // Dans votre boucle principale
        vector<thread> downloadThreads;

        for (const string& file : filesToDownload) {
            if (file == "updater.exe") {
                continue;
            }
            string url = SERVER_URL + file;
            string localPath = fishFolder + "\\" + file;

            cout << localPath << endl;

            // Création d'un nouveau thread pour chaque téléchargement
            downloadThreads.emplace_back(DownloadFile, url, localPath);
        }

        // Attendre que tous les téléchargements soient terminés
        for (auto& thread : downloadThreads) {
            thread.join();
        }

        this_thread::sleep_for(chrono::seconds(5));

        /*
        if (!DownloadFile(url, localPath)) {
            MessageBox(NULL, ("Échec du téléchargement de " + file).c_str(), "Erreur", MB_OK | MB_ICONERROR);
            continue;
        }
        */


    // Démarre fish.exe
    string fishPath = fishFolder + "\\fish.exe";
    ExecuteFile(fishPath);

    return 0;
}