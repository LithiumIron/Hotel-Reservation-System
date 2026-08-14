#pragma once

#include <string>

struct Date
{
    int day;
    int month;
    int year;
};

struct Room
{
    std::string roomId;
    std::string roomType;
    double nightlyRate;
};

struct Booking
{
    std::string bookingId;
    std::string customerId;
    std::string roomId;
    Date bookingDate;
    Date checkInDate;
    Date checkOutDate;
    Date expiryDate;
    std::string status;
    bool paid;
};
