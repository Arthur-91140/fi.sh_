#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include <cstdio>

#pragma comment(lib, "rpcrt4.lib")

using namespace std;

// Fonction pour lire l'UUID depuis un fichier INI existant
void readUUIDFromFile() {
    ifstream file("system_info.dat");
    if (!file.is_open()) return;

    string line;
    while (getline(file, line)) {
        if (line.find("UUID=") != string::npos) {
            globalUUID = line.substr(5); // Récupère l'UUID (après "UUID=")
        }
    }
    file.close();
}

// Fonction pour générer un UUID localement
string getUUID() {
    UUID uuid;
    UuidCreate(&uuid);
    char buffer[37]; // 36 caractères + '\0'
    snprintf(buffer, sizeof(buffer),
             "%08x-%04x-%04x-%04x-%02x%02x%02x%02x%02x%02x",
             uuid.Data1, uuid.Data2, uuid.Data3,
             (uuid.Data4[0] << 8) | uuid.Data4[1],
             uuid.Data4[2], uuid.Data4[3], uuid.Data4[4],
             uuid.Data4[5], uuid.Data4[6], uuid.Data4[7]);
    return string(buffer);
}