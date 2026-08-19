#pragma once

#include <string>
using namespace std;

struct Date
{
    int day;
    int month;
    int year;
};

struct Room
{
    string roomId;
    string roomType;
    double nightlyRate;
    int capacity;
};

struct Booking
{
    string bookingId;
    string customerId;
    string roomId;
    Date bookingDate;
    Date checkInDate;
    Date checkOutDate;
    Date expiryDate;
    string status;
    bool paid;
    string accessCode;
};
