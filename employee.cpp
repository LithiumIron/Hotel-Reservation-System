#include "employee.h"
#include "scheduler.h"
#include "utilities.h"
#include "booking.h"

#include <iostream>
#include <cctype>
#include <limits>
#include <iomanip>
#include <vector>
#include <fstream>

using namespace std;

bool empHomeScreen()
{
    const string EMPLOYEE_PASSCODE = "1234";
    string enteredPasscode;

    while (true)
    {   
        clearScreen();
        cout << "\n====================================\n";
        cout << "         Employee Access\n";
        cout << "====================================\n";
        cout << "Enter employee passcode [Z to go back]: ";
        getline(cin, enteredPasscode);

        if (toupper(static_cast<unsigned char>(enteredPasscode[0])) == 'Z') 
            return false;

        if (enteredPasscode == EMPLOYEE_PASSCODE)
        {
            cout << "\nEmployee passcode accepted.\n";
            EnterToContinue();
            return true;
        }

        cout << "Invalid employee passcode. Please try again.\n";
        EnterToContinue();
    }
}
void viewEmployeeProfile()
{
    clearScreen();
    cout << "\n====================================\n";
    cout << "         EMPLOYEE PROFILE\n";
    cout << "====================================\n";

    ifstream inFile("employeeData.txt");
    if (inFile.fail())
    {
        cout << "Error: Could not open employee data file.\n";
        cout << "====================================\n";
        EnterToContinue();
        return;
    }

    string username, password;
    bool found = false;

    cout << "  " << left << setw(20) << "USERNAME" << "\n";
    cout << "  " << string(40, '-') << "\n";

    while (inFile >> username >> password)
    {
        cout << "  " << left << setw(20) << username << setw(20) << password << "\n";
        found = true;
    }

    if (!found)
    {
        cout << "  No employee records found.\n";
    }

    inFile.close();
    cout << "====================================\n";
    EnterToContinue();
}

void viewAllCustomers()
{
    clearScreen();
    cout << "\n====================================\n";
    cout << "         CUSTOMER DETAILS\n";
    cout << "====================================\n";
    cout << "  All registered customers and their booking stats\n\n";

    // Read all customers
    vector<pair<string, string>> customers;
    ifstream inFile("customerData.txt");
    if (inFile.fail())
    {
        cout << "Error: Could not open customer data file.\n";
        cout << "====================================\n";
        EnterToContinue();
        return;
    }

    string username, password;
    while (inFile >> username >> password)
    {
        customers.push_back({username, password});
    }
    inFile.close();

    if (customers.empty())
    {
        cout << "  No customers registered.\n";
        cout << "====================================\n";
        EnterToContinue();
        return;
    }

    // Load all bookings
    vector<Booking> saved = loadSavedBookings();

    // Display customer table
    cout << "  " << left << setw(15) << "USERNAME" 
         << setw(12) << "BOOKINGS" 
         << setw(12) << "ACTIVE" 
         << setw(12) << "COMPLETED" 
         << "VIP STATUS\n";
    cout << "  " << string(65, '-') << "\n";

    for (const auto& customer : customers)
    {
        string username = customer.first;
        int totalBookings = 0;
        int activeBookings = 0;
        int completedBookings = 0;
        
        for (const Booking& b : saved)
        {
            if (b.customerId == username)
            {
                totalBookings++;
                if (b.status == "COMPLETED")
                {
                    completedBookings++;
                }
                else if (b.status != "CANCELLED")
                {
                    activeBookings++;
                }
            }
        }
        
        string vipStatus = "Standard";
        if (totalBookings >= 10)
            vipStatus = "⭐ Platinum";
        else if (totalBookings >= 5)
            vipStatus = "⭐ Gold";
        else if (totalBookings >= 3)
            vipStatus = "⭐ Silver";
        
        cout << "  " << left << setw(15) << username 
             << setw(12) << totalBookings 
             << setw(12) << activeBookings 
             << setw(12) << completedBookings 
             << vipStatus << "\n";
    }

    cout << "\n  " << string(65, '-') << "\n";
    cout << "  Total Customers: " << customers.size() << "\n";
    cout << "  Total Bookings: " << saved.size() << "\n";
    cout << "====================================\n";
    EnterToContinue();
}