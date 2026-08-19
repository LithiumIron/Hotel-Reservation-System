#include "booking.h"
#include "customer.h"
#include "utilities.h"
#include "tempBook.h"

#include <fstream>
#include <iostream>
#include <limits>
#include <string>
using namespace std;

bool signup()
{
    string username;
    string password;
    string password2;

    cout << "Sign Up\n";
    while (true)
    {
        cout << "Create your username [999 to go back]: ";
        getline(cin, username);

        if (username == "999")
        {
            return false;
        }

        if (username.empty())
        {
            cout << "Error: Username cannot be empty.\n\n";
            continue;
        }

        // Check if username already exists
        {
            ifstream inFile("customerData.txt");
            if (inFile)
            {
                string existingUser, existingPass;
                bool found = false;
                while (inFile >> existingUser >> existingPass)
                {
                    if (existingUser == username)
                    {
                        found = true;
                        break;
                    }
                }
                inFile.close();
                if (found)
                {
                    cout << "Error: Username already exists. "
                         << "Please choose another.\n\n";
                    continue;
                }
            }
        }

        cout << "Password: ";
        getline(cin, password);

        cout << "Confirm Password: ";
        getline(cin, password2);

        if (password != password2)
        {
            cout << "Invalid input. Passwords do not match.\n\n";
        }
        else if (password.length() < 8)
        {
            cout << "Invalid Input. Password must contain at least 8 characters.\n\n";
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

    // Auto-login after successful signup
    loggedInUser = username;

    return true;
}

void custHomeScreen()
{
    while (true)
    {
        cout << "\nCustomer Menu\n";
        cout << "[1] Login\n";
        cout << "[2] Sign up\n";
        cout << "[0] Return\n";

        int choice = readInteger("Enter your choice: ", 0, 2);

        if (choice == 0)
        {
            return;
        }

        if (choice == 1)
        {
            if (login(2))
            {
                return;
            }
        }
        else if (choice == 2)
        {
            if (signup())
            {
                return;
            }
        }
    }
}
int main()
{
    while (true){
        roleSelection();
    }
    

    return 0;
}