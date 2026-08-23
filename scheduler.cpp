#include "scheduler.h"
#include "utilities.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <vector>

using namespace std;

Date getCurrentSystemDate()
{
    // Get current time from system
    time_t now = time(nullptr);
    tm* localTime = localtime(&now);
    
    Date today;
    today.day = localTime->tm_mday;
    today.month = localTime->tm_mon + 1;  // tm_mon is 0-11
    today.year = localTime->tm_year + 1900;  // tm_year is years since 1900
    
    return today;
}
const int UI_WIDTH = 82;
const int CUSTOMER_MIN_YEAR = 2026;
const int EMPLOYEE_MIN_YEAR = 2020;
const int MAX_SCHEDULE_YEAR = 2028;
const Date SYSTEM_DATE= getCurrentSystemDate();
const string STATUS_PENDING = "PENDING";
const string STATUS_CONFIRMED = "CONFIRMED";
const string STATUS_CHECKED_IN = "CHECKED_IN";
const string STATUS_CANCELLED = "CANCELLED";
const string STATUS_COMPLETED = "COMPLETED";

const string ROOM_SS = "Standard Single";
const string ROOM_SD = "Standard Double";
const string ROOM_DQ = "Deluxe Queen";
const string ROOM_FS = "Family Suite";
const string ROOM_PS = "Presidential Suite";

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



void viewFloorAvailabilityMap(const vector<Booking>& bookings,
    const vector<Room>& rooms, const Date& checkInDate,
    const Date& checkOutDate)
{
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
}







namespace
{
    struct MapRoom
    {
        string roomId;
        string capLabel;
        int width;
    };

    void renderRoomBlock(const MapRoom& room, bool windowOnTop,
        vector<string>& lines)
    {
        string windowEdge(room.width - 2,
            windowOnTop ? '=' : '-');
        string plainEdge(room.width - 2, '-');
        lines.push_back("+" + windowEdge + "+");
        lines.push_back("|"
            + centeredText(room.roomId + " " + room.capLabel,
                room.width - 2) + "|");
        lines.push_back("+" + plainEdge + "+");
    }

    void renderRoomBand(const vector<MapRoom>& band, bool windowOnTop)
    {
        string rows[3];
        for (const MapRoom& room : band)
        {
            vector<string> block;
            renderRoomBlock(room, windowOnTop, block);
            for (int i = 0; i < 3; i++)
            {
                rows[i] += block[i];
            }
        }
        for (int i = 0; i < 3; i++)
        {
            cout << "  " << rows[i] << '\n';
        }
    }

    void renderCorridorWithLift(int totalWidth, int liftWidth)
    {
        int sideWidth = (totalWidth - liftWidth) / 2;
        string grass(sideWidth, '.');
        string liftEdge(liftWidth - 2, '=');
        cout << "  " << grass << "+" << liftEdge << "+\n";
        cout << "  " << grass << "|"
             << centeredText("LIFT", liftWidth - 2) << "|\n";
        cout << "  " << grass << "+" << liftEdge << "+\n";
    }

    void renderFloorMap(int floorNumber,
        const vector<MapRoom>& topBand,
        const vector<MapRoom>& bottomBand)
    {
        cout << "\n  FLOOR " << floorNumber << "\n";
        renderRoomBand(topBand, true);
        renderCorridorWithLift(72, 12);
        renderRoomBand(bottomBand, false);
    }
}

void viewRoomLocationGuide(const vector<Room>& rooms)
{
    (void)rooms;

    printBanner("ROOM LOCATION GUIDE",
        "All rooms face an exterior window wall");

    cout << "  Room number = FLOOR + ROOM (e.g. 205 = Floor 2, Room 05).\n";
    cout << "  '=' marks the window wall. Block size = room size:\n";
    cout << "  [1P] 1-person (small)   [2P] 2-person (medium)"
        << "   [4P] 4-person (large)\n";

    // Floor 1: 1-person and 2-person only (band widths: 4x12 + 3x8 = 72)
    vector<MapRoom> f1Top = {
        { "101", "2P", 12 }, { "102", "2P", 12 },
        { "103", "2P", 12 }, { "104", "2P", 12 },
        { "105", "1P", 8 },  { "106", "1P", 8 },  { "107", "1P", 8 }
    };
    vector<MapRoom> f1Bottom = {
        { "108", "2P", 12 }, { "109", "2P", 12 },
        { "110", "2P", 12 }, { "111", "2P", 12 },
        { "112", "1P", 8 },  { "113", "1P", 8 },  { "114", "1P", 8 }
    };

    // Floor 2: 2-person and 4-person only (band widths: 2x18 + 3x12 = 72)
    vector<MapRoom> f2Top = {
        { "201", "4P", 18 }, { "202", "4P", 18 },
        { "203", "2P", 12 }, { "204", "2P", 12 }, { "205", "2P", 12 }
    };
    vector<MapRoom> f2Bottom = {
        { "206", "4P", 18 }, { "207", "4P", 18 },
        { "208", "2P", 12 }, { "209", "2P", 12 }, { "210", "2P", 12 }
    };

    // Floor 3: all 2-person (band widths: 6x12 = 72)
    vector<MapRoom> f3Top = {
        { "301", "2P", 12 }, { "302", "2P", 12 }, { "303", "2P", 12 },
        { "304", "2P", 12 }, { "305", "2P", 12 }, { "306", "2P", 12 }
    };
    vector<MapRoom> f3Bottom = {
        { "307", "2P", 12 }, { "308", "2P", 12 }, { "309", "2P", 12 },
        { "310", "2P", 12 }, { "311", "2P", 12 }, { "312", "2P", 12 }
    };

    renderFloorMap(1, f1Top, f1Bottom);
    renderFloorMap(2, f2Top, f2Bottom);
    renderFloorMap(3, f3Top, f3Bottom);

    cout << "\n  Legend: '.' corridor  |  centre block = lift core"
        << "  |  '=' window wall\n";
    cout << "  Every floor occupies the same total area"
        << " (72 x 9 units incl. lift core).\n";
    cout << "\n  FLOOR SUMMARY\n";
    cout << "  Floor 1: 6 x 1-person, 8 x 2-person  (14 rooms)\n";
    cout << "  Floor 2: 6 x 2-person, 4 x 4-person  (10 rooms)\n";
    cout << "  Floor 3: 12 x 2-person               (12 rooms)\n";
    cout << "  Total:   6 x 1-person, 20 x 2-person, 4 x 4-person"
        << "  (36 rooms)\n";

    cout << "\n  Press ENTER to return...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void loadSchedulerDemoData(vector<Room>& rooms, vector<Booking>& bookings)
{
    // Central hotel room catalogue shared with Booking and Scheduler.
    rooms = {
        // Floor 1: Standard rooms
        { "101", ROOM_SD, 80.0, 2 },
        { "102", ROOM_SD, 80.0, 2 },
        { "103", ROOM_SD, 80.0, 2 },
        { "104", ROOM_SD, 80.0, 2 },
        { "105", ROOM_SS, 50.0, 1 },
        { "106", ROOM_SS, 50.0, 1 },
        { "107", ROOM_SS, 50.0, 1 },

        { "108", ROOM_SD, 80.0, 2 },
        { "109", ROOM_SD, 80.0, 2 },
        { "110", ROOM_SD, 80.0, 2 },
        { "111", ROOM_SD, 80.0, 2 },
        { "112", ROOM_SS, 50.0, 1 },
        { "113", ROOM_SS, 50.0, 1 },
        { "114", ROOM_SS, 50.0, 1 },

        // Floor 2: Deluxe and family rooms
        { "201", ROOM_FS, 200.0, 4 },
        { "202", ROOM_FS, 200.0, 4 },
        { "203", ROOM_DQ, 120.0, 2 },
        { "204", ROOM_DQ, 120.0, 2 },
        { "205", ROOM_DQ, 120.0, 2 },

        { "206", ROOM_FS, 200.0, 4 },
        { "207", ROOM_FS, 200.0, 4 },
        { "208", ROOM_DQ, 120.0, 2 },
        { "209", ROOM_DQ, 120.0, 2 },
        { "210", ROOM_DQ, 120.0, 2 },

        // Floor 3: Presidential suites
        { "301", ROOM_PS, 500.0, 2 },
        { "302", ROOM_PS, 500.0, 2 },
        { "303", ROOM_PS, 500.0, 2 },
        { "304", ROOM_PS, 500.0, 2 },
        { "305", ROOM_PS, 500.0, 2 },
        { "306", ROOM_PS, 500.0, 2 },
        { "307", ROOM_PS, 500.0, 2 },
        { "308", ROOM_PS, 500.0, 2 },
        { "309", ROOM_PS, 500.0, 2 },
        { "310", ROOM_PS, 500.0, 2 },
        { "311", ROOM_PS, 500.0, 2 },
        { "312", ROOM_PS, 500.0, 2 }
    };

    // Real bookings are loaded from bookingData.txt by Booking Module.
    // Scheduler must not create fake reservations.
    bookings.clear();
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
    const vector<Booking>& bookings)
{
    while (true)
    {
        printBanner("SCHEDULER & APPOINTMENT MANAGEMENT", "EMPLOYEE CONTROL DESK");
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
        
    }
}
