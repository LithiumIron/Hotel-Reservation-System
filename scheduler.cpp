#include "scheduler.h"

#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <vector>

using namespace std;

namespace
{
    const int UI_WIDTH = 82;
    const int CUSTOMER_MIN_YEAR = 2026;
    const int EMPLOYEE_MIN_YEAR = 2020;
    const int MAX_SCHEDULE_YEAR = 2028;
    const Date SYSTEM_DATE{ 14, 8, 2026 };
    const string STATUS_PENDING = "PENDING";
    const string STATUS_CONFIRMED = "CONFIRMED";
    const string STATUS_CHECKED_IN = "CHECKED_IN";
    const string STATUS_CANCELLED = "CANCELLED";
    const string STATUS_COMPLETED = "COMPLETED";

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

    void printMenuOption(int option, const string& title, const string& description)
    {
        ostringstream label;
        label << "[" << option << "] " << title;
        cout << "|  " << left << setw(32) << label.str()
             << setw(UI_WIDTH - 36) << description << "|\n";
    }

    string occupancyBar(int percentage, int width = 20)
    {
        int filled = percentage * width / 100;
        return "[" + string(filled, '#') + string(width - filled, '.') + "]";
    }

    void printResultPanel(const string& label, const string& message)
    {
        cout << '+' << string(UI_WIDTH - 2, '-') << "+\n";
        cout << "| " << left << setw(14) << label << setw(UI_WIDTH - 17)
             << message << "|\n";
        cout << '+' << string(UI_WIDTH - 2, '-') << "+\n";
    }

    bool isRoomBlocked(const Booking& booking)
    {
        return booking.status == STATUS_PENDING
            || booking.status == STATUS_CONFIRMED
            || booking.status == STATUS_CHECKED_IN;
    }

    bool bookingOccupiesDate(const Booking& booking, const Date& date)
    {
        return booking.status != STATUS_CANCELLED
            && compareDates(booking.checkInDate, date) <= 0
            && compareDates(date, booking.checkOutDate) < 0;
    }

    bool dateRangesOverlap(const Date& firstCheckIn, const Date& firstCheckOut,
        const Date& secondCheckIn, const Date& secondCheckOut)
    {
        return compareDates(firstCheckIn, secondCheckOut) < 0
            && compareDates(secondCheckIn, firstCheckOut) < 0;
    }

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

    int readInteger(const string& prompt, int minimum, int maximum)
    {
        while (true)
        {
            string input;
            cout << prompt;
            getline(cin, input);

            try
            {
                size_t processedCharacters = 0;
                int value = stoi(input, &processedCharacters);
                if (processedCharacters == input.length()
                    && value >= minimum && value <= maximum)
                {
                    return value;
                }
            }
            catch (...)
            {
            }

            cout << "Invalid input. Enter a number from " << minimum
                 << " to " << maximum << ".\n";
        }
    }

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
            cout << "Room ID not found. Available rooms: ";
            for (const Room& room : rooms)
            {
                cout << room.roomId << ' ';
            }
            cout << '\n';
        }
    }

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

    string dayName(const Date& date)
    {
        static const string names[] =
            { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
        return names[dayIndexForDate(date)];
    }
}

bool isLeapYear(int year)
{
    return year % 400 == 0 || (year % 4 == 0 && year % 100 != 0);
}

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

bool isValidDate(const Date& date)
{
    return date.year >= 1 && date.month >= 1 && date.month <= 12
        && date.day >= 1 && date.day <= daysInMonth(date.month, date.year);
}

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

string formatDate(const Date& date)
{
    ostringstream output;
    output << setfill('0') << setw(2) << date.day << '/'
           << setw(2) << date.month << '/' << date.year;
    return output.str();
}

void viewWeeklyHeatmap(const vector<Booking>& bookings,
    const vector<Room>& rooms, const Date& startDate)
{
    const int DAYS = 7;
    const int MAX_ROOMS = 50;
    int occupancy[DAYS][MAX_ROOMS] = {};

    printBanner("WEEKLY OCCUPANCY HEATMAP");
    cout << "  Legend: . Empty | L Low | M Moderate | H High | F Full\n\n";
    cout << "  " << left << setw(17) << "DATE" << setw(8) << "LEVEL"
         << setw(12) << "ROOMS" << setw(24) << "VISUAL LOAD" << "RATE\n";
    cout << "  " << string(76, '-') << '\n';

    int roomLimit = rooms.size() < MAX_ROOMS
        ? static_cast<int>(rooms.size()) : MAX_ROOMS;

    for (int dayIndex = 0; dayIndex < DAYS; dayIndex++)
    {
        Date date = addDays(startDate, dayIndex);
        int occupiedCount = 0;

        for (int roomIndex = 0; roomIndex < roomLimit; roomIndex++)
        {
            if (findBookingForRoom(bookings, rooms[roomIndex].roomId, date))
            {
                occupancy[dayIndex][roomIndex] = 1;
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
}

void viewMonthlyAvailability(const vector<Booking>& bookings,
    const vector<Room>& rooms, int month, int year)
{
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
}

void viewDailyRoomSchedule(const vector<Booking>& bookings,
    const vector<Room>& rooms, const Date& selectedDate)
{
    printBanner("DAILY ROOM SCHEDULE");
    cout << "  Schedule date: " << dayName(selectedDate) << ' '
         << formatDate(selectedDate) << "\n\n";
    cout << "  " << left << setw(8) << "ROOM" << setw(13) << "TYPE"
         << setw(13) << "BOOKING" << setw(13) << "CUSTOMER" << "STATUS\n";
    cout << "  " << string(66, '-') << '\n';

    for (const Room& room : rooms)
    {
        const Booking* booking = findBookingForRoom(bookings, room.roomId, selectedDate);
        cout << "  " << left << setw(8) << room.roomId << setw(13) << room.roomType;
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
}

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
}

void viewOccupancyForecast(const vector<Booking>& bookings,
    const vector<Room>& rooms, const Date& startDate, int numberOfDays)
{
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
}

int autoReleaseExpiredBookings(vector<Booking>& bookings,
    const Date& currentDate)
{
    vector<Booking> ignoredReleasedBookings;
    return autoReleaseExpiredBookings(bookings, currentDate,
        ignoredReleasedBookings);
}

int autoReleaseExpiredBookings(vector<Booking>& bookings,
    const Date& currentDate, vector<Booking>& releasedBookings)
{
    int releasedCount = 0;
    releasedBookings.clear();
    for (Booking& booking : bookings)
    {
        if (booking.status == STATUS_PENDING && !booking.paid
            && compareDates(booking.expiryDate, currentDate) < 0)
        {
            booking.status = STATUS_CANCELLED;
            releasedBookings.push_back(booking);
            releasedCount++;
        }
    }
    return releasedCount;
}

void viewFloorAvailabilityMap(const vector<Booking>& bookings,
    const vector<Room>& rooms, const Date& checkInDate,
    const Date& checkOutDate)
{
    printBanner("MULTI-FLOOR ROOM AVAILABILITY",
        formatDate(checkInDate) + " to " + formatDate(checkOutDate));
    cout << "\n  A displayed room number means FREE for the whole stay.\n";
    cout << "  X means the room is occupied for at least one selected night.\n\n";

    for (int floor = 3; floor >= 1; floor--)
    {
        cout << "  +" << string(68, '-') << "+\n";
        ostringstream floorRow;
        floorRow << " FLOOR " << floor << "     ";

        for (int roomNumber = 1; roomNumber <= 3; roomNumber++)
        {
            string roomId = to_string(floor * 100 + roomNumber);
            bool roomExists = false;
            for (const Room& room : rooms)
            {
                if (room.roomId == roomId)
                {
                    roomExists = true;
                    break;
                }
            }

            string displayValue = "N/A";
            if (roomExists)
            {
                displayValue = roomAvailableForStay(bookings, roomId,
                    checkInDate, checkOutDate) ? roomId : "X";
            }

            floorRow << "[" << centeredText(displayValue, 9) << "]";
            if (roomNumber < 3)
            {
                floorRow << "     ";
            }
        }
        cout << "  |" << left << setw(68) << floorRow.str() << "|\n";
    }
    cout << "  +" << string(68, '-') << "+\n";
    cout << "\n  Legend: [ ROOM NO. ] Available  |  [    X    ] Occupied\n";
}

int auditDoubleBookings(const vector<Booking>& bookings)
{
    int conflictCount = 0;
    printBanner("DOUBLE-BOOKING CONFLICT AUDIT", "Integrity and overlap control");
    cout << "  " << left << setw(10) << "ROOM" << setw(14) << "BOOKING A"
         << setw(14) << "Booking B" << "Overlap\n";
    cout << "  " << string(64, '-') << '\n';

    for (size_t first = 0; first < bookings.size(); first++)
    {
        if (!isRoomBlocked(bookings[first]))
        {
            continue;
        }
        for (size_t second = first + 1; second < bookings.size(); second++)
        {
            if (bookings[first].roomId == bookings[second].roomId
                && isRoomBlocked(bookings[second])
                && dateRangesOverlap(bookings[first].checkInDate,
                    bookings[first].checkOutDate, bookings[second].checkInDate,
                    bookings[second].checkOutDate))
            {
                conflictCount++;
                Date overlapStart = compareDates(bookings[first].checkInDate,
                    bookings[second].checkInDate) > 0
                    ? bookings[first].checkInDate : bookings[second].checkInDate;
                Date overlapEnd = compareDates(bookings[first].checkOutDate,
                    bookings[second].checkOutDate) < 0
                    ? bookings[first].checkOutDate : bookings[second].checkOutDate;

                cout << "  " << left << setw(10) << bookings[first].roomId
                     << setw(14) << bookings[first].bookingId
                     << setw(14) << bookings[second].bookingId
                     << formatDate(overlapStart) << " to "
                     << formatDate(overlapEnd) << '\n';
            }
        }
    }

    if (conflictCount == 0)
    {
        printResultPanel("AUDIT PASS", "No active double-booking conflict detected");
    }
    else
    {
        printResultPanel("ACTION NEEDED", to_string(conflictCount)
            + " conflict(s) require employee action");
    }
    return conflictCount;
}

void loadHotelRooms(vector<vector<Room>>& hotelRooms)
{
    // hotelRooms[floor][room]: 3 floors x 3 rooms
    // bookingStatus is the occupancy snapshot for system date 14/08/2026
    hotelRooms = {
        {
            { "101", "Standard", 180.0, true },
            { "102", "Standard", 180.0, false },
            { "103", "Standard", 180.0, false }
        },
        {
            { "201", "Deluxe", 260.0, true },
            { "202", "Deluxe", 260.0, false },
            { "203", "Deluxe", 260.0, false }
        },
        {
            { "301", "Suite", 420.0, false },
            { "302", "Suite", 420.0, true },
            { "303", "Suite", 420.0, false }
        }
    };
}

void loadSchedulerDemoData(vector<Room>& rooms, vector<Booking>& bookings)
{
    vector<vector<Room>> hotelRooms;
    loadHotelRooms(hotelRooms);

    rooms.clear();
    for (const vector<Room>& floor : hotelRooms)
    {
        for (const Room& room : floor)
        {
            rooms.push_back(room);
        }
    }

    bookings = {
        { "B001", "C001", "101", {10,8,2026}, {14,8,2026}, {17,8,2026}, {11,8,2026}, STATUS_CONFIRMED, true },
        { "B002", "C002", "102", {11,8,2026}, {15,8,2026}, {19,8,2026}, {12,8,2026}, STATUS_CONFIRMED, true },
        { "B003", "C003", "201", {12,8,2026}, {14,8,2026}, {16,8,2026}, {13,8,2026}, STATUS_CHECKED_IN, true },
        { "B004", "C004", "202", {12,8,2026}, {16,8,2026}, {18,8,2026}, {13,8,2026}, STATUS_PENDING, false },
        { "B005", "C005", "301", {13,8,2026}, {18,8,2026}, {22,8,2026}, {16,8,2026}, STATUS_PENDING, false },
        { "B006", "C006", "302", {10,8,2026}, {14,8,2026}, {20,8,2026}, {11,8,2026}, STATUS_CONFIRMED, true },
        { "B007", "C007", "103", {10,8,2026}, {20,8,2026}, {24,8,2026}, {11,8,2026}, STATUS_CONFIRMED, true },
        { "B008", "C008", "101", {13,8,2026}, {15,8,2026}, {16,8,2026}, {14,8,2026}, STATUS_CONFIRMED, true },
        { "B009", "C009", "203", {10,5,2025}, {20,5,2025}, {23,5,2025}, {11,5,2025}, STATUS_COMPLETED, true }
    };
}

void customerSchedulerMenu(const vector<Room>& rooms,
    const vector<Booking>& bookings)
{
    while (true)
    {
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

void employeeSchedulerMenu(const vector<Room>& rooms,
    vector<Booking>& bookings)
{
    while (true)
    {
        printBanner("SCHEDULER & APPOINTMENT MANAGEMENT", "EMPLOYEE CONTROL DESK");
        printMenuOption(1, "DAILY SCHEDULE", "Inspect room operations by date");
        printMenuOption(2, "ROOM TIMELINE", "Track one room across a month");
        printMenuOption(3, "OCCUPANCY FORECAST", "Project demand and peak dates");
        printMenuOption(4, "AUTO-RELEASE", "Cancel expired unpaid reservations");
        printMenuOption(5, "CONFLICT AUDIT", "Detect overlapping room bookings");
        cout << '|' << string(UI_WIDTH - 2, '-') << "|\n";
        printMenuOption(0, "RETURN", "Exit Scheduler");
        cout << '+' << string(UI_WIDTH - 2, '=') << "+\n";
        int choice = readInteger("Choice: ", 0, 5);

        if (choice == 0)
        {
            return;
        }
        if (choice == 1)
        {
            viewDailyRoomSchedule(bookings, rooms,
                readDate("Enter schedule date", EMPLOYEE_MIN_YEAR, MAX_SCHEDULE_YEAR));
        }
        else if (choice == 2)
        {
            string roomId = readRoomId(rooms);
            int year = readInteger("Year (2020-2028): ",
                EMPLOYEE_MIN_YEAR, MAX_SCHEDULE_YEAR);
            int month = readInteger("Month (1-12): ", 1, 12);
            viewRoomMonthlyTimeline(bookings, rooms, roomId, month, year);
        }
        else if (choice == 3)
        {
            Date startDate = readCurrentOrFutureDate("Enter forecast start date");
            int days = readInteger("Forecast days (1-31): ", 1, 31);
            viewOccupancyForecast(bookings, rooms, startDate, days);
        }
        else if (choice == 4)
        {
            vector<Booking> releasedBookings;
            int released = autoReleaseExpiredBookings(bookings, SYSTEM_DATE,
                releasedBookings);

            printBanner("AUTO-RELEASE REPORT",
                "System date: " + formatDate(SYSTEM_DATE));
            if (releasedBookings.empty())
            {
                printResultPanel("NO ACTION",
                    "No expired unpaid PENDING booking found");
            }
            else
            {
                cout << "  " << left << setw(10) << "BOOKING" << setw(11)
                     << "CUSTOMER" << setw(8) << "ROOM" << setw(13)
                     << "CHECK-IN" << setw(13) << "CHECK-OUT" << "RESULT\n";
                cout << "  " << string(76, '-') << '\n';

                for (const Booking& booking : releasedBookings)
                {
                    cout << "  " << left << setw(10) << booking.bookingId
                         << setw(11) << booking.customerId << setw(8)
                         << booking.roomId << setw(13)
                         << formatDate(booking.checkInDate) << setw(13)
                         << formatDate(booking.checkOutDate)
                         << "PENDING -> CANCELLED\n";
                    cout << "  Payment deadline expired on "
                         << formatDate(booking.expiryDate) << ". Room "
                         << booking.roomId << " is now available.\n";
                }

                printResultPanel("RELEASED", to_string(released)
                    + " unpaid room hold(s) cancelled and made available");
            }
        }
        else
        {
            auditDoubleBookings(bookings);
        }
    }
}
