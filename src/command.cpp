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
// Fonction relatives au commandes éxécutables par le main
// =============================================================

/*
string parseCommand(const string& input, string& command, string& argument) {
	int pos;

	pos = (input.find_first_of(' '));

	argument = 
}
*/

// Fonction pour éteindre l'ordinateur
void shtd() {
    system("shutdown /s /t 0");
}

// Fonction pour éffectuer un WIN + D
void wind() {
    keybd_event(VK_LWIN, 0, 0, 0);
    keybd_event('D', 0, 0, 0);
    keybd_event('D', 0, KEYEVENTF_KEYUP, 0);
    keybd_event(VK_LWIN, 0, KEYEVENTF_KEYUP, 0);
}