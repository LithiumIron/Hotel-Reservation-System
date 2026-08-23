#include "report.h"
#include "scheduler.h"
#include "utilities.h"
#include "booking.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <algorithm>

using namespace std;

bool isPastDate(const Date& date){
    Date today = getCurrentSystemDate();  
    return compareDates(date, today) < 0;
}

bool isRoomOccupied(const Booking& booking, const Date& date){
    return booking.status != "CANCELLED" && 
            booking.status != "COMPLETED" &&
            compareDates(booking.checkInDate, date) <= 0 &&
            compareDates(date, booking.checkOutDate) < 0;
}

int countOccupiedOnDate(const vector<Booking>& bookings, const Date& date){
    int count = 0;
    for (const Booking& b : bookings)
    {
        if (isRoomOccupied(b, date))
            count++;
    }
    return count;
}

double calculateBookingRevenue(const Booking& booking){
    double baseRate = 100.0; // Average room rate
    int nights = 0;
    Date temp = booking.checkInDate;
    while (compareDates(temp, booking.checkOutDate) < 0)
    {
        nights++;
        temp = addDays(temp, 1);
    }
    
    // Add add-on costs
    double addonCost = 0;
    if (!booking.addons.empty())
    {
        // Simple estimation
        addonCost = 20.0 * nights;
    }
    
    return (baseRate * nights) + addonCost;
}


DailyReport generateDailyReport(const Date& date, 
                                const vector<Room>& rooms, 
                                const vector<Booking>& bookings)
{
    DailyReport report;
    report.date = date;
    report.totalRooms = rooms.size();
    report.occupiedRooms = countOccupiedOnDate(bookings, date);
    report.availableRooms = report.totalRooms - report.occupiedRooms;
    report.occupancyRate = report.totalRooms > 0 ? 
        (report.occupiedRooms * 100.0 / report.totalRooms) : 0;
    
    report.checkIns = 0;
    report.checkOuts = 0;
    report.totalRevenue = 0;
    report.pendingBookings = 0;
    report.cancelledBookings = 0;
    
    for (const Booking& b : bookings)
    {
        // Check-ins
        if (b.checkInDate.day == date.day &&
            b.checkInDate.month == date.month &&
            b.checkInDate.year == date.year &&
            b.status != "CANCELLED")
        {
            report.checkIns++;
        }
        
        // Check-outs
        if (b.checkOutDate.day == date.day &&
            b.checkOutDate.month == date.month &&
            b.checkOutDate.year == date.year &&
            b.status != "CANCELLED")
        {
            report.checkOuts++;
        }
        
        // Revenue
        if (b.status != "CANCELLED" && b.paid)
        {
            report.totalRevenue += calculateBookingRevenue(b);
        }
        
        // Status counts
        if (b.status == "PENDING")
            report.pendingBookings++;
        if (b.status == "CANCELLED")
            report.cancelledBookings++;
    }
    
    return report;
}

MonthlyReport generateMonthlyReport(int month, int year,
                                   const vector<Room>& rooms,
                                   const vector<Booking>& bookings)
{
    MonthlyReport report;
    report.month = month;
    report.year = year;
    report.totalRooms = rooms.size();
    
    int totalOccupancy = 0;
    int daysInMonthCount = daysInMonth(month, year);
    report.totalBookings = 0;
    report.totalCheckIns = 0;
    report.totalCheckOuts = 0;
    report.totalRevenue = 0;
    report.cancellations = 0;
    
    for (int day = 1; day <= daysInMonthCount; day++)
    {
        Date date{ day, month, year };
        totalOccupancy += countOccupiedOnDate(bookings, date);
    }
    
    report.avgOccupancyRate = (daysInMonthCount > 0 && report.totalRooms > 0) ?
        (totalOccupancy * 100.0 / (daysInMonthCount * report.totalRooms)) : 0;
    
    for (const Booking& b : bookings)
    {
        if (b.checkInDate.month == month && b.checkInDate.year == year)
        {
            report.totalBookings++;
            if (b.status != "CANCELLED")
            {
                report.totalCheckIns++;
                if (b.paid)
                {
                    report.totalRevenue += calculateBookingRevenue(b);
                }
            }
            if (b.status == "CANCELLED")
                report.cancellations++;
        }
    }
    
    report.cancellationRate = report.totalBookings > 0 ?
        (report.cancellations * 100.0 / report.totalBookings) : 0;
    
    return report;
}

vector<CustomerReport> generateCustomerReport(const vector<Booking>& bookings)
{
    map<string, CustomerReport> customerMap;
    
    for (const Booking& b : bookings)
    {
        if (customerMap.find(b.customerId) == customerMap.end())
        {
            CustomerReport cr;
            cr.customerId = b.customerId;
            cr.totalBookings = 0;
            cr.completedBookings = 0;
            cr.cancelledBookings = 0;
            cr.totalSpent = 0;
            customerMap[b.customerId] = cr;
        }
        
        CustomerReport& cr = customerMap[b.customerId];
        cr.totalBookings++;
        
        if (b.status == "COMPLETED")
        {
            cr.completedBookings++;
            if (b.paid)
                cr.totalSpent += calculateBookingRevenue(b);
        }
        else if (b.status == "CANCELLED")
        {
            cr.cancelledBookings++;
        }
    }
    
    vector<CustomerReport> result;
    for (auto& pair : customerMap)
    {
        // Calculate VIP status
        int total = pair.second.totalBookings;
        if (total >= 10)
            pair.second.vipStatus = "Platinum";
        else if (total >= 5)
            pair.second.vipStatus = "Gold";
        else if (total >= 3)
            pair.second.vipStatus = "Silver";
        else
            pair.second.vipStatus = "Standard";
        
        result.push_back(pair.second);
    }
    
    return result;
}

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
        bookings.push_back(b);
    
    DailyReport report = generateDailyReport(date, rooms, bookings);
    
    cout << "\n  " << string(50, '-') << "\n";
    cout << "  Report Date: " << formatDate(report.date) << "\n";
    cout << "  " << string(50, '-') << "\n";
    cout << "  " << left << setw(25) << "Total Rooms:" << report.totalRooms << "\n";
    cout << "  " << left << setw(25) << "Occupied Rooms:" << report.occupiedRooms << "\n";
    cout << "  " << left << setw(25) << "Available Rooms:" << report.availableRooms << "\n";
    cout << "  " << left << setw(25) << "Occupancy Rate:" << fixed << setprecision(1) << report.occupancyRate << "%\n";
    cout << "  " << string(50, '-') << "\n";
    cout << "  " << left << setw(25) << "Check-ins Today:" << report.checkIns << "\n";
    cout << "  " << left << setw(25) << "Check-outs Today:" << report.checkOuts << "\n";
    cout << "  " << string(50, '-') << "\n";
    cout << "  " << left << setw(25) << "Pending Bookings:" << report.pendingBookings << "\n";
    cout << "  " << left << setw(25) << "Cancelled Bookings:" << report.cancelledBookings << "\n";
    cout << "  " << string(50, '-') << "\n";
    cout << "  " << left << setw(25) << "Total Revenue:" << "RM " << fixed << setprecision(2) << report.totalRevenue << "\n";
    cout << "  " << string(50, '-') << "\n";
    cout << "====================================\n";
    EnterToContinue();
}

Date readDateForReport(const string& prompt, int minYear, int maxYear)
{
    while (true)
    {
        cout << '\n' << prompt << "\n";
        Date date;
        date.year = readInteger("Year (" + to_string(minYear) + "-" + to_string(maxYear) + "): ", minYear, maxYear);
        date.month = readInteger("Month (1-12): ", 1, 12);
        date.day = readInteger("Day: ", 1, 31);

        if (isValidDate(date))
        {
            return date;
        }
        cout << "Invalid date. Please try again.\n";
    }
}

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
        bookings.push_back(b);
    
    MonthlyReport report = generateMonthlyReport(month, year, rooms, bookings);
    
    cout << "\n  " << string(50, '-') << "\n";
    cout << "  Report Month: " << month << "/" << year << "\n";
    cout << "  " << string(50, '-') << "\n";
    cout << "  " << left << setw(25) << "Total Rooms:" << report.totalRooms << "\n";
    cout << "  " << left << setw(25) << "Avg Occupancy:" << fixed << setprecision(1) << report.avgOccupancyRate << "%\n";
    cout << "  " << string(50, '-') << "\n";
    cout << "  " << left << setw(25) << "Total Bookings:" << report.totalBookings << "\n";
    cout << "  " << left << setw(25) << "Check-ins:" << report.totalCheckIns << "\n";
    cout << "  " << left << setw(25) << "Check-outs:" << report.totalCheckOuts << "\n";
    cout << "  " << left << setw(25) << "Cancellations:" << report.cancellations << "\n";
    cout << "  " << left << setw(25) << "Cancellation Rate:" << fixed << setprecision(1) << report.cancellationRate << "%\n";
    cout << "  " << string(50, '-') << "\n";
    cout << "  " << left << setw(25) << "Total Revenue:" << "RM " << fixed << setprecision(2) << report.totalRevenue << "\n";
    cout << "  " << string(50, '-') << "\n";
    cout << "====================================\n";
    EnterToContinue();
}

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
    
    cout << "\n  " << left << setw(12) << "CUSTOMER" 
         << setw(10) << "TOTAL" 
         << setw(10) << "COMPLETED" 
         << setw(10) << "CANCELLED"
         << setw(15) << "TOTAL SPENT"
         << "VIP STATUS\n";
    cout << "  " << string(70, '-') << "\n";
    
    for (const CustomerReport& cr : reports)
    {
        cout << "  " << left << setw(12) << cr.customerId
             << setw(10) << cr.totalBookings
             << setw(10) << cr.completedBookings
             << setw(10) << cr.cancelledBookings
             << "RM " << setw(10) << fixed << setprecision(2) << cr.totalSpent
             << cr.vipStatus << "\n";
    }
    
    cout << "  " << string(70, '-') << "\n";
    cout << "  Total Customers: " << reports.size() << "\n";
    cout << "====================================\n";
    EnterToContinue();
}

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
    
    // Group revenue by month
    map<string, double> monthlyRevenue;
    map<string, int> monthlyBookings;
    
    for (const Booking& b : saved)
    {
        if (b.status != "CANCELLED" && b.paid)
        {
            string key = to_string(b.bookingDate.month) + "/" + to_string(b.bookingDate.year);
            monthlyRevenue[key] += calculateBookingRevenue(b);
            monthlyBookings[key]++;
        }
    }
    
    cout << "\n  Monthly Revenue Breakdown:\n";
    cout << "  " << string(50, '-') << "\n";
    cout << "  " << left << setw(12) << "MONTH" 
         << setw(10) << "BOOKINGS" 
         << setw(20) << "REVENUE" 
         << setw(12) << "AVG/BOOKING\n";
    cout << "  " << string(55, '-') << "\n";
    
    double totalRevenue = 0;
    int totalBookings = 0;
    
    for (const auto& pair : monthlyRevenue)
    {
        string month = pair.first;
        double revenue = pair.second;
        int count = monthlyBookings[month];
        double avg = count > 0 ? revenue / count : 0;
        
        totalRevenue += revenue;
        totalBookings += count;
        
        cout << "  " << left << setw(12) << month
             << setw(10) << count
             << "RM " << setw(16) << fixed << setprecision(2) << revenue
             << "RM " << fixed << setprecision(2) << avg << "\n";
    }
    
    cout << "  " << string(55, '-') << "\n";
    double overallAvg = totalBookings > 0 ? totalRevenue / totalBookings : 0;
    cout << "  " << left << setw(12) << "TOTAL"
         << setw(10) << totalBookings
         << "RM " << setw(16) << fixed << setprecision(2) << totalRevenue
         << "RM " << fixed << setprecision(2) << overallAvg << "\n";
    
    cout << "====================================\n";
    EnterToContinue();
}

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