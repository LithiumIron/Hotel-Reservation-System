#pragma once

#include "models.h"

#include <string>
#include <vector>

bool isLeapYear(int year);
int daysInMonth(int month, int year);
bool isValidDate(const Date& date);
int compareDates(const Date& firstDate, const Date& secondDate);
Date addDays(const Date& date, int numberOfDays);
std::string formatDate(const Date& date);

void viewWeeklyHeatmap(const std::vector<Booking>& bookings,
    const std::vector<Room>& rooms, const Date& startDate);
void viewMonthlyAvailability(const std::vector<Booking>& bookings,
    const std::vector<Room>& rooms, int month, int year);
void viewDailyRoomSchedule(const std::vector<Booking>& bookings,
    const std::vector<Room>& rooms, const Date& selectedDate);
void viewRoomMonthlyTimeline(const std::vector<Booking>& bookings,
    const std::vector<Room>& rooms, const std::string& roomId,
    int month, int year);
void viewOccupancyForecast(const std::vector<Booking>& bookings,
    const std::vector<Room>& rooms, const Date& startDate, int numberOfDays);
int autoReleaseExpiredBookings(std::vector<Booking>& bookings,
    const Date& currentDate);
int autoReleaseExpiredBookings(std::vector<Booking>& bookings,
    const Date& currentDate, std::vector<Booking>& releasedBookings);
void viewFloorAvailabilityMap(const std::vector<Booking>& bookings,
    const std::vector<Room>& rooms, const Date& checkInDate,
    const Date& checkOutDate);
int auditDoubleBookings(const std::vector<Booking>& bookings);
void loadSchedulerDemoData(std::vector<Room>& rooms,
    std::vector<Booking>& bookings);
void customerSchedulerMenu(const std::vector<Room>& rooms,
    const std::vector<Booking>& bookings);
void employeeSchedulerMenu(const std::vector<Room>& rooms,
    std::vector<Booking>& bookings);
