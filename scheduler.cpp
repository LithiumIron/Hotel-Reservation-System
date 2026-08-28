#include "scheduler.h"
#include "utilities.h"
#include "booking.h"

#include <set>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <vector>

using namespace std;

// Get today's date from the system
Date getCurrentSystemDate()
{
    time_t now = time(nullptr);
    tm localTime{};
    localtime_s(&localTime, &now);

    Date today;
    today.day = localTime.tm_mday;
    today.month = localTime.tm_mon + 1;
    today.year = localTime.tm_year + 1900;

    return today;
}

// System display and date settings
const int UI_WIDTH = 82;
const int CUSTOMER_MIN_YEAR = 2026;
const int MANAGER_MIN_YEAR = 2020;
const int MAX_SCHEDULE_YEAR = 2028;
const Date SYSTEM_DATE= getCurrentSystemDate();

// Booking status values
const string STATUS_PENDING = "PENDING";
const string STATUS_CONFIRMED = "CONFIRMED";
const string STATUS_CHECKED_IN = "CHECKED_IN";
const string STATUS_CANCELLED = "CANCELLED";
const string STATUS_COMPLETED = "COMPLETED";

// Room type names
const string ROOM_SS = "Standard Single";
const string ROOM_SD = "Standard Double";
const string ROOM_DQ = "Deluxe Queen";
const string ROOM_FS = "Family Suite";
const string ROOM_PS = "Presidential Suite";

// Center text within a given width
string centeredText(const string& text, int width)
{
    if (static_cast<int>(text.length()) >= width)
    {
        return text.substr(0, width);
    }
    int leftPadding = (width - static_cast<int>(text.length())) / 2;
    return string(leftPadding, ' ') + text
        + string(width - leftPadding - static_cast<int>(text.length()), ' ');
}

// Display a formatted banner
void printBanner(const string& title, const string& subtitle = "")
{
    cout << '\n' << '+' << string(UI_WIDTH - 2, '=') << "+\n";
    cout << '|' << centeredText("HOTEL RESERVATION SYSTEM", UI_WIDTH - 2) << "|\n";
    cout << '|' << centeredText(title, UI_WIDTH - 2) << "|\n";
    if (!subtitle.empty())
    {
        cout << '|' << centeredText(subtitle, UI_WIDTH - 2) << "|\n";
    }
    cout << '+' << string(UI_WIDTH - 2, '=') << "+\n";
}

// Display a menu option
void printMenuOption(int option, const string& title, const string& description)
{
    ostringstream label;
    label << "[" << option << "] " << title;
    cout << "|  " << left << setw(32) << label.str()
            << setw(UI_WIDTH - 36) << description << "|\n";
}

// Create a simple occupancy bar
string occupancyBar(int percentage, int width = 20)
{
    int filled = percentage * width / 100;
    return "[" + string(filled, '#') + string(width - filled, '.') + "]";
}

// Display a result message
void printResultPanel(const string& label, const string& message)
{
    cout << '+' << string(UI_WIDTH - 2, '-') << "+\n";
    cout << "| " << left << setw(14) << label << setw(UI_WIDTH - 17)
            << message << "|\n";
    cout << '+' << string(UI_WIDTH - 2, '-') << "+\n";
}

// Check if a booking blocks a room
bool isRoomBlocked(const Booking& booking)
{
    return booking.status == STATUS_PENDING
        || booking.status == STATUS_CONFIRMED
        || booking.status == STATUS_CHECKED_IN;
}

// Check if a booking occupies a date
bool bookingOccupiesDate(const Booking& booking, const Date& date)
{
    return booking.status != STATUS_CANCELLED
        && compareDates(booking.checkInDate, date) <= 0
        && compareDates(date, booking.checkOutDate) < 0;
}

// Check if two booking periods overlap
bool dateRangesOverlap(const Date& firstCheckIn, const Date& firstCheckOut,
    const Date& secondCheckIn, const Date& secondCheckOut)
{
    return compareDates(firstCheckIn, secondCheckOut) < 0
        && compareDates(secondCheckIn, firstCheckOut) < 0;
}

// Check if a room is available for a stay
bool roomAvailableForStay(const vector<Booking>& bookings,
    const string& roomId, const Date& checkInDate, const Date& checkOutDate)
{
    for (const Booking& booking : bookings)
    {
        if (booking.roomId == roomId && isRoomBlocked(booking)
            && dateRangesOverlap(checkInDate, checkOutDate,
                booking.checkInDate, booking.checkOutDate))
        {
            return false;
        }
    }
    return true;
}

// Find the booking for a room on a date
const Booking* findBookingForRoom(const vector<Booking>& bookings,
    const string& roomId, const Date& date)
{
    for (const Booking& booking : bookings)
    {
        if (booking.roomId == roomId && bookingOccupiesDate(booking, date))
        {
            return &booking;
        }
    }
    return nullptr;
}

// Count occupied rooms on a date
int countOccupiedRooms(const vector<Booking>& bookings,
    const vector<Room>& rooms, const Date& date)
{
    int occupiedRooms = 0;
    for (const Room& room : rooms)
    {
        if (findBookingForRoom(bookings, room.roomId, date) != nullptr)
        {
            occupiedRooms++;
        }
    }
    return occupiedRooms;
}

// Read and validate a date
Date readDate(const string& heading, int minimumYear, int maximumYear)
{
    while (true)
    {
        cout << '\n' << heading << "\n";
        Date date;
        ostringstream yearPrompt;
        yearPrompt << "Year (" << minimumYear << '-' << maximumYear << "): ";
        date.year = readInteger(yearPrompt.str(), minimumYear, maximumYear);
        date.month = readInteger("Month (1-12): ", 1, 12);
        date.day = readInteger("Day: ", 1, 31);

        if (isValidDate(date))
        {
            return date;
        }
        cout << "Invalid date. Please try again.\n";
    }
}

// Read a current or future date
Date readCurrentOrFutureDate(const string& heading)
{
    while (true)
    {
        Date date = readDate(heading, CUSTOMER_MIN_YEAR, MAX_SCHEDULE_YEAR);
        if (compareDates(date, SYSTEM_DATE) >= 0)
        {
            return date;
        }
        cout << "Past dates are not available to customers. Earliest date: "
                << formatDate(SYSTEM_DATE) << ".\n";
    }
}

// Read and validate a room ID
string readRoomId(const vector<Room>& rooms)
{
    while (true)
    {
        string roomId;
        cout << "Room ID: ";
        getline(cin, roomId);

        for (const Room& room : rooms)
        {
            if (room.roomId == roomId)
            {
                return roomId;
            }
        }
        cout << "Room ID not found. \nAvailable rooms: ";
        for (const Room& room : rooms)
        {
            cout << room.roomId << ' ';
        }
        cout << '\n';
    }
}

// Get the day index for a date
int dayIndexForDate(const Date& date)
{
    static const int offsets[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
    int year = date.year;
    if (date.month < 3)
    {
        year--;
    }
    return (year + year / 4 - year / 100 + year / 400
        + offsets[date.month - 1] + date.day) % 7;
}

// Get the short name of a day
string dayName(const Date& date)
{
    static const string names[] =
        { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
    return names[dayIndexForDate(date)];
}

// Check if a year is a leap year
bool isLeapYear(int year)
{
    return year % 400 == 0 || (year % 4 == 0 && year % 100 != 0);
}

// Get the number of days in a month
int daysInMonth(int month, int year)
{
    static const int days[] = { 31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31 };
    if (month < 1 || month > 12)
    {
        return 0;
    }
    if (month == 2 && isLeapYear(year))
    {
        return 29;
    }
    return days[month - 1];
}

// Check if a date is valid
bool isValidDate(const Date& date)
{
    return date.year >= 1 && date.month >= 1 && date.month <= 12
        && date.day >= 1 && date.day <= daysInMonth(date.month, date.year);
}

// Compare two dates
int compareDates(const Date& firstDate, const Date& secondDate)
{
    if (firstDate.year != secondDate.year)
    {
        return firstDate.year < secondDate.year ? -1 : 1;
    }
    if (firstDate.month != secondDate.month)
    {
        return firstDate.month < secondDate.month ? -1 : 1;
    }
    if (firstDate.day != secondDate.day)
    {
        return firstDate.day < secondDate.day ? -1 : 1;
    }
    return 0;
}

// Add days to a date
Date addDays(const Date& date, int numberOfDays)
{
    Date result = date;
    for (int count = 0; count < numberOfDays; count++)
    {
        result.day++;
        if (result.day > daysInMonth(result.month, result.year))
        {
            result.day = 1;
            result.month++;
            if (result.month > 12)
            {
                result.month = 1;
                result.year++;
            }
        }
    }
    return result;
}

// Format a date as DD/MM/YYYY
string formatDate(const Date& date)
{
    ostringstream output;
    output << setfill('0') << setw(2) << date.day << '/'
           << setw(2) << date.month << '/' << date.year;
    return output.str();
}

// Display weekly room occupancy
void viewWeeklyHeatmap(const vector<Booking>& bookings,
    const vector<Room>& rooms, const Date& startDate)
{
    clearScreen();

    const int DAYS = 7;

    printBanner("WEEKLY OCCUPANCY HEATMAP");
    cout << "  Legend: . Empty | L Low | M Moderate | H High | F Full\n\n";
    cout << "  " << left << setw(17) << "DATE" << setw(8) << "LEVEL"
         << setw(12) << "ROOMS" << setw(24) << "VISUAL LOAD" << "RATE\n";
    cout << "  " << string(76, '-') << '\n';

    int roomLimit = static_cast<int>(rooms.size());

    for (int dayIndex = 0; dayIndex < DAYS; dayIndex++)
    {
        Date date = addDays(startDate, dayIndex);
        int occupiedCount = 0;

        for (int roomIndex = 0; roomIndex < roomLimit; roomIndex++)
        {
            if (findBookingForRoom(bookings, rooms[roomIndex].roomId, date))
            {
                occupiedCount++;
            }
        }

        int percentage = roomLimit == 0 ? 0 : occupiedCount * 100 / roomLimit;
        char level = percentage == 0 ? '.'
            : percentage <= 40 ? 'L'
            : percentage <= 75 ? 'M'
            : percentage < 100 ? 'H' : 'F';

        cout << "  " << left << setw(17) << (dayName(date) + " " + formatDate(date))
             << setw(8) << level
             << setw(12) << (to_string(occupiedCount) + "/" + to_string(roomLimit))
             << setw(24) << occupancyBar(percentage) << percentage << "%\n";
    }

    EnterToContinue();
}

// Display monthly room availability
void viewMonthlyAvailability(const vector<Booking>& bookings,
    const vector<Room>& rooms, int month, int year)
{
    clearScreen();

    printBanner("MONTHLY ROOM AVAILABILITY");
    cout << "  Month: " << month << '/' << year
         << "    Cell format: DAY:AVAILABLE ROOMS\n\n";
    const string weekdayNames[] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };
    cout << "  ";
    for (const string& weekday : weekdayNames)
    {
        cout << centeredText(weekday, 10);
    }
    cout << "\n  " << string(70, '-') << "\n  ";

    int firstDayIndex = dayIndexForDate(Date{ 1, month, year });
    for (int blank = 0; blank < firstDayIndex; blank++)
    {
        cout << setw(10) << " ";
    }

    for (int day = 1; day <= daysInMonth(month, year); day++)
    {
        Date date{ day, month, year };
        ostringstream cell;
        cell << setfill('0') << setw(2) << day << setfill(' ') << ':';
        if (compareDates(date, SYSTEM_DATE) < 0)
        {
            cell << "--";
        }
        else
        {
            int available = static_cast<int>(rooms.size())
                - countOccupiedRooms(bookings, rooms, date);
            cell << available;
        }
        cout << centeredText(cell.str(), 10);

        if ((firstDayIndex + day) % 7 == 0)
        {
            cout << "\n  ";
        }
    }
    cout << "\n\n  Availability key: -- PAST | 0 FULL | 1-2 LIMITED | 3+ AVAILABLE\n";

    EnterToContinue();
}

// Display room schedule for a date
void viewDailyRoomSchedule(const vector<Booking>& bookings,
    const vector<Room>& rooms, const Date& selectedDate)
{
    clearScreen();

    printBanner("DAILY ROOM SCHEDULE");
    cout << "  Schedule date: " << dayName(selectedDate) << ' '
         << formatDate(selectedDate) << "\n\n";
    cout << "  " << left << setw(8) << "ROOM" << setw(22) << "TYPE"
         << setw(13) << "BOOKING" << setw(13) << "CUSTOMER" << "STATUS\n";
    cout << "  " << string(75, '-') << '\n';

    for (const Room& room : rooms)
    {
        const Booking* booking = findBookingForRoom(bookings, room.roomId, selectedDate);
        cout << "  " << left << setw(8) << room.roomId << setw(22) << room.roomType;
        if (booking)
        {
            cout << setw(13) << booking->bookingId << setw(13)
                 << booking->customerId << '[' << booking->status << "]\n";
        }
        else
        {
            cout << setw(13) << "-" << setw(13) << "-" << "[AVAILABLE]\n";
        }
    }

    EnterToContinue();
}

// Display a room's monthly booking timeline
void viewRoomMonthlyTimeline(const vector<Booking>& bookings,
    const vector<Room>& rooms, const string& roomId, int month, int year)
{
    bool roomExists = false;
    for (const Room& room : rooms)
    {
        if (room.roomId == roomId)
        {
            roomExists = true;
            break;
        }
    }
    if (!roomExists)
    {
        cout << "Room ID not found.\n";
        return;
    }

    char timeline[31];
    int totalDays = daysInMonth(month, year);
    for (int day = 1; day <= totalDays; day++)
    {
        const Booking* booking = findBookingForRoom(
            bookings, roomId, Date{ day, month, year });
        timeline[day - 1] = booking == nullptr ? '.'
            : booking->status == STATUS_PENDING ? 'P'
            : booking->status == STATUS_CHECKED_IN ? 'I' : 'B';
    }

    clearScreen();
    printBanner("ROOM MONTHLY TIMELINE");
    cout << "Room: " << roomId << " | Month: " << month << '/' << year << "\n";
    cout << "Legend: . Available, P Pending, B Confirmed, I Checked-in\n\n";
    for (int start = 1; start <= totalDays; start += 10)
    {
        int end = start + 9 < totalDays ? start + 9 : totalDays;
        cout << "Day: ";
        for (int day = start; day <= end; day++)
        {
            cout << setw(3) << day;
        }
        cout << "\n     ";
        for (int day = start; day <= end; day++)
        {
            cout << setw(3) << timeline[day - 1];
        }
        cout << "\n\n";
    }

    EnterToContinue();
}

// Display future occupancy forecast
void viewOccupancyForecast(const vector<Booking>& bookings,
    const vector<Room>& rooms, const Date& startDate, int numberOfDays)
{
    clearScreen();

    printBanner("OCCUPANCY FORECAST");
    cout << "  " << left << setw(17) << "DATE" << setw(11) << "USED"
         << setw(11) << "FREE" << setw(24) << "FORECAST LOAD" << "RATE\n";
    cout << "  " << string(76, '-') << '\n';

    int totalOccupied = 0;
    int peakOccupied = -1;
    Date peakDate = startDate;

    for (int index = 0; index < numberOfDays; index++)
    {
        Date date = addDays(startDate, index);
        int occupied = countOccupiedRooms(bookings, rooms, date);
        int available = static_cast<int>(rooms.size()) - occupied;
        int rate = rooms.empty() ? 0 : occupied * 100 / static_cast<int>(rooms.size());
        totalOccupied += occupied;

        if (occupied > peakOccupied)
        {
            peakOccupied = occupied;
            peakDate = date;
        }

        cout << "  " << left << setw(17) << (dayName(date) + " " + formatDate(date))
             << setw(11) << occupied << setw(11) << available
             << setw(24) << occupancyBar(rate) << rate << "%\n";
    }

    double averageRate = rooms.empty() || numberOfDays == 0 ? 0.0
        : totalOccupied * 100.0 / (rooms.size() * numberOfDays);
    cout << fixed << setprecision(1);
    ostringstream averageText;
    averageText << fixed << setprecision(1) << averageRate << "% occupancy";
    printResultPanel("AVERAGE", averageText.str());
    printResultPanel("PEAK DATE", formatDate(peakDate) + " | "
        + to_string(peakOccupied) + " rooms occupied");

    EnterToContinue();
}

// Display available rooms by floor
void viewFloorAvailabilityMap(const vector<Booking>& bookings,
    const vector<Room>& rooms, const Date& checkInDate,
    const Date& checkOutDate)
{
    clearScreen();

    printBanner("MULTI-FLOOR ROOM AVAILABILITY",
        formatDate(checkInDate) + " to " + formatDate(checkOutDate));

    cout << "\n  A room number means the room is available for the entire stay.\n";
    cout << "  X means the room is occupied for at least one selected night.\n";

    int totalAvailable = 0;

    for (int floor = 3; floor >= 1; floor--)
    {
        cout << "\n  FLOOR " << floor << "\n";
        cout << "  " << string(70, '-') << "\n  ";

        int roomsOnLine = 0;

        for (const Room& room : rooms)
        {
            if (room.roomId.empty()
                || room.roomId[0] - '0' != floor)
            {
                continue;
            }

            bool available = roomAvailableForStay(
                bookings, room.roomId, checkInDate, checkOutDate);

            string displayValue = available ? room.roomId : "X";

            if (roomsOnLine > 0 && roomsOnLine % 6 == 0)
            {
                cout << "\n  ";
            }

            cout << "[" << centeredText(displayValue, 7) << "] ";
            roomsOnLine++;

            if (available)
            {
                totalAvailable++;
            }
        }

        cout << '\n';

        cout << "  " << string(70, '-') << '\n';
    }

    cout << "\n  Legend: [ ROOM ] Available | [   X   ] Occupied\n";
    printResultPanel("AVAILABLE",
        to_string(totalAvailable) + " of "
        + to_string(rooms.size())
        + " rooms available for the entire stay");

    EnterToContinue();
}

namespace
{
    // Create centred text for the room map
    string center(const string& text, int width)
    {
        if (static_cast<int>(text.size()) >= width)
        {
            return text.substr(0, width);
        }
        int left = (width - static_cast<int>(text.size())) / 2;
        return string(left, ' ') + text
            + string(width - left - static_cast<int>(text.size()), ' ');
    }

    // Display a room cell
    string cell(const string& roomId, int width, const set<string>& booked)
    {
        if (booked.count(roomId))
        {
            return center("*" + roomId + "*", width);
        }
        return center(roomId, width);
    }

    // Display the first floor map
    void printFloor1(const set<string>& booked)
    {
        cout << "\n  FLOOR 1\n";
        cout << "  -------------------------------------------------------------\n";
        cout << "  |     |     |     |     |     |     |     |     |     |     |\n";
        cout << "  |" << cell("101", 5, booked) << "|" << cell("102", 5, booked)
            << "|" << cell("103", 5, booked) << "|" << cell("104", 5, booked)
            << "|" << cell("105", 5, booked) << "|" << cell("106", 5, booked)
            << "|" << cell("107", 5, booked) << "|" << cell("108", 5, booked)
            << "|" << cell("109", 5, booked) << "|" << cell("110", 5, booked)
            << "|\n";
        cout << "  |     |     |     |     |     |     |     |     |     |     |\n";
        cout << "  -------------------------------------------------------------\n";
        cout << "\n";
        cout << "  -------------                                   -------------\n";
        cout << "  |           |                                   |           |\n";
        cout << "  |" << cell("111", 11, booked)
            << "|                                   |"
            << cell("114", 11, booked) << "|\n";
        cout << "  |           |                                   |           |\n";
        cout << "  -------------              -------              -------------\n";
        cout << "  |           |              |     |              |           |\n";
        cout << "  |" << cell("112", 11, booked)
            << "|              | LIF |              |"
            << cell("115", 11, booked) << "|\n";
        cout << "  |           |              |     |              |           |\n";
        cout << "  -------------              -------              -------------\n";
        cout << "  |           |                                   |           |\n";
        cout << "  |" << cell("113", 11, booked)
            << "|                                   |"
            << cell("116", 11, booked) << "|\n";
        cout << "  |           |                                   |           |\n";
        cout << "  -------------                                   -------------\n";
        cout << "\n";
        cout << "  -------------------------------------------------------------\n";
        cout << "  |     |     |     |     |     |     |     |     |     |     |\n";
        cout << "  |" << cell("117", 5, booked) << "|" << cell("118", 5, booked)
            << "|" << cell("119", 5, booked) << "|" << cell("120", 5, booked)
            << "|" << cell("121", 5, booked) << "|" << cell("122", 5, booked)
            << "|" << cell("123", 5, booked) << "|" << cell("124", 5, booked)
            << "|" << cell("125", 5, booked) << "|" << cell("126", 5, booked)
            << "|\n";
        cout << "  |     |     |     |     |     |     |     |     |     |     |\n";
        cout << "  -------------------------------------------------------------\n";
    }

    // Display the second floor map
    void printFloor2(const set<string>& booked)
    {
        cout << "\n  FLOOR 2\n";
        cout << "  -------------------------------------------------------------\n";
        cout << "  |                 |           |           |                 |\n";
        cout << "  |" << cell("201", 17, booked)
            << "|" << cell("202", 11, booked)
            << "|" << cell("203", 11, booked)
            << "|" << cell("204", 17, booked) << "|\n";
        cout << "  |                 |           |           |                 |\n";
        cout << "  -------------------------------------------------------------\n";
        cout << "\n";
        cout << "  -------------                                   -------------\n";
        cout << "  |           |                                   |           |\n";
        cout << "  |" << cell("205", 11, booked)
            << "|                                   |"
            << cell("208", 11, booked) << "|\n";
        cout << "  |           |                                   |           |\n";
        cout << "  -------------              -------              -------------\n";
        cout << "  |           |              |     |              |           |\n";
        cout << "  |" << cell("206", 11, booked)
            << "|              | LIF |              |"
            << cell("209", 11, booked) << "|\n";
        cout << "  |           |              |     |              |           |\n";
        cout << "  -------------              -------              -------------\n";
        cout << "  |           |                                   |           |\n";
        cout << "  |" << cell("207", 11, booked)
            << "|                                   |"
            << cell("210", 11, booked) << "|\n";
        cout << "  |           |                                   |           |\n";
        cout << "  -------------                                   -------------\n";
        cout << "\n";
        cout << "  -------------------------------------------------------------\n";
        cout << "  |                 |           |           |                 |\n";
        cout << "  |" << cell("211", 17, booked)
            << "|" << cell("212", 11, booked)
            << "|" << cell("213", 11, booked)
            << "|" << cell("214", 17, booked) << "|\n";
        cout << "  |                 |           |           |                 |\n";
        cout << "  -------------------------------------------------------------\n";
    }

    // Display the third floor map
    void printFloor3(const set<string>& booked)
    {
        cout << "\n  FLOOR 3\n";
        cout << "  -------------------------------------------------------------\n";
        cout << "  |           |           |           |           |           |\n";
        cout << "  |" << cell("301", 11, booked)
            << "|" << cell("302", 11, booked)
            << "|" << cell("303", 11, booked)
            << "|" << cell("304", 11, booked)
            << "|" << cell("305", 11, booked) << "|\n";
        cout << "  |           |           |           |           |           |\n";
        cout << "  -------------------------------------------------------------\n";
        cout << "\n";
        cout << "  -------------                                   -------------\n";
        cout << "  |           |                                   |           |\n";
        cout << "  |" << cell("306", 11, booked)
            << "|                                   |"
            << cell("309", 11, booked) << "|\n";
        cout << "  |           |                                   |           |\n";
        cout << "  -------------              -------              -------------\n";
        cout << "  |           |              |     |              |           |\n";
        cout << "  |" << cell("307", 11, booked)
            << "|              | LIF |              |"
            << cell("310", 11, booked) << "|\n";
        cout << "  |           |              |     |              |           |\n";
        cout << "  -------------              -------              -------------\n";
        cout << "  |           |                                   |           |\n";
        cout << "  |" << cell("308", 11, booked)
            << "|                                   |"
            << cell("311", 11, booked) << "|\n";
        cout << "  |           |                                   |           |\n";
        cout << "  -------------                                   -------------\n";
        cout << "\n";
        cout << "  -------------------------------------------------------------\n";
        cout << "  |           |           |           |           |           |\n";
        cout << "  |" << cell("312", 11, booked)
            << "|" << cell("313", 11, booked)
            << "|" << cell("314", 11, booked)
            << "|" << cell("315", 11, booked)
            << "|" << cell("316", 11, booked) << "|\n";
        cout << "  |           |           |           |           |           |\n";
        cout << "  -------------------------------------------------------------\n";
    }
}

// Display the customer's room location
void viewRoomLocationGuide(const vector<Room>& rooms, const string& customerId)
{
    clearScreen();

    (void)rooms;

    vector<Booking> saved = loadSavedBookings();

    cout << "\n====================================\n";
    cout << "       ROOM LOCATION GUIDE\n";
    cout << "====================================\n";

    // Ask for a valid booking ID
    set<string> bookedRooms;
    string bookingId;
    while (true)
    {
        cout << "\n  Enter your Booking ID to view your room location"
            << " (Enter 0 to back to menu): ";
        getline(cin, bookingId);

        if (bookingId == "0")
        {
            return;
        }

        // Convert the booking ID to uppercase
        string upperId = bookingId;
        for (char& c : upperId)
        {
            c = static_cast<char>(toupper(
                static_cast<unsigned char>(c)));
        }

        bookedRooms.clear();
        bool owned = false;
        for (const Booking& b : saved)
        {
            if (b.customerId == customerId
                && b.status != "CANCELLED")
            {
                string bUpper = b.bookingId;
                for (char& c : bUpper)
                {
                    c = static_cast<char>(toupper(
                        static_cast<unsigned char>(c)));
                }
                if (bUpper == upperId)
                {
                    bookedRooms.insert(b.roomId);
                    owned = true;
                }
            }
        }

        if (owned)
        {
            break;
        }
        cout << "Invalid booking ID. Try again.\n";
    }

    cout << "\n  Rooms marked with ** are yours.\n";

    bool showFloor1 = false;
    bool showFloor2 = false;
    bool showFloor3 = false;
    for (const string& roomId : bookedRooms)
    {
        if (roomId[0] == '1') showFloor1 = true;
        else if (roomId[0] == '2') showFloor2 = true;
        else if (roomId[0] == '3') showFloor3 = true;
    }

    if (showFloor1) printFloor1(bookedRooms);
    if (showFloor2) printFloor2(bookedRooms);
    if (showFloor3) printFloor3(bookedRooms);

    // Display the customer's assigned rooms
    cout << "\n  Your room : ";
    bool first = true;
    for (const string& roomId : bookedRooms)
    {
        if (!first)
        {
            cout << ", ";
        }
        cout << roomId;
        first = false;
    }
    cout << "\n";

    cout << "\n  Press ENTER to return...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

// Load the hotel's room data
void loadSchedulerDemoData(vector<Room>& rooms, vector<Booking>& bookings)
{
    (void)bookings;

    rooms = {
        { "101", ROOM_SS, 50.0, 1 },
        { "102", ROOM_SS, 50.0, 1 },
        { "103", ROOM_SS, 50.0, 1 },
        { "104", ROOM_SS, 50.0, 1 },
        { "105", ROOM_SS, 50.0, 1 },
        { "106", ROOM_SS, 50.0, 1 },
        { "107", ROOM_SS, 50.0, 1 },

        { "108", ROOM_SD, 80.0, 2 },
        { "109", ROOM_SD, 80.0, 2 },
        { "110", ROOM_SD, 80.0, 2 },
        { "111", ROOM_SD, 80.0, 2 },
        { "112", ROOM_SD, 80.0, 2 },
        { "113", ROOM_SD, 80.0, 2 },
        { "114", ROOM_SD, 80.0, 2 },
        { "115", ROOM_SD, 80.0, 2 },
        { "116", ROOM_SD, 80.0, 2 },
        { "117", ROOM_SS, 50.0, 1 },
        { "118", ROOM_SS, 50.0, 1 },
        { "119", ROOM_SS, 50.0, 1 },
        { "120", ROOM_SS, 50.0, 1 },
        { "121", ROOM_SS, 50.0, 1 },
        { "122", ROOM_SS, 50.0, 1 },
        { "123", ROOM_SS, 50.0, 1 },
        { "124", ROOM_SS, 50.0, 1 },
        { "125", ROOM_SS, 50.0, 1 },
        { "126", ROOM_SS, 50.0, 1 },

        { "201", ROOM_FS, 150.0, 4 },
        { "202", ROOM_SD, 80.0, 2 },
        { "203", ROOM_SD, 80.0, 2 },
        { "204", ROOM_FS, 150.0, 4 },
        { "205", ROOM_SD, 80.0, 2 },
        { "206", ROOM_SD, 80.0, 2 },
        { "207", ROOM_SD, 80.0, 2 },
        { "208", ROOM_SD, 80.0, 2 },
        { "209", ROOM_SD, 80.0, 2 },
        { "210", ROOM_SD, 80.0, 2 },
        { "211", ROOM_FS, 150.0, 4 },
        { "212", ROOM_SD, 80.0, 2 },
        { "213", ROOM_SD, 80.0, 2 },
        { "214", ROOM_FS, 150.0, 4 },

        { "301", ROOM_PS, 300.0, 2 },
        { "302", ROOM_DQ, 120.0, 2 },
        { "303", ROOM_DQ, 120.0, 2 },
        { "304", ROOM_DQ, 120.0, 2 },
        { "305", ROOM_PS, 300.0, 2 },
        { "306", ROOM_DQ, 120.0, 2 },
        { "307", ROOM_DQ, 120.0, 2 },
        { "308", ROOM_DQ, 120.0, 2 },
        { "309", ROOM_DQ, 120.0, 2 },
        { "310", ROOM_DQ, 120.0, 2 },
        { "311", ROOM_DQ, 120.0, 2 },
        { "312", ROOM_DQ, 120.0, 2 },
        { "313", ROOM_PS, 300.0, 2 },
        { "314", ROOM_DQ, 120.0, 2 },
        { "315", ROOM_DQ, 120.0, 2 },
        { "316", ROOM_PS, 300.0, 2 }
    };
    bookings.clear();
}

// Display scheduler options for customers
void customerSchedulerMenu(const vector<Room>& rooms,
    const vector<Booking>& bookings)
{
    while (true)
    {
        clearScreen();

        printBanner("SCHEDULER & APPOINTMENT MANAGEMENT", "CUSTOMER PORTAL");
        printMenuOption(1, "WEEKLY HEATMAP", "Compare seven days of occupancy");
        printMenuOption(2, "MONTHLY AVAILABILITY", "Browse a calendar of free rooms");
        printMenuOption(3, "FLOOR AVAILABILITY MAP", "Check every room for a date range");
        cout << '|' << string(UI_WIDTH - 2, '-') << "|\n";
        printMenuOption(0, "RETURN", "Exit Scheduler");
        cout << '+' << string(UI_WIDTH - 2, '=') << "+\n";
        int choice = readInteger("Choice: ", 0, 3);

        if (choice == 0)
        {
            return;
        }
        if (choice == 1)
        {
            viewWeeklyHeatmap(bookings, rooms,
                readCurrentOrFutureDate("Enter the first date of the week"));
        }
        else if (choice == 2)
        {
            int year;
            int month;
            while (true)
            {
                year = readInteger("Year (2026-2028): ",
                    CUSTOMER_MIN_YEAR, MAX_SCHEDULE_YEAR);
                month = readInteger("Month (1-12): ", 1, 12);
                Date monthEnd{ daysInMonth(month, year), month, year };
                if (compareDates(monthEnd, SYSTEM_DATE) >= 0)
                {
                    break;
                }
                cout << "That month is entirely in the past. Select August 2026 or later.\n";
            }
            viewMonthlyAvailability(bookings, rooms, month, year);
        }
        else
        {
            Date checkInDate = readCurrentOrFutureDate("Enter check-in date");
            Date checkOutDate = readCurrentOrFutureDate("Enter check-out date");
            while (compareDates(checkOutDate, checkInDate) <= 0)
            {
                cout << "Check-out date must be later than check-in date.\n";
                checkOutDate = readCurrentOrFutureDate("Re-enter check-out date");
            }
            viewFloorAvailabilityMap(bookings, rooms, checkInDate, checkOutDate);
        }
    }
}

// Display scheduler options for managers
void managerSchedulerMenu(const vector<Room>& rooms,
    const vector<Booking>& bookings)
{
    while (true)
    {
        clearScreen();

        printBanner("SCHEDULER & APPOINTMENT MANAGEMENT", "MANAGER CONTROL DESK");
        printMenuOption(1, "DAILY SCHEDULE", "Inspect room operations by date");
        printMenuOption(2, "ROOM TIMELINE", "Track one room across a month");
        printMenuOption(3, "OCCUPANCY FORECAST", "Project demand and peak dates");

        cout << '|' << string(UI_WIDTH - 2, '-') << "|\n";
        printMenuOption(0, "RETURN", "Exit Scheduler");
        cout << '+' << string(UI_WIDTH - 2, '=') << "+\n";
        int choice = readInteger("Choice: ", 0, 3);

        if (choice == 0)
        {
            return;
        }
        if (choice == 1)
        {
            viewDailyRoomSchedule(bookings, rooms,
                readDate("Enter schedule date", MANAGER_MIN_YEAR, MAX_SCHEDULE_YEAR));
        }
        else if (choice == 2)
        {
            string roomId = readRoomId(rooms);
            int year = readInteger("Year (2020-2028): ",
                MANAGER_MIN_YEAR, MAX_SCHEDULE_YEAR);
            int month = readInteger("Month (1-12): ", 1, 12);
            viewRoomMonthlyTimeline(bookings, rooms, roomId, month, year);
        }
        else if (choice == 3)
        {
            Date startDate = readCurrentOrFutureDate("Enter forecast start date");
            int days = readInteger("Forecast days (1-31): ", 1, 31);
            viewOccupancyForecast(bookings, rooms, startDate, days);
        }

    }
}