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

// Check if the user entered 0 to go back
bool isGoBackInput(const string& input)
{
    return input == "0";
}

// Read and validate an integer within a given range
int readInteger(const string& prompt, int minimum, int maximum)
{
    while (true)
    {
        string input;

        cout << prompt;
        getline(cin, input);

        // Check for empty input
        if (input.empty())
        {
            cout << "Error: Input cannot be empty.\n";
            continue;
        }

        // Allow 0 to return to the previous menu
        if (isGoBackInput(input))
            return 0;

        try
        {
            size_t processedCharacters = 0;
            int value = stoi(input, &processedCharacters);

            // Check if the input is a valid number within the range
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

// Handle login for managers and customers
bool login(int role, string &loggedInUser)
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

        // Read username
        cout << "Enter your username [0 to go back]: ";
        getline(cin, username);

        if (username.empty())
        {
            cout << "Error: Username cannot be empty.\n";
            continue;
        }

        // Return to previous menu
        if (isGoBackInput(username))
        {
            return false;
        }

        // Read password
        cout << "Password: ";
        getline(cin, password);

        ifstream inFile;

        // Open the correct account file based on the role
        // Role 1 = manager, Role 2 = customer
        if (role == 1)
        {
            inFile.open("managerData.txt");
        }
        else
        {
            inFile.open("customerData.txt");
        }

        // Check if the file opened successfully
        if (inFile.fail())
        {
            cout << "Error: Data file could not be opened.\n";
            return false;
        }

        // Search for the entered username
        while (inFile >> fileUsername >> filePassword)
        {
            if (fileUsername == username)
            {
                usernameFound = true;
                break;
            }
        }

        inFile.close();

        // Check if the username exists
        if (!usernameFound)
        {
            cout << "Error: Username does not exist.\n\n";
            continue;
        }

        // Check if the password matches
        if (password != filePassword)
        {
            cout << "Error: Wrong password.\n\n";
            continue;
        }

        // Store the logged-in customer username
        if (role == 2)
            loggedInUser = username;

        return true;
    }
}

// Pause the program until the user presses Enter
void EnterToContinue(){
    cout << "\nPress Enter to continue...";
    string dummy;
    getline(cin, dummy);
}

// Clear the console screen
void clearScreen()
{
    system("cls");
}