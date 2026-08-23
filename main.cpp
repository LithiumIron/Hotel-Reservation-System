#include <iostream>

#include "utilities.h"
#include "manager.h"
#include "customer.h"
#include "booking.h"
#include "scheduler.h"
#include "report.h"
#include "vip.h"
using namespace std;

void roleSelection()
{
    clearScreen();
    cout << "\n========================================\n";
    cout << "  Welcome to Hotel Reservation System!\n";
    cout << "========================================\n";
    cout << "Are you:\n";
    cout << "----------------------------------------\n";
    cout << "[1] An Manager\n";
    cout << "[2] A Customer\n";
    cout << "[0] Exiting\n";

    int choice = readInteger("Enter your choice: ", 0, 2);

    if (choice==0)
    {
        exit(0);
    }
    
    else if (choice == 1)
    {
        if(empHomeScreen()==true) {
            login(1);
            mainMenu(1);
        }
        
    }
    else
    {
        loggedInUser.clear();
        custHomeScreen();
        if (!loggedInUser.empty())
        {
            mainMenu(2);
        }
    }
}

void mainMenu(int role)
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
            cout << "[3] Customer Details\n";
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
            cout << "[2] Edit Profile\n";
            cout << "[3] View VIP Benefits\n";
            cout << "[4] Purchase VIP Membership\n";
            cout << "[5] Booking\n";
            cout << "[6] View Previous Booking Record\n";
            cout << "[7] Cancel Booking\n";
            cout << "[8] View Room Availability\n";
            cout << "[9] View Room Location Guide\n";
            cout << "[0] Return\n";
            cout << "====================================\n";

            int choice = readInteger(
                "Enter your choice: ", 0, 9);

            if (choice == 0){
                loggedInUser.clear();
                return;
            }

            if (choice == 1) viewCustomerProfile();
            else if (choice == 2) editCustomerProfile();
            else if (choice == 3) viewVIPBenefits();
            else if (choice == 4) purchaseVIPMembership();
            else if (choice == 5) bookingScreen();
            else if (choice == 6) viewPreviousBookings();
            else if (choice == 7) cancelBooking();
            else if (choice == 8)
            {
                vector<Room> rooms;
                vector<Booking> unusedDemoBookings;
                loadSchedulerDemoData(rooms, unusedDemoBookings);

                // Load real bookings so availability reflects actual reservations.
                vector<Booking> bookings = loadSavedBookings();

                customerSchedulerMenu(rooms, bookings);
            }
            else if (choice == 9)
            {
                vector<Room> rooms;
                vector<Booking> unusedDemoBookings;
                loadSchedulerDemoData(rooms, unusedDemoBookings);

                viewRoomLocationGuide(rooms);
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