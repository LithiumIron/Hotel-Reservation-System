#include "booking.h"
#include "customer.h"
#include "utilities.h"
#include "vip.h"
#include "scheduler.h"

#include <fstream>
#include <iostream>
#include <limits>
#include <iomanip>
#include <cctype>
#include <string>
#include <vector>
#include <utility>
using namespace std;

// Handle customer sign up
bool signup(string& loggedInUser)
{
    clearScreen();
    string username;
    string password;
    string password2;

    cout << "\n====================================\n";
    cout << "             SIGN UP\n";
    cout << "====================================\n";
    while (true)
    {
        cout << "Create your username [0 to go back]: ";
        getline(cin, username);

        if (username.empty())
        {
            cout << "Error: Username cannot be empty.\n\n";
            continue;
        }

        if (isGoBackInput(username)) 
            return false;

        // Check existing username
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

        // Validate password
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

    // Save new account
    ofstream outFile("customerData.txt", ios::app);

    if (outFile.fail())
    {
        cout << "Error opening the customer data file.\n";
        return false;
    }

    outFile << username << '\t' << password << '\n';
    outFile.close();

    // Log in automatically
    loggedInUser = username;

    return true;
}

// Display customer login menu
void custHomeScreen(string& loggedInUser)
{
    while (true)
    {
        clearScreen();
        cout << "\n====================================\n";
        cout << "          Customer Menu\n";
        cout << "====================================\n";
        cout << "[1] Login\n";
        cout << "[2] Sign up\n";
        cout << "[0] Return\n";
        cout << "====================================\n";

        int choice = readInteger("Enter your choice: ", 0, 2);

        if (choice == 0)
        {
            return;
        }

        if (choice == 1)
        {
            if (login(2, loggedInUser))
            {
                return;
            }
        }
        else if (choice == 2)
        {
            if (signup(loggedInUser))
            {
                return;
            }
        }
    }
}

// Display customer profile
void viewCustomerProfile(string& loggedInUser)
{
    clearScreen();

    cout << "\n====================================\n";
    cout << "         CUSTOMER PROFILE\n";
    cout << "====================================\n";

    if (loggedInUser.empty())
    {
        cout << "  No user is currently logged in.\n";
        cout << "====================================\n";
        return;
    }

    // Display profile details
    displayCustomerInfo(loggedInUser);

    // Display VIP details
    displayVIPStatus(loggedInUser);

    // Display booking statistics
    displayBookingStatistics(loggedInUser);

    // Display booking history
    displayBookingHistory(loggedInUser);

    cout << "=========================================\n";

    displayProfileMenu(loggedInUser);
}

// Edit customer profile
void editCustomerProfile(string& loggedInUser)
{
    clearScreen();
    cout << "\n====================================\n";
    cout << "         EDIT PROFILE\n";
    cout << "====================================\n";

    if (loggedInUser.empty())
    {
        cout << "  No user is currently logged in.\n";
        cout << "====================================\n";
        EnterToContinue();
        return;
    }

    // Load users from file
    vector<pair<string, string>> users;
    ifstream inFile("customerData.txt");
    if (inFile.fail())
    {
        cout << "Error: Could not open customer data file.\n";
        cout << "====================================\n";
        EnterToContinue();
        return;
    }

    string username, password;
    bool found = false;
    while (inFile >> username >> password)
    {
        users.push_back({username, password});
        if (username == loggedInUser)
        {
            found = true;
        }
    }
    inFile.close();

    if (!found)
    {
        cout << "\n  User not found.\n";
        cout << "====================================\n";
        EnterToContinue();
        return;
    }

    // Select profile changes
    cout << "\nWhat would you like to change?\n";
    cout << "[1] Change Username\n";
    cout << "[2] Change Password\n";
    cout << "[3] Change Both\n";
    cout << "[0] Cancel\n";
    cout << "----------------------------------------\n";

    int choice = readInteger("Enter your choice: ", 0, 3);

    if (choice == 0)
    {
        cout << "\n  Edit cancelled.\n";
        cout << "====================================\n";
        EnterToContinue();
        return;
    }

    string newUsername = loggedInUser;
    string newPassword;

    // Change username
    if (choice == 1 || choice == 3)
    {
        cout << "\nEnter current password to verify: ";
        string currentPass;
        getline(cin, currentPass);
        
        // Verify password
        bool passVerified = false;
        for (const auto& user : users)
        {
            if (user.first == loggedInUser && user.second == currentPass)
            {
                passVerified = true;
                break;
            }
        }
        
        if (!passVerified)
        {
            cout << "Incorrect password.\n";
            cout << "====================================\n";
            EnterToContinue();
            return;
        }
        
        cout << "Enter new username: ";
        string newUser;
        getline(cin, newUser);
        
        if (newUser.empty())
        {
            cout << "Username cannot be empty.\n";
            cout << "====================================\n";
            EnterToContinue();
            return;
        }

        // Check new username
        bool usernameExists = false;
        for (const auto& user : users)
        {
            if (user.first == newUser)
            {
                usernameExists = true;
                break;
            }
        }
        
        if (usernameExists)
        {
            cout << "Username already exists. Please choose another.\n";
            cout << "====================================\n";
            EnterToContinue();
            return;
        }

        newUsername = newUser;
    }

    // Change password
    if (choice == 2 || choice == 3)
    {
        cout << "\nEnter current password: ";
        string currentPass;
        getline(cin, currentPass);
        
        // Verify password
        bool passVerified = false;
        for (const auto& user : users)
        {
            if (user.first == loggedInUser && user.second == currentPass)
            {
                passVerified = true;
                break;
            }
        }
        
        if (!passVerified)
        {
            cout << "Incorrect password.\n";
            cout << "====================================\n";
            EnterToContinue();
            return;
        }
        
        cout << "Enter new password (min 8 characters): ";
        getline(cin, newPassword);
        
        if (newPassword.length() < 8)
        {
            cout << "Password must be at least 8 characters.\n";
            cout << "====================================\n";
            EnterToContinue();
            return;
        }
        
        cout << "Confirm new password: ";
        string confirmPassword;
        getline(cin, confirmPassword);
        
        if (newPassword != confirmPassword)
        {
            cout << "Passwords do not match.\n";
            cout << "====================================\n";
            EnterToContinue();
            return;
        }
    }

    // Update user details
    for (auto& user : users)
    {
        if (user.first == loggedInUser)
        {
            if (choice == 1 || choice == 3)
            {
                user.first = newUsername;
            }
            if (choice == 2 || choice == 3)
            {
                user.second = newPassword;
            }
            break;
        }
    }

    // Save updated data
    ofstream outFile("customerData.txt");
    if (outFile.fail())
    {
        cout << "Error: Could not save changes.\n";
        cout << "====================================\n";
        EnterToContinue();
        return;
    }

    for (const auto& user : users)
        outFile << user.first << '\t' << user.second << '\n';
    outFile.close();

    // Update logged in username
    if (choice == 1 || choice == 3)
        loggedInUser = newUsername;

    cout << "\nProfile updated successfully!\n";
    
    if (choice == 1 || choice == 3)
        cout << "Username changed to: " << newUsername << "\n";

    if (choice == 2 || choice == 3)
        cout << "Password changed.\n";

    cout << "====================================\n";
    EnterToContinue();
}

// Display customer information
void displayCustomerInfo(const string& loggedInUser)
{
    ifstream inFile("customerData.txt");

    if (inFile.fail())
    {
        cout << "Error: Could not open customer data file.\n";
        return;
    }

    string username;
    string password;
    bool found = false;

    // Find customer account
    while (inFile >> username >> password)
    {
        if (username == loggedInUser)
        {
            cout << "\n";
            cout << "  " << left << setw(20)
                 << "Username:" << username << "\n";

            cout << "  " << left << setw(20)
                 << "Password:" << "*********" << "\n";

            found = true;
            break;
        }
    }

    inFile.close();

    if (!found)
    {
        cout << "\n  User '" << loggedInUser
             << "' not found in customer records.\n";
    }
}

// Display booking statistics
void displayBookingStatistics(const string& loggedInUser)
{
    vector<Booking> saved = loadSavedBookings();

    int totalBookings = 0;
    int activeBookings = 0;
    int completedBookings = 0;

    // Count customer bookings
    for (const Booking& b : saved)
    {
        if (b.customerId == loggedInUser)
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

    cout << "\n  " << left << setw(20)
         << "Account Status:" << "Active" << "\n";

    cout << "  " << left << setw(20)
         << "Active Bookings:" << activeBookings << "\n";

    cout << "  " << left << setw(20)
         << "Completed Bookings:" << completedBookings << "\n";

    cout << "  " << left << setw(20)
         << "Total Bookings:" << totalBookings << "\n";
}

// Display VIP status
void displayVIPStatus(const string& loggedInUser)
{
    string vipTier = getVIPStatus(loggedInUser);

    cout << "\n" << string(45, '-') << "\n";
    cout << "  VIP MEMBERSHIP\n";
    cout << string(45, '-') << "\n";

    if (vipTier == "None")
    {
        cout << "  " << left << setw(20)
             << "Status:" << "Not a VIP Member" << "\n";

        cout << "  " << left << setw(20)
             << "Action:" << "Purchase VIP to enjoy benefits!" << "\n";

        cout << string(45, '-') << "\n";
        return;
    }

    // Load VIP memberships
    vector<VIPMembership> memberships = loadVIPMemberships();

    for (const VIPMembership& m : memberships)
    {
        if (m.customerId == loggedInUser && m.isActive)
        {
            Date today = getCurrentSystemDate();

            int daysRemaining = 0;
            Date temp = today;

            // Calculate remaining days
            while (compareDates(temp, m.expiryDate) < 0)
            {
                daysRemaining++;
                temp = addDays(temp, 1);
            }

            cout << "  " << left << setw(20)
                 << "Tier:" << m.tier << " VIP" << "\n";

            cout << "  " << left << setw(20)
                 << "Valid Until:" << formatDate(m.expiryDate) << "\n";

            cout << "  " << left << setw(20)
                 << "Days Remaining:" << daysRemaining << " days" << "\n";

            // Set discount
            string discount = "10%";

            if (m.tier == "Silver")
                discount = "10%";
            else if (m.tier == "Gold")
                discount = "20%";
            else if (m.tier == "Platinum")
                discount = "30%";

            cout << "  " << left << setw(20)
                 << "Discount:" << discount << " off bookings" << "\n";

            break;
        }
    }

    cout << string(45, '-') << "\n";
}

// Display booking history
void displayBookingHistory(const string& loggedInUser)
{
    vector<Booking> saved = loadSavedBookings();

    if (saved.empty())
        return;

    cout << "\n  --- Recent Booking History ---\n";

    vector<string> displayedBookingIds;
    int count = 0;

    // Display recent bookings
    for (const Booking& b : saved)
    {
        if (b.customerId == loggedInUser && count < 5)
        {
            bool alreadyDisplayed = false;

            // Check duplicate booking
            for (const string& id : displayedBookingIds)
            {
                if (id == b.bookingId)
                {
                    alreadyDisplayed = true;
                    break;
                }
            }

            if (alreadyDisplayed)
                continue;

            displayedBookingIds.push_back(b.bookingId);

            cout << "  " << b.bookingId << " | "
                 << formatDate(b.checkInDate) << " - "
                 << formatDate(b.checkOutDate) << " | "
                 << b.status << "\n";

            count++;
        }
    }

    if (count == 0)
    {
        cout << "  No booking history found.\n";
    }
}

// Display profile options
void displayProfileMenu(string& loggedInUser)
{
    cout << "\n[1] Edit Profile\n";
    cout << "[2] View VIP Benefits\n";
    cout << "[3] Purchase VIP Membership\n";
    cout << "[0] Back\n";

    int choice = readInteger("Enter your choice: ", 0, 3);

    if (choice == 1)
    {
        editCustomerProfile(loggedInUser);
    }
    else if (choice == 2)
    {
        viewVIPBenefits(loggedInUser);
    }
    else if (choice == 3)
    {
        purchaseVIPMembership(loggedInUser);
    }
}