#include "employee.h"
#include "scheduler.h"
#include "utilities.h"

#include <iostream>
#include <limits>
#include <vector>

using namespace std;

void empHomeScreen()
{
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (!login(1))
    {
        return;
    }

    vector<Room> rooms;
    vector<Booking> bookings;
    loadSchedulerDemoData(rooms, bookings);
    employeeSchedulerMenu(rooms, bookings);
}
