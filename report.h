#pragma once

#include "models.h"
#include <vector>
#include <string>

// Report types
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

// Report generation functions
DailyReport generateDailyReport(const Date& date, 
                                const std::vector<Room>& rooms, 
                                const std::vector<Booking>& bookings);

MonthlyReport generateMonthlyReport(int month, int year,
                                   const std::vector<Room>& rooms,
                                   const std::vector<Booking>& bookings);

std::vector<CustomerReport> generateCustomerReport(const std::vector<Booking>& bookings);

// Display functions
void viewDailyReport();
Date readDateForReport(const string& prompt, int minYear, int maxYear);
void viewMonthlyReport();
void viewCustomerReport();
void viewRevenueReport();
void reportMenu();