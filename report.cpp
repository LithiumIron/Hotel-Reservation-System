#include "report.h"
#include "scheduler.h"
#include "utilities.h"
#include "booking.h"
#include "vip.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <algorithm>

using namespace std;

// ============================================================
// Helper Functions
// ============================================================

bool isPastDate(const Date& date)
{
    Date today = getCurrentSystemDate();
    return compareDates(date, today) < 0;
}

// Check whether a booking occupies a room on a particular date
bool isRoomOccupied(const Booking& booking, const Date& date)
{
    return booking.status != "CANCELLED" &&
           booking.status != "COMPLETED" &&
           compareDates(booking.checkInDate, date) <= 0 &&
           compareDates(date, booking.checkOutDate) < 0;
}

// Count occupied rooms on a date
int countOccupiedOnDate(const vector<Booking>& bookings, const Date& date)
{
    int count = 0;

    for (const Booking& b : bookings)
    {
        if (isRoomOccupied(b, date))
        {
            count++;
        }
    }

    return count;
}

// Calculate number of nights
int calculateNights(const Booking& booking)
{
    int nights = 0;
    Date temp = booking.checkInDate;

    while (compareDates(temp, booking.checkOutDate) < 0)
    {
        nights++;
        temp = addDays(temp, 1);
    }

    return nights;
}

// Find actual room nightly rate
double getRoomNightlyRate(const Booking& booking, const vector<Room>& rooms)
{
    for (const Room& room : rooms)
    {
        if (room.roomId == booking.roomId)
        {
            return room.nightlyRate;
        }
    }

    return 0.0;
}

// Get actual add-on price
double getAddonPrice(const string& addonName)
{
    if (addonName == "Buffet Breakfast")
    {
        return 35.0;
    }
    else if (addonName == "Extra Bed")
    {
        return 10.0;
    }
    else if (addonName == "Gym & Pool VIP Pass")
    {
        return 20.0;
    }
    else if (addonName == "High-Speed WiFi")
    {
        return 40.0;
    }

    return 0.0;
}

// Calculate add-on cost per day from saved addon string
double calculateAddonPerDay(const string& addons)
{
    if (addons.empty())
    {
        return 0.0;
    }

    double total = 0.0;
    istringstream addonStream(addons);
    string addonItem;

    while (getline(addonStream, addonItem, ';'))
    {
        size_t xPos = addonItem.find("x ");

        if (xPos == string::npos)
        {
            continue;
        }

        int quantity = 0;

        try
        {
            quantity = stoi(addonItem.substr(0, xPos));
        }
        catch (...)
        {
            continue;
        }

        string addonName = addonItem.substr(xPos + 2);
        double price = getAddonPrice(addonName);
        total += quantity * price;
    }

    return total;
}

// Calculate revenue for a group of records sharing the same booking ID.
//
// createBookingRecords() creates one Booking record per room,
// but stores the same addonsString in every record.
//
// Room cost = calculated for every room
// Add-on cost = calculated only once
double calculateBookingGroupRevenue(const vector<Booking>& group, const vector<Room>& rooms)
{
    if (group.empty())
    {
        return 0.0;
    }

    const Booking& first = group[0];
    int nights = calculateNights(first);

    if (nights <= 0)
    {
        return 0.0;
    }

    double roomTotal = 0.0;

    // Calculate every room in this booking
    for (const Booking& b : group)
    {
        double nightlyRate = getRoomNightlyRate(b, rooms);
        roomTotal += nightlyRate * nights;
    }

    // Add-ons are stored on every room record,
    // so only count them once.
    double addonTotal = calculateAddonPerDay(first.addons) * nights;

    return roomTotal + addonTotal;
}

// Group bookings by booking ID
map<string, vector<Booking>> groupBookingsById(const vector<Booking>& bookings)
{
    map<string, vector<Booking>> grouped;

    for (const Booking& b : bookings)
    {
        grouped[b.bookingId].push_back(b);
    }

    return grouped;
}

// ============================================================
// Daily Report
// ============================================================

DailyReport generateDailyReport(const Date& date, const vector<Room>& rooms,
                                const vector<Booking>& bookings)
{
    DailyReport report;

    report.date = date;
    report.totalRooms = static_cast<int>(rooms.size());
    report.occupiedRooms = countOccupiedOnDate(bookings, date);
    report.availableRooms = report.totalRooms - report.occupiedRooms;

    report.occupancyRate = report.totalRooms > 0
                               ? (report.occupiedRooms * 100.0 / report.totalRooms)
                               : 0.0;

    report.checkIns = 0;
    report.checkOuts = 0;
    report.totalRevenue = 0.0;
    report.pendingBookings = 0;
    report.cancelledBookings = 0;

    // --------------------------------------------------------
    // Check-ins / check-outs / pending / cancelled
    // --------------------------------------------------------

    for (const Booking& b : bookings)
    {
        if (compareDates(b.checkInDate, date) == 0 && b.status != "CANCELLED")
        {
            report.checkIns++;
        }

        if (compareDates(b.checkOutDate, date) == 0 && b.status != "CANCELLED")
        {
            report.checkOuts++;
        }

        if (b.status == "PENDING" && compareDates(b.checkInDate, date) == 0)
        {
            report.pendingBookings++;
        }

        if (b.status == "CANCELLED" && compareDates(b.checkInDate, date) == 0)
        {
            report.cancelledBookings++;
        }
    }

    // --------------------------------------------------------
    // Revenue
    // --------------------------------------------------------

    map<string, vector<Booking>> grouped = groupBookingsById(bookings);

    for (const auto& pair : grouped)
    {
        const vector<Booking>& group = pair.second;

        if (group.empty())
        {
            continue;
        }

        const Booking& first = group[0];

        // Only count bookings starting on this date
        if (compareDates(first.checkInDate, date) != 0)
        {
            continue;
        }

        // Cancelled bookings generate no revenue
        if (first.status == "CANCELLED")
        {
            continue;
        }

        // Revenue only for paid bookings
        if (!first.paid)
        {
            continue;
        }

        report.totalRevenue += calculateBookingGroupRevenue(group, rooms);
    }

    return report;
}

// ============================================================
// Monthly Report
// ============================================================

MonthlyReport generateMonthlyReport(int month, int year, const vector<Room>& rooms,
                                    const vector<Booking>& bookings)
{
    MonthlyReport report;

    report.month = month;
    report.year = year;
    report.totalRooms = static_cast<int>(rooms.size());

    report.totalBookings = 0;
    report.totalCheckIns = 0;
    report.totalCheckOuts = 0;
    report.totalRevenue = 0.0;
    report.cancellations = 0;

    int totalOccupancy = 0;
    int daysInMonthCount = daysInMonth(month, year);

    // --------------------------------------------------------
    // Calculate daily occupancy
    // --------------------------------------------------------

    for (int day = 1; day <= daysInMonthCount; day++)
    {
        Date date{day, month, year};
        totalOccupancy += countOccupiedOnDate(bookings, date);
    }

    if (daysInMonthCount > 0 && report.totalRooms > 0)
    {
        report.avgOccupancyRate =
            totalOccupancy * 100.0 / (daysInMonthCount * report.totalRooms);
    }
    else
    {
        report.avgOccupancyRate = 0.0;
    }

    // --------------------------------------------------------
    // Booking statistics
    // --------------------------------------------------------

    map<string, vector<Booking>> grouped = groupBookingsById(bookings);

    for (const auto& pair : grouped)
    {
        const vector<Booking>& group = pair.second;

        if (group.empty())
        {
            continue;
        }

        const Booking& first = group[0];

        // Booking belongs to month based on check-in date
        if (first.checkInDate.month == month && first.checkInDate.year == year)
        {
            report.totalBookings++;

            if (first.status == "CANCELLED")
            {
                report.cancellations++;
            }
            else
            {
                report.totalCheckIns++;

                if (first.paid)
                {
                    report.totalRevenue +=
                        calculateBookingGroupRevenue(group, rooms);
                }
            }
        }

        // Check-out statistics
        if (first.checkOutDate.month == month &&
            first.checkOutDate.year == year &&
            first.status != "CANCELLED")
        {
            report.totalCheckOuts++;
        }
    }

    // --------------------------------------------------------
    // Cancellation rate
    // --------------------------------------------------------

    report.cancellationRate = report.totalBookings > 0
                                  ? (report.cancellations * 100.0 /
                                     report.totalBookings)
                                  : 0.0;

    return report;
}

// ============================================================
// Customer Report
// ============================================================

vector<CustomerReport> generateCustomerReport(const vector<Booking>& bookings)
{
    map<string, CustomerReport> customerMap;

    // Load actual room information
    vector<Room> rooms;
    vector<Booking> demoBookings;

    loadSchedulerDemoData(rooms, demoBookings);

    // Load VIP memberships
    vector<VIPMembership> memberships = loadVIPMemberships();

    Date today = getCurrentSystemDate();

    // Group bookings by booking ID first
    map<string, vector<Booking>> bookingsById = groupBookingsById(bookings);

    for (const auto& pair : bookingsById)
    {
        const vector<Booking>& group = pair.second;

        if (group.empty())
        {
            continue;
        }

        const Booking& first = group[0];
        CustomerReport& report = customerMap[first.customerId];

        // Initialise only when first created
        if (report.customerId.empty())
        {
            report.customerId = first.customerId;
            report.totalBookings = 0;
            report.completedBookings = 0;
            report.cancelledBookings = 0;
            report.totalSpent = 0.0;
            report.vipStatus = "Standard";
        }

        // One booking ID = one customer booking
        report.totalBookings++;

        if (first.status == "COMPLETED")
        {
            report.completedBookings++;
        }

        if (first.status == "CANCELLED")
        {
            report.cancelledBookings++;
        }

        // Only paid bookings contribute to spending
        if (first.paid && first.status != "CANCELLED")
        {
            report.totalSpent +=
                calculateBookingGroupRevenue(group, rooms);
        }
    }

    // --------------------------------------------------------
    // Determine VIP status
    // --------------------------------------------------------

    vector<CustomerReport> result;

    for (auto& pair : customerMap)
    {
        CustomerReport& report = pair.second;
        report.vipStatus = "Standard";

        for (const VIPMembership& membership : memberships)
        {
            if (membership.customerId != report.customerId)
            {
                continue;
            }

            if (!membership.isActive)
            {
                continue;
            }

            // Membership is still valid
            if (compareDates(today, membership.expiryDate) <= 0)
            {
                report.vipStatus = membership.tier;
                break;
            }
        }

        result.push_back(report);
    }

    return result;
}

// ============================================================
// Read Date For Report
// ============================================================

Date readDateForReport(const string& prompt, int minYear, int maxYear)
{
    while (true)
    {
        cout << '\n' << prompt << "\n";

        Date date;

        date.year = readInteger("Year (" + to_string(minYear) + "-" +
                                    to_string(maxYear) + "): ",
                                minYear, maxYear);

        date.month = readInteger("Month (1-12): ", 1, 12);
        date.day = readInteger("Day: ", 1, 31);

        if (isValidDate(date))
        {
            return date;
        }

        cout << "Invalid date. Please try again.\n";
    }
}

// ============================================================
// View Daily Report
// ============================================================

void viewDailyReport()
{
    cout << "\n====================================\n";
    cout << "         DAILY REPORT\n";
    cout << "====================================\n";

    Date date = readDateForReport("Enter date for report", 2026, 2028);

    vector<Room> rooms;
    vector<Booking> bookings;

    loadSchedulerDemoData(rooms, bookings);

    vector<Booking> saved = loadSavedBookings();

    for (const Booking& b : saved)
    {
        bookings.push_back(b);
    }

    DailyReport report = generateDailyReport(date, rooms, bookings);

    cout << "\n  " << string(50, '-') << "\n";
    cout << "  Report Date: " << formatDate(report.date) << "\n";
    cout << "  " << string(50, '-') << "\n";

    cout << "  Total Rooms: " << report.totalRooms << "\n";
    cout << "  Occupied Rooms: " << report.occupiedRooms << "\n";
    cout << "  Available Rooms: " << report.availableRooms << "\n";
    cout << "  Occupancy Rate: " << fixed << setprecision(1)
         << report.occupancyRate << "%\n";

    cout << "  " << string(50, '-') << "\n";

    cout << "  Check-ins Today: " << report.checkIns << "\n";
    cout << "  Check-outs Today: " << report.checkOuts << "\n";

    cout << "  " << string(50, '-') << "\n";

    cout << "  Pending Bookings: " << report.pendingBookings << "\n";
    cout << "  Cancelled Bookings: " << report.cancelledBookings << "\n";

    cout << "  " << string(50, '-') << "\n";

    cout << "  Total Revenue: RM " << fixed << setprecision(2)
         << report.totalRevenue << "\n";

    cout << "  " << string(50, '-') << "\n";
    cout << "====================================\n";

    EnterToContinue();
}

// ============================================================
// View Monthly Report
// ============================================================

void viewMonthlyReport()
{
    cout << "\n====================================\n";
    cout << "         MONTHLY REPORT\n";
    cout << "====================================\n";

    int year = readInteger("Enter year (2026-2028): ", 2026, 2028);
    int month = readInteger("Enter month (1-12): ", 1, 12);

    vector<Room> rooms;
    vector<Booking> bookings;

    loadSchedulerDemoData(rooms, bookings);

    vector<Booking> saved = loadSavedBookings();

    for (const Booking& b : saved)
    {
        bookings.push_back(b);
    }

    MonthlyReport report =
        generateMonthlyReport(month, year, rooms, bookings);

    cout << "\n  " << string(50, '-') << "\n";
    cout << "  Report Month: " << month << "/" << year << "\n";
    cout << "  " << string(50, '-') << "\n";

    cout << "  Total Rooms: " << report.totalRooms << "\n";
    cout << "  Avg Occupancy: " << fixed << setprecision(1)
         << report.avgOccupancyRate << "%\n";

    cout << "  " << string(50, '-') << "\n";

    cout << "  Total Bookings: " << report.totalBookings << "\n";
    cout << "  Check-ins: " << report.totalCheckIns << "\n";
    cout << "  Check-outs: " << report.totalCheckOuts << "\n";
    cout << "  Cancellations: " << report.cancellations << "\n";
    cout << "  Cancellation Rate: " << fixed << setprecision(1)
         << report.cancellationRate << "%\n";

    cout << "  " << string(50, '-') << "\n";

    cout << "  Total Revenue: RM " << fixed << setprecision(2)
         << report.totalRevenue << "\n";

    cout << "  " << string(50, '-') << "\n";
    cout << "====================================\n";

    EnterToContinue();
}

// ============================================================
// View Customer Report
// ============================================================

void viewCustomerReport()
{
    cout << "\n====================================\n";
    cout << "         CUSTOMER REPORT\n";
    cout << "====================================\n";

    vector<Booking> saved = loadSavedBookings();
    vector<CustomerReport> reports = generateCustomerReport(saved);

    if (reports.empty())
    {
        cout << "  No customer data available.\n";
        cout << "====================================\n";

        EnterToContinue();
        return;
    }

    cout << "\n  " << left << setw(12) << "CUSTOMER" << setw(10) << "TOTAL"
         << setw(10) << "COMPLETED" << setw(10) << "CANCELLED"
         << setw(15) << "TOTAL SPENT" << "VIP STATUS\n";

    cout << "  " << string(70, '-') << "\n";

    for (const CustomerReport& cr : reports)
    {
        cout << "  " << left << setw(12) << cr.customerId
             << setw(10) << cr.totalBookings
             << setw(10) << cr.completedBookings
             << setw(10) << cr.cancelledBookings
             << "RM " << setw(10) << fixed << setprecision(2)
             << cr.totalSpent << cr.vipStatus << "\n";
    }

    cout << "  " << string(70, '-') << "\n";
    cout << "  Total Customers: " << reports.size() << "\n";
    cout << "====================================\n";

    EnterToContinue();
}

// ============================================================
// View Revenue Report
// ============================================================

void viewRevenueReport()
{
    cout << "\n====================================\n";
    cout << "         REVENUE REPORT\n";
    cout << "====================================\n";

    vector<Booking> saved = loadSavedBookings();

    if (saved.empty())
    {
        cout << "  No booking data available.\n";
        cout << "====================================\n";

        EnterToContinue();
        return;
    }

    // Load actual room data
    vector<Room> rooms;
    vector<Booking> demoBookings;

    loadSchedulerDemoData(rooms, demoBookings);

    // Group booking records
    map<string, vector<Booking>> grouped = groupBookingsById(saved);

    // Revenue by check-in month
    map<string, double> monthlyRevenue;
    map<string, int> monthlyBookings;

    for (const auto& pair : grouped)
    {
        const vector<Booking>& group = pair.second;

        if (group.empty())
        {
            continue;
        }

        const Booking& first = group[0];

        if (first.status == "CANCELLED")
        {
            continue;
        }

        if (!first.paid)
        {
            continue;
        }

        string key = to_string(first.checkInDate.month) + "/" +
                     to_string(first.checkInDate.year);

        double revenue = calculateBookingGroupRevenue(group, rooms);

        monthlyRevenue[key] += revenue;
        monthlyBookings[key]++;
    }

    cout << "\n  Monthly Revenue Breakdown:\n";
    cout << "  " << string(50, '-') << "\n";

    cout << "  " << left << setw(12) << "MONTH"
         << setw(10) << "BOOKINGS"
         << setw(20) << "REVENUE"
         << setw(12) << "AVG/BOOKING" << "\n";

    cout << "  " << string(55, '-') << "\n";

    double totalRevenue = 0.0;
    int totalBookings = 0;

    for (const auto& pair : monthlyRevenue)
    {
        string month = pair.first;
        double revenue = pair.second;
        int count = monthlyBookings[month];

        double avg = count > 0 ? revenue / count : 0.0;

        totalRevenue += revenue;
        totalBookings += count;

        cout << "  " << left << setw(12) << month
             << setw(10) << count
             << "RM " << setw(16) << fixed << setprecision(2) << revenue
             << "RM " << fixed << setprecision(2) << avg << "\n";
    }

    cout << "  " << string(55, '-') << "\n";

    double overallAvg = totalBookings > 0
                            ? totalRevenue / totalBookings
                            : 0.0;

    cout << "  " << left << setw(12) << "TOTAL"
         << setw(10) << totalBookings
         << "RM " << setw(16) << fixed << setprecision(2) << totalRevenue
         << "RM " << fixed << setprecision(2) << overallAvg << "\n";

    cout << "====================================\n";

    EnterToContinue();
}

// ============================================================
// Report Menu
// ============================================================

void reportMenu()
{
    while (true)
    {
        cout << "\n====================================\n";
        cout << "       REPORT MANAGEMENT\n";
        cout << "====================================\n";
        cout << "[1] Daily Report\n";
        cout << "[2] Monthly Report\n";
        cout << "[3] Customer Report\n";
        cout << "[4] Revenue Report\n";
        cout << "[0] Return to Main Menu\n";
        cout << "====================================\n";

        int choice = readInteger("Enter your choice: ", 0, 4);

        if (choice == 0) return;
            

        if (choice == 1) viewDailyReport();
        else if (choice == 2) viewMonthlyReport();
        else if (choice == 3) viewCustomerReport();
        else if (choice == 4) viewRevenueReport();
            
    }
}