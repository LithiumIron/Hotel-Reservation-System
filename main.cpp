#include <iostream>

#include "utilities.h"
#include "manager.h"
#include "customer.h"
#include "booking.h"
#include "scheduler.h"
#include "report.h"
using namespace std;

void roleSelection()
{
    clearScreen();
    cout << "\n========================================\n";
    cout << "  Welcome to Hotel Reservation System!\n";
    cout << "========================================\n";
    cout << "Are you:\n";
    cout << "----------------------------------------\n";
    cout << "[1] A Manager\n";
    cout << "[2] A Customer\n";
    cout << "[0] Exiting\n";

    int choice = readInteger("Enter your choice: ", 0, 2);

    if (choice==0)
    {
        exit(0);
    }
    
    else if (choice == 1)
    {
        string managerUser;    
        if (empHomeScreen() && login(1, managerUser))
            mainMenu(1,managerUser);
        
    }
    else
    {
        string currentUser;
        currentUser.clear();
        custHomeScreen(currentUser);
        if (!currentUser.empty())
        {
            mainMenu(2, currentUser);
        }
    }
}

void mainMenu(int role, string &username)
{
    while (true)
    {
        clearScreen();
        cout << "\n====================================\n";
        cout << "     HOTEL RESERVATION SYSTEM\n";
        cout << "====================================\n";

        if (role == 1)
        {
            // Manager menu (numbered)
            cout << "[1] View Profile\n";
            cout << "[2] View Schedule\n";
            cout << "[3] View All Customer Details\n";
            cout << "[4] Report Management\n";
            cout << "[0] Return\n";
            cout << "====================================\n";

            int choice = readInteger(
                "Enter your choice: ", 0, 4);

            if (choice == 0) return;

            if (choice == 1) viewManagerProfile();

            else if (choice == 2)
            {
                vector<Room> rooms;
                vector<Booking> unusedDemoBookings;
                loadSchedulerDemoData(rooms, unusedDemoBookings);

                // Scheduler displays only real bookings saved by Booking Module.
                vector<Booking> bookings = loadSavedBookings();

                managerSchedulerMenu(rooms, bookings);
            }
            else if (choice == 3) viewAllCustomers();
            else if (choice == 4) reportMenu();
            
        }
        else
        {
            // Customer menu (numbered)
            cout << "[1] View Profile\n";
            cout << "[2] Booking\n";
            cout << "[3] View Previous Booking Record\n";
            cout << "[4] Cancel Booking\n";
            cout << "[5] View Room Availability\n";
            cout << "[6] View Room Location Guide\n";
            cout << "[0] Return\n";
            cout << "====================================\n";

            int choice = readInteger(
                "Enter your choice: ", 0, 9);

            if (choice == 0){
                username.clear();
                return;
            }

            if (choice == 1) viewCustomerProfile(username);
            else if (choice == 2) bookingScreen(username);
            else if (choice == 3) viewPreviousBookings(username);
            else if (choice == 4) cancelBooking(username);
            else if (choice == 5)
            {
                vector<Room> rooms;
                vector<Booking> unusedDemoBookings;
                loadSchedulerDemoData(rooms, unusedDemoBookings);

                vector<Booking> bookings = loadSavedBookings();

                customerSchedulerMenu(rooms, bookings);
            }
            else if (choice == 6)
            {
                vector<Room> rooms;
                vector<Booking> unusedDemoBookings;
                loadSchedulerDemoData(rooms, unusedDemoBookings);

                viewRoomLocationGuide(rooms, username);
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