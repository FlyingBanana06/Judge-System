// Utils.cpp

#include <iostream>
#include <string>
#include "Utils.hpp"

bool promptYesNo() {
    while (true) {
        std::string input;
        std::cin >> input;
        if (input == "y") return 1;
        if (input == "n") return 0;

        std::cout << "Please enter 'y' or 'n'. Try again.\n";
    }
}

#ifdef _WIN32
#include <windows.h>

void ClearScreen() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD count;
    DWORD cellCount;
    COORD homeCoords = {0, 0};

    if (hConsole == INVALID_HANDLE_VALUE) return;
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) return;

    cellCount = csbi.dwSize.X * csbi.dwSize.Y;

    FillConsoleOutputCharacter(hConsole, ' ', cellCount, homeCoords, &count);
    FillConsoleOutputAttribute(hConsole, csbi.wAttributes, cellCount, homeCoords, &count);
    SetConsoleCursorPosition(hConsole, homeCoords);
}
#else
#include <stdio.h>
void ClearScreen() {
    printf("\033[3J\033[H\033[2J");
    fflush(stdout);
}
#endif