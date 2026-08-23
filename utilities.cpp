#include "booking.h"
#include "customer.h"
#include "employee.h"
#include "utilities.h"
#include "scheduler.h"

#include <cctype>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

string loggedInUser;

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
    string username, password;
    string fileUsername, filePassword;

    cout << "\nLogin\n";

    while (true)
    {
        bool usernameFound = false;

        cout << "Enter your username [Z to go back]: ";
        getline(cin, username);

        if (username.empty())
        {
            cout << "Error: Username cannot be empty.\n";
            continue;
        }

        if (toupper(static_cast<unsigned char>(username[0])) == 'Z')
        {
            return false;
        }

        cout << "Password: ";
        getline(cin, password);

        ifstream inFile;

        // Role 1 = employee, Role 2 = customer
        if (role == 1)
        {
            inFile.open("employeeData.txt");
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
 