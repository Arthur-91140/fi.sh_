#ifndef core_H_INCLUDED
#define core_H_INCLUDED

#include <windows.h>
#include <shlobj.h>
#include <wininet.h>
#include <cstdlib>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <tchar.h>
#include <sstream>
#include <stdexcept>
#include <algorithm>

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "user32.lib")

using namespace std;

// =============================================================
// Header du coeur du programme (fonction primaire du syst�me)
// De pr�f�rence �viter de toucher aux diff�rents prototypes
// =============================================================


// Request UAC
void RequestAdminPrivileges();

// Return user folder
string GetUserFolderPath();

// Fonction pour t�l�charger un fichier depuis une URL
bool DownloadFile(const string& url, const string& localPath);

// Fonction pour lire la liste des fichiers depuis files.txt
vector<string> GetFileList(const string& filePath);

// Fonction pour tuer un processus (requiert UAC)
void killProcess(const char* processName);

// Fonction pour uploader le fichier sur un serveur FTP
bool uploadFileToFTP(const char* ftpServer, const char* username, const char* password, const char* localFile, const char* remoteFile);

#endif
