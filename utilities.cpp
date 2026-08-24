#include "booking.h"
#include "customer.h"
#include "manager.h"
#include "utilities.h"
#include "scheduler.h"

#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

string loggedInUser;

bool isGoBackInput(const string& input)
{
    return input == "0";
}

int readInteger(const string& prompt, int minimum, int maximum)
{
    while (true)
    {
        string input;

        cout << prompt;
        getline(cin, input);

        if (input.empty())
        {
            cout << "Error: Input cannot be empty.\n";
            continue;
        }

        if (isGoBackInput(input))
        {
            if (minimum <= 0 && maximum >= 0)
            {
                return 0;
            }

            cout << "Invalid input. Enter a number from "
                << minimum << " to " << maximum << ".\n";
            continue;
        }

        try
        {
            size_t processedCharacters = 0;
            int value = stoi(input, &processedCharacters);

            if (processedCharacters == input.length()
                && value >= minimum
                && value <= maximum)
            {
                return value;
            }
        }
        catch (...)
        {
            cout << "Error: Please enter a valid number.\n";
            continue;
        }

        cout << "Invalid input. Enter a number from "
             << minimum << " to " << maximum << ".\n";
    }
}

bool login(int role)
{
    clearScreen();
    string username, password;
    string fileUsername, filePassword;

    cout << "\n====================================\n";
    cout << "               LOGIN\n";
    cout << "====================================\n";

    while (true)
    {
        bool usernameFound = false;

        cout << "Enter your username [0 to go back]: ";
        getline(cin, username);

        if (username.empty())
        {
            cout << "Error: Username cannot be empty.\n";
            continue;
        }

        if (isGoBackInput(username))
        {
            return false;
        }

        cout << "Password: ";
        getline(cin, password);

        ifstream inFile;

        // Role 1 = manager, Role 2 = customer
        if (role == 1)
        {
            inFile.open("managerData.txt");
        }
        else
        {
            inFile.open("customerData.txt");
        }

        if (inFile.fail())
        {
            cout << "Error: Data file could not be opened.\n";
            return false;
        }

        while (inFile >> fileUsername >> filePassword)
        {
            if (fileUsername == username)
            {
                usernameFound = true;
                break;
            }
        }

        inFile.close();

        if (!usernameFound)
        {
            cout << "Error: Username does not exist.\n\n";
            continue;
        }

        if (password != filePassword)
        {
            cout << "Error: Wrong password.\n\n";
            continue;
        }

        if (role == 2)
            loggedInUser = username;

        return true;
    }
}

void EnterToContinue(){
    cout << "\nPress Enter to continue...";
    string dummy;
    getline(cin, dummy);
}

void clearScreen()
{
    system("cls");
}
 
