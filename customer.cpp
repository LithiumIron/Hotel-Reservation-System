#include "booking.h"
#include "customer.h"
#include "utilities.h"

#include <fstream>
#include <iostream>
#include <string>

using namespace std;

bool signup()
{
    string username;
    string password;
    string password2;

    cout << "Sign Up\n";
    cout << "Create your username [999 to go back]: ";
    getline(cin, username);

    if (username == "999")
    {
        return false;
    }

    while (true)
    {
        cout << "Password: ";
        getline(cin, password);

        cout << "Confirm Password: ";
        getline(cin, password2);

        if (password != password2)
        {
            cout << "Error: Passwords do not match.\n\n";
        }
        else if (password.length() < 8)
        {
            cout << "Error: Password must contain at least 8 characters.\n\n";
        }
        else
        {
            break;
        }
    }

    ofstream outFile("customerData.txt", ios::app);

    if (outFile.fail())
    {
        cout << "Error opening the customer data file.\n";
        return false;
    }

    outFile << username << '\t' << password << '\n';
    outFile.close();

    return true;
}

void custHomeScreen()
{
    while (true)
    {
        string userInput;

        cout << "\nCustomer Menu\n";
        cout << "[1] Login\n";
        cout << "[2] Sign up\n";

        do
        {
            cout << "Enter your choice: ";
            cin >> userInput;
        } while (numberValidation(userInput, 2) != 0);

        cin.ignore();

        if (stoi(userInput) == 1)
        {
            if (login(2))
            {
                break;
            }
        }
        else
        {
            if (signup())
            {
                break;
            }
        }
    }

    bookingScreen();
}

int main()
{
    roleSelection();
    cout << "\nThank you for using the Hotel Reservation System.\n";

    return 0;
}

// View profile
// Edit profile
// Booking history
