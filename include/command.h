#include <windows.h>
#include <wininet.h>
#include <shlobj.h>
#include <cstdlib>
#include <tchar.h>
#include <thread>
#include <vector>
#include <chrono>
#include <string>
#include <fstream>
#include <iostream>
#include "../include/core.h"

using namespace std;

#pragma comment(lib, "wininet.lib")

// =============================================================
// Prototypes relatifs au commandes �x�cutables par le main
// =============================================================

// Fonction pour extraire une commande et ses arguments
//string parseCommand(const string& input, string& command, string& argument);

// Fonction pour �teindre l'ordinateur
void shtd();

// Fonction pour �ffectuer un windows + d
void wind();

// Affiche une bo�te de dialogue d'alerte.
void ShowMessage();

// Fonction pour ex�cuter une commande PowerShell et retourner le r�sultat.
string ExecuteShellCommand(const string& command);