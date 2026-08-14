#include "booking.h"
#include "scheduler.h"

#include <vector>

using namespace std;

void bookingScreen()
{
    vector<Room> rooms;
    vector<Booking> bookings;
    loadSchedulerDemoData(rooms, bookings);
    autoReleaseExpiredBookings(bookings, Date{ 14, 8, 2026 });
    customerSchedulerMenu(rooms, bookings);
}
