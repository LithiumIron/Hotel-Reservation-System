#include "customer.h"
#include "employee.h"
#include "utilities.h"

#include <cctype>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

int numberValidation(const string& userInput, int limit)
{
    if (userInput.empty())
    {
        cout << "Error: Input cannot be empty.\n";
        return 1;
    }

    for (char character : userInput)
    {
        if (!isdigit(static_cast<unsigned char>(character)))
        {
            cout << "Error: Please enter a number from 1 to "
                << limit << ".\n";
            return 1;
        }
    }

    try
    {
        int number = stoi(userInput);

        if (number < 1 || number > limit)
        {
            cout << "Error: Please enter a number from 1 to "
                << limit << ".\n";
            return 1;
        }
    }
    catch (...)
    {
        cout << "Error: The number entered is too large.\n";
        return 1;
    }

    return 0;
}

bool login(int role)
{
    string username;
    string password;
    string fileUsername;
    string filePassword;

    cout << "\nLogin\n";

    while (true)
    {
        bool usernameFound = false;

        cout << "Enter your username [999 to go back]: ";
        getline(cin, username);

        if (username == "999")
        {
            return false;
        }

        cout << "Password: ";
        getline(cin, password);

        ifstream inFile;

        // Role 1 represents employee; role 2 represents customer.
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

        return true;
    }
}

void roleSelection()
{
    string userInput;

    cout << "Welcome to Hotel Reservation System!\n";
    cout << "Are you:\n";
    cout << "[1] An Employee\n";
    cout << "[2] A Customer\n";

    do
    {
        cout << "Enter your choice: ";
        cin >> userInput;
    } while (numberValidation(userInput, 2) != 0);

    if (stoi(userInput) == 1)
    {
        empHomeScreen();
    }
    else
    {
        custHomeScreen();
    }
}
