#ifndef core_H_INCLUDED
#define core_H_INCLUDED

#include <windows.h>
#include <shlobj.h>
#include <wininet.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "advapi32.lib")

using namespace std;

// Request UAC
void RequestAdminPrivileges();

// Return user folder
string GetUserFolderPath();

// Fonction pour télécharger un fichier depuis une URL
bool DownloadFile(const string& url, const string& localPath);

// Fonction pour lire la liste des fichiers depuis files.txt
vector<string> GetFileList(const string& filePath);

#endif
