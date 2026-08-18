
#include <string>
#pragma once

//validation can reuse
int readInteger(const std::string& prompt, int minimum, int maximum);

bool login(int role);

void roleSelection();

void mainMenu(int role);
