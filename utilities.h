
#pragma once

#include <string>

//validation can reuse
int readInteger(const std::string& prompt, int minimum, int maximum);

// Returns true if the user entered 0 (go back). Case-safe.
bool isGoBackInput(const std::string& input);

bool login(int role, string &loggedInUser);

void roleSelection();

void mainMenu(int role, std::string& username);

void EnterToContinue();

// Clears the console before displaying a new screen.
void clearScreen();
