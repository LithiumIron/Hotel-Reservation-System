
#include <string>
#pragma once

// Currently logged-in customer username
extern std::string loggedInUser;

//validation can reuse
int readInteger(const std::string& prompt, int minimum, int maximum);

bool login(int role);

void roleSelection();

void mainMenu(int role);
