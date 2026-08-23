#include "manager.h"
#include "scheduler.h"
#include "utilities.h"
#include "booking.h"
#include "vip.h"

#include <iostream>
#include <cctype>
#include <limits>
#include <iomanip>
#include <vector>
#include <fstream>

using namespace std;

bool empHomeScreen()
{
    const string MANAGER_PASSCODE = "1234";
    string enteredPasscode;

    while (true)
    {   
        clearScreen();
        cout << "\n====================================\n";
        cout << "         Manager Access\n";
        cout << "====================================\n";
        cout << "Enter manager passcode [0 to go back]: ";
        getline(cin, enteredPasscode);

        if (isGoBackInput(enteredPasscode)) 
            return false;

        if (enteredPasscode == MANAGER_PASSCODE)
        {
            cout << "\nManager passcode accepted.\n";
            EnterToContinue();
            return true;
        }

        cout << "Invalid manager passcode. Please try again.\n";
        EnterToContinue();
    }
}
void viewManagerProfile()
{
    clearScreen();

    cout << "\n====================================\n";
    cout << "         MANAGER PROFILE\n";
    cout << "====================================\n";

    ifstream inFile("managerData.txt");

    if (inFile.fail())
    {
        cout << "Error: Could not open manager data file.\n";
        cout << "====================================\n";
        EnterToContinue();
        return;
    }

    string username;
    bool found = false;

    while (inFile >> username)
    {
        cout << "  Username : " << username << "\n";
        cout << "  Password : ********\n";
        cout << "  Status   : Active\n";
        cout << "  Role     : Manager\n";

        found = true;
        break;
    }

    inFile.close();

    if (!found)
    {
        cout << "  No manager records found.\n";
    }

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

    vector<VIPMembership> memberships = loadVIPMemberships();
    Date today = getCurrentSystemDate();
    
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
        
        // ✅ CORRECT: Check actual VIP membership
        string vipStatus = "Standard";
        for (const VIPMembership& m : memberships)
        {
            if (m.customerId == username && m.isActive)
            {
                if (compareDates(today, m.expiryDate) <= 0)
                {
                    vipStatus = "⭐ " + m.tier;
                    break;
                }
            }
        }
        
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