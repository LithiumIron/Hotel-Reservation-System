#pragma once

#include "models.h"
#include <vector>
#include <string>

using namespace std;

// ============================================================
// Report Types
// ============================================================

struct DailyReport
{
    Date date;
    int totalRooms;
    int occupiedRooms;
    int availableRooms;
    double occupancyRate;
    int checkIns;
    int checkOuts;
    double totalRevenue;
    int pendingBookings;
    int cancelledBookings;
};

struct MonthlyReport
{
    int month;
    int year;
    int totalRooms;
    double avgOccupancyRate;
    int totalBookings;
    int totalCheckIns;
    int totalCheckOuts;
    double totalRevenue;
    int cancellations;
    double cancellationRate;
};

struct CustomerReport
{
    string customerId;
    int totalBookings;
    int completedBookings;
    int cancelledBookings;
    double totalSpent;
    string vipStatus;
};

// ============================================================
// Report Generation Functions
// ============================================================

DailyReport generateDailyReport(const Date& date, const vector<Room>& rooms, const vector<Booking>& bookings);
MonthlyReport generateMonthlyReport(int month, int year, const vector<Room>& rooms, const vector<Booking>& bookings);
vector<CustomerReport> generateCustomerReport(const vector<Booking>& bookings);

// ============================================================
// Display Functions
// ============================================================

void viewDailyReport();
Date readDateForReport(const string& prompt, int minYear, int maxYear);
void viewMonthlyReport();
void viewCustomerReport();
void viewRevenueReport();
void reportMenu();