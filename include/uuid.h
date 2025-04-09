#ifndef UUID_H_INCLUDED
#define UUID_H_INCLUDED

#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include <cstdio>

#pragma comment(lib, "rpcrt4.lib")

using namespace std;

// Fonction pour lire l'UUID depuis un fichier INI existant
void readUUIDFromFile()

// Fonction pour générer un UUID localement
string getUUID();

#endif