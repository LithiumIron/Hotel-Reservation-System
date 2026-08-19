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
    string username,password,fileUsername,filePassword;

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
    cout << "Welcome to Hotel Reservation System!\n";
    cout << "Are you:\n";
    cout << "[1] An Employee\n";
    cout << "[2] A Customer\n";
    cout << "[0] Exit\n";

    int choice = readInteger("Enter your choice: ", 0, 2);

    if (choice==0)
    {
        exit(0);
    }
    
    else if (choice == 1)
    {
        empHomeScreen();
        mainMenu(1);
    }
    else
    {
        custHomeScreen();
        mainMenu(2);
    }
}

void mainMenu(int role)
{
    while (true)
    {
        cout << "\n====================================\n";
        cout << "     HOTEL RESERVATION SYSTEM\n";
        cout << "====================================\n";

        if (role == 1)
        {
            // Employee menu (numbered)
            cout << "[1] View Profile\n";
            cout << "[2] View Schedule\n";
            cout << "[0] Return\n";
            cout << "====================================\n";

            int choice = readInteger(
                "Enter your choice: ", 0, 2);

            if (choice == 0) return;

            if (choice == 1)
            {
                //view profile
            }
            else if (choice == 2)
            {
                vector<Room> rooms;
                vector<Booking> bookings;
                loadSchedulerDemoData(rooms, bookings);
                employeeSchedulerMenu(rooms, bookings);
            }
        }
        else
        {
            // Customer menu (lettered)
            cout << "[A] View Profile\n";
            cout << "[B] View Schedule\n";
            cout << "[C] Booking\n";
            cout << "[D] View Previous Booking Record\n";
            cout << "[E] Cancel Booking\n";
            cout << "[Z] Return\n";
            cout << "====================================\n";

            cout << "Select (A-E, Z to return): ";
            string input;
            getline(cin, input);

            char choice = ' ';
            if (input.length() == 1)
            {
                choice = toupper(input[0]);
            }

            if (choice == 'Z') return;

            switch (choice)
            {
            case 'A':
                //view profile
                break;

            case 'B':
            {
                vector<Room> rooms;
                vector<Booking> bookings;
                loadSchedulerDemoData(rooms, bookings);
                vector<Booking> saved =
                    loadSavedBookings();
                for (const Booking& b : saved)
                {
                    bookings.push_back(b);
                }
                customerSchedulerMenu(rooms, bookings);
                break;
            }

            case 'C':
                bookingScreen();
                break;

            case 'D':
                viewPreviousBookings();
                break;

            case 'E':
                cancelBooking();
                break;

            default:
                cout << "Invalid. "
                     << "Please enter A-E or Z.\n";
                break;
            }
        }
    }
}