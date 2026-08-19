#include "booking.h"
#include "scheduler.h"
#include "utilities.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

namespace
{
    const Date SYSTEM_DATE{ 19, 8, 2026 };
    const Date GO_BACK{ 0, 0, 0 };

    enum BookingStage
    {
        STAGE_CHECKIN,
        STAGE_CHECKOUT,
        STAGE_ROOMS,
        STAGE_ADDONS,
        STAGE_SUMMARY,
        STAGE_EXIT
    };

    struct RoomTypeOption
    {
        string name;
        int capacity;
        double price;
    };

    const RoomTypeOption ROOM_TYPES[] = {
        { "Standard Single", 1, 50.0 },
        { "Standard Double", 2, 80.0 },
        { "Deluxe Queen", 2, 120.0 },
        { "Family Suite", 4, 200.0 },
        { "Presidential Suite", 2, 500.0 }
    };
    const int NUM_ROOM_TYPES = 5;

    bool isGoBack(const Date& d)
    {
        return d.day == 0 && d.month == 0 && d.year == 0;
    }

    Date readDate(const string& heading,
        const string& backLabel)
    {
        while (true)
        {
            cout << '\n' << heading
                 << "  (Enter Z to go back to "
                 << backLabel << ")\n";
            cout << "Enter date (DD/MM/YYYY): ";
            string input;
            getline(cin, input);

            if (input == "Z" || input == "z")
            {
                return GO_BACK;
            }

            if (input.length() != 10
                || input[2] != '/' || input[5] != '/')
            {
                cout << "Invalid format. Please use DD/MM/YYYY.\n";
                continue;
            }

            istringstream parser(input);
            int day, month, year;
            char slash1, slash2;

            if (!(parser >> day >> slash1 >> month
                >> slash2 >> year))
            {
                cout << "Invalid date. Please enter numeric"
                     << " values.\n";
                continue;
            }

            Date date{ day, month, year };

            if (!isValidDate(date))
            {
                cout << "Invalid date. Please check"
                     << " day/month/year.\n";
                continue;
            }

            if (year < 2026 || year > 2028)
            {
                cout << "Year must be between 2026 and 2028.\n";
                continue;
            }

            return date;
        }
    }

    Date readCurrentOrFutureDate(const string& heading,
        const string& backLabel)
    {
        while (true)
        {
            Date date = readDate(heading, backLabel);
            if (isGoBack(date))
            {
                return date;
            }
            if (compareDates(date, SYSTEM_DATE) >= 0)
            {
                return date;
            }
            cout << "Past dates are not allowed. Earliest date: "
                 << formatDate(SYSTEM_DATE) << ".\n";
        }
    }

    int readQuantityOrBack(int maxVal,
        const string& backLabel)
    {
        while (true)
        {
            cout << "Quantity (Enter Z to go back to "
                 << backLabel << "): ";
            string input;
            getline(cin, input);
            if (input == "Z" || input == "z")
            {
                return -1;
            }
            try
            {
                size_t processed = 0;
                int value = stoi(input, &processed);
                if (processed == input.length()
                    && value >= 1 && value <= maxVal)
                {
                    return value;
                }
            }
            catch (...) {}
            cout << "Invalid. Enter 1-" << maxVal << ".\n";
        }
    }

    char readLetterOrBack(char minCh, char maxCh,
        const string& backLabel)
    {
        string range;
        range += minCh;
        range += '-';
        range += maxCh;

        while (true)
        {
            cout << "Select (" << range
                 << ", Enter Z to go back to "
                 << backLabel << "): ";
            string input;
            getline(cin, input);
            if (input == "Z" || input == "z")
            {
                return 'Z';
            }
            if (input.length() == 1 && input[0] >= minCh
                && input[0] <= maxCh)
            {
                return input[0];
            }
            cout << "Invalid. Please enter " << range << ".\n";
        }
    }

    bool isBookingActive(const Booking& booking)
    {
        return booking.status == "PENDING"
            || booking.status == "CONFIRMED"
            || booking.status == "CHECKED_IN";
    }

    bool datesOverlap(const Date& a1, const Date& a2,
        const Date& b1, const Date& b2)
    {
        return compareDates(a1, b2) < 0
            && compareDates(b1, a2) < 0;
    }

    bool isRoomAvailable(const vector<Booking>& bookings,
        const string& roomId,
        const Date& checkIn, const Date& checkOut)
    {
        for (const Booking& b : bookings)
        {
            if (b.roomId == roomId && isBookingActive(b)
                && datesOverlap(checkIn, checkOut,
                    b.checkInDate, b.checkOutDate))
            {
                return false;
            }
        }
        return true;
    }

    int countAvailableByType(const vector<Room>& rooms,
        const vector<Booking>& bookings,
        const string& roomType,
        const Date& checkIn, const Date& checkOut)
    {
        int count = 0;
        for (const Room& room : rooms)
        {
            if (room.roomType == roomType
                && isRoomAvailable(bookings, room.roomId,
                    checkIn, checkOut))
            {
                count++;
            }
        }
        return count;
    }

    struct SelectedRoom
    {
        int typeIndex;
        int quantity;
    };

    struct AddOnOption
    {
        string name;
        string unit;
        double pricePerUnit;
    };

    const AddOnOption ADDONS[] = {
        { "Buffet Breakfast", "person", 35.0 },
        { "Extra Bed", "bed", 10.0 },
        { "Gym & Pool VIP Pass", "person", 20.0 },
        { "High-Speed WiFi", "room", 40.0 }
    };
    const int NUM_ADDONS = 4;

    struct SelectedAddOn
    {
        int addonIndex;
        int quantity;
    };

    // Returns: "valid", "format", or "invalid"
    string validatePhone(const string& input)
    {
        if (input.length() < 5 || input[3] != '-')
        {
            return "format";
        }

        string prefix = input.substr(0, 3);
        string digits = input.substr(4);

        if (prefix == "011")
        {
            if (digits.length() != 8) return "format";
        }
        else
        {
            bool validPrefix =
                prefix == "010" || prefix == "012"
                || prefix == "013" || prefix == "014"
                || prefix == "016" || prefix == "017"
                || prefix == "018" || prefix == "019";

            if (!validPrefix) return "invalid";
            if (digits.length() != 7) return "format";
        }

        for (char c : digits)
        {
            if (!isdigit(c)) return "invalid";
        }

        return "valid";
    }

    string readSixDigitPin(const string& prompt)
    {
        while (true)
        {
            cout << prompt;
            string input;
            getline(cin, input);
            if (input.length() != 6)
            {
                cout << "PIN must be exactly 6 digits.\n";
                continue;
            }
            bool allDigits = true;
            for (char c : input)
            {
                if (!isdigit(c))
                {
                    allDigits = false; break;
                }
            }
            if (allDigits) return input;
            cout << "PIN must contain digits only.\n";
        }
    }

    // Returns 6-digit OTP, or "Z" to reselect bank
    string readTacOtp(const string& prompt)
    {
        while (true)
        {
            cout << prompt;
            string input;
            getline(cin, input);
            if (input == "Z" || input == "z") return "Z";
            if (input.length() != 6)
            {
                cout << "TAC/OTP must be exactly "
                     << "6 digits.\n";
                continue;
            }
            bool allDigits = true;
            for (char c : input)
            {
                if (!isdigit(c))
                {
                    allDigits = false; break;
                }
            }
            if (allDigits) return input;
            cout << "TAC/OTP must contain digits "
                 << "only.\n";
        }
    }

    const string CUSTOMER_ID = "CUST";

    string generateAccessCode()
    {
        static bool seeded = false;
        if (!seeded)
        {
            srand((unsigned)time(nullptr));
            seeded = true;
        }
        const string chars =
            "ABCDEFGHJKLMNPQRSTUVWXYZ"
            "23456789";
        string code;
        for (int i = 0; i < 6; i++)
        {
            code += chars[rand() % chars.length()];
        }
        return code;
    }

    void createBookingRecords(
        const vector<SelectedRoom>& selections,
        const Date& checkIn, const Date& checkOut,
        const vector<Room>& rooms,
        const vector<Booking>& existingBookings)
    {
        vector<Booking> saved = loadSavedBookings();

        int maxNum = 0;
        auto scanId = [&](const vector<Booking>& src)
        {
            for (const Booking& b : src)
            {
                if (b.bookingId.length() > 1
                    && b.bookingId[0] == 'B')
                {
                    try
                    {
                        int n = stoi(
                            b.bookingId.substr(1));
                        if (n > maxNum) maxNum = n;
                    }
                    catch (...) {}
                }
            }
        };
        scanId(existingBookings);
        scanId(saved);

        set<string> usedRooms;
        for (const Booking& b : existingBookings)
        {
            if (b.status == "CONFIRMED"
                || b.status == "PENDING"
                || b.status == "CHECKED_IN")
            {
                if (datesOverlap(checkIn, checkOut,
                    b.checkInDate, b.checkOutDate))
                {
                    usedRooms.insert(b.roomId);
                }
            }
        }
        for (const Booking& b : saved)
        {
            if (b.status == "CONFIRMED"
                || b.status == "PENDING"
                || b.status == "CHECKED_IN")
            {
                if (datesOverlap(checkIn, checkOut,
                    b.checkInDate, b.checkOutDate))
                {
                    usedRooms.insert(b.roomId);
                }
            }
        }

        for (const SelectedRoom& sel : selections)
        {
            string type =
                ROOM_TYPES[sel.typeIndex].name;
            int assigned = 0;

            for (const Room& room : rooms)
            {
                if (room.roomType != type) continue;
                if (usedRooms.count(room.roomId))
                    continue;
                if (assigned >= sel.quantity) break;

                maxNum++;
                Booking newBooking;
                newBooking.bookingId = "B"
                    + to_string(maxNum);
                newBooking.customerId = CUSTOMER_ID;
                newBooking.roomId = room.roomId;
                newBooking.bookingDate = SYSTEM_DATE;
                newBooking.checkInDate = checkIn;
                newBooking.checkOutDate = checkOut;
                newBooking.expiryDate = checkOut;
                newBooking.status = "CONFIRMED";
                newBooking.paid = true;
                newBooking.accessCode =
                    generateAccessCode();

                saveBookingToFile(newBooking);
                usedRooms.insert(room.roomId);
                assigned++;
            }
        }
    }
}

const string BOOKING_FILE = "bookingData.txt";

vector<Booking> loadSavedBookings()
{
    vector<Booking> result;
    ifstream inFile(BOOKING_FILE);
    if (!inFile) return result;

    string line;
    while (getline(inFile, line))
    {
        istringstream iss(line);
        Booking b;
        string token;
        getline(iss, b.bookingId, '\t');
        getline(iss, b.customerId, '\t');
        getline(iss, b.roomId, '\t');
        getline(iss, token, '\t');
        b.bookingDate.day = stoi(token);
        getline(iss, token, '\t');
        b.bookingDate.month = stoi(token);
        getline(iss, token, '\t');
        b.bookingDate.year = stoi(token);
        getline(iss, token, '\t');
        b.checkInDate.day = stoi(token);
        getline(iss, token, '\t');
        b.checkInDate.month = stoi(token);
        getline(iss, token, '\t');
        b.checkInDate.year = stoi(token);
        getline(iss, token, '\t');
        b.checkOutDate.day = stoi(token);
        getline(iss, token, '\t');
        b.checkOutDate.month = stoi(token);
        getline(iss, token, '\t');
        b.checkOutDate.year = stoi(token);
        getline(iss, b.status, '\t');
        getline(iss, token, '\t');
        b.paid = (token == "1");
        getline(iss, b.accessCode);
        result.push_back(b);
    }
    inFile.close();
    return result;
}

static void saveBookingToFile(const Booking& b)
{
    ofstream outFile(BOOKING_FILE, ios::app);
    outFile << b.bookingId << '\t'
            << b.customerId << '\t'
            << b.roomId << '\t'
            << b.bookingDate.day << '\t'
            << b.bookingDate.month << '\t'
            << b.bookingDate.year << '\t'
            << b.checkInDate.day << '\t'
            << b.checkInDate.month << '\t'
            << b.checkInDate.year << '\t'
            << b.checkOutDate.day << '\t'
            << b.checkOutDate.month << '\t'
            << b.checkOutDate.year << '\t'
            << b.status << '\t'
            << (b.paid ? "1" : "0") << '\t'
            << b.accessCode << '\n';
    outFile.close();
}

static void saveAllBookings(const vector<Booking>& allBookings)
{
    ofstream outFile(BOOKING_FILE);
    for (const Booking& b : allBookings)
    {
        outFile << b.bookingId << '\t'
                << b.customerId << '\t'
                << b.roomId << '\t'
                << b.bookingDate.day << '\t'
                << b.bookingDate.month << '\t'
                << b.bookingDate.year << '\t'
                << b.checkInDate.day << '\t'
                << b.checkInDate.month << '\t'
                << b.checkInDate.year << '\t'
                << b.checkOutDate.day << '\t'
                << b.checkOutDate.month << '\t'
                << b.checkOutDate.year << '\t'
                << b.status << '\t'
                << (b.paid ? "1" : "0") << '\t'
                << b.accessCode << '\n';
    }
    outFile.close();
}

void bookingScreen()
{
    vector<Room> rooms;
    vector<Booking> bookings;
    loadSchedulerDemoData(rooms, bookings);

    // Merge saved bookings for availability
    vector<Booking> saved = loadSavedBookings();
    for (const Booking& b : saved)
    {
        bookings.push_back(b);
    }

    Date checkInDate;
    Date checkOutDate;
    vector<SelectedRoom> selections;
    vector<SelectedAddOn> addonSelections;

    BookingStage stage = STAGE_CHECKIN;

    cout << "\n====================================\n";
    cout << "         ROOM BOOKING\n";
    cout << "====================================\n";
    cout << "Format: DD/MM/YYYY (e.g. 25/12/2026)\n";
    cout << "====================================\n";

    while (stage != STAGE_SUMMARY && stage != STAGE_EXIT)
    {
        // ─────────────────────────────────────────
        switch (stage)
        {
        // ── STAGE: Check-in Date ──
        case STAGE_CHECKIN:
        {
            checkInDate =
                readCurrentOrFutureDate(
                    "Check-in Date", "main menu");
            if (isGoBack(checkInDate))
            {
                stage = STAGE_EXIT;  // Z → main menu
            }
            else
            {
                stage = STAGE_CHECKOUT;
            }
            break;
        }

        // ── STAGE: Check-out Date ──
        case STAGE_CHECKOUT:
        {
            checkOutDate =
                readCurrentOrFutureDate(
                    "Check-out Date",
                    "re-enter check-in date");
            if (isGoBack(checkOutDate))
            {
                stage = STAGE_CHECKIN;  // Z → back to check-in
                break;
            }
            if (compareDates(checkOutDate, checkInDate) <= 0)
            {
                cout << "Check-out must be later than check-in ("
                     << formatDate(checkInDate) << ").\n";
                break;
            }
            selections.clear();
            addonSelections.clear();
            stage = STAGE_ROOMS;
            break;
        }

        // ── STAGE: Room Selection ──
        case STAGE_ROOMS:
        {
            while (true)
            {
                cout << "\n====================================\n";
                cout << "  Stay: " << formatDate(checkInDate)
                     << " to " << formatDate(checkOutDate)
                     << "\n";
                cout << "====================================\n";
                cout << "  Select room type:\n\n";

                int avail[NUM_ROOM_TYPES];
                for (int i = 0; i < NUM_ROOM_TYPES; i++)
                {
                    avail[i] = countAvailableByType(
                        rooms, bookings,
                        ROOM_TYPES[i].name,
                        checkInDate, checkOutDate);

                    char label = 'A' + i;
                    cout << "  [" << label << "] "
                         << ROOM_TYPES[i].name << "\n";
                    cout << "      Recommended Occupancy: "
                         << ROOM_TYPES[i].capacity
                         << " Guest(s)\n";
                    cout << "      Price: RM" << fixed
                         << setprecision(2)
                         << ROOM_TYPES[i].price
                         << " / night\n";
                    cout << "      Available: " << avail[i]
                         << " room(s)\n\n";
                }

                char choice = readLetterOrBack(
                    'A', 'E', "reselect check-out date");
                if (choice == 'Z')
                {
                    stage = STAGE_CHECKOUT;
                    break;
                }

                int typeIndex = choice - 'A';

                if (avail[typeIndex] == 0)
                {
                    cout << "\nSorry, no "
                         << ROOM_TYPES[typeIndex].name
                         << " rooms available.\n";
                    continue;
                }

                // Room quantity
                int qty = readQuantityOrBack(
                    avail[typeIndex],
                    "reselect room type");
                if (qty == -1)
                {
                    continue;  // Z → re-select room type
                }

                selections.push_back({ typeIndex, qty });

                cout << "\n  >> Selected: " << qty << "x "
                     << ROOM_TYPES[typeIndex].name << "\n";

                cout << "\n  -- Your selections --\n";
                double total = 0;
                for (const SelectedRoom& sel : selections)
                {
                    double cost = sel.quantity
                        * ROOM_TYPES[sel.typeIndex].price;
                    cout << "  " << sel.quantity << "x "
                         << ROOM_TYPES[sel.typeIndex].name
                         << " (RM" << fixed << setprecision(2)
                         << cost << "/night)\n";
                    total += cost;
                }
                cout << "  ----------------------\n";
                cout << "  Total per night: RM" << fixed
                     << setprecision(2) << total << "\n";

                cout << "\nBook more rooms? (Y/N)\n"
                     << "  Enter Z to reselect room type: ";
                string more;
                getline(cin, more);
                if (more == "Z" || more == "z")
                {
                    if (!selections.empty())
                    {
                        selections.pop_back();
                    }
                    continue;
                }
                if (more != "Y" && more != "y")
                {
                    stage = STAGE_ADDONS;
                    break;
                }
            }

            if (stage == STAGE_ADDONS && selections.empty())
            {
                cout << "\nNo rooms selected."
                     << " Booking cancelled.\n";
                stage = STAGE_EXIT;
            }
            break;
        }

        // ── STAGE: Add-on Services ──
        case STAGE_ADDONS:
        {
            cout << "\n====================================\n";
            cout << "     ADD-ON SERVICES\n";
            cout << "====================================\n";
            cout << "  Enhance your stay with extras!\n";
            cout << "  (Prices are per unit per day)\n";

            cout << "\nWould you like to add extra "
                 << "services? (Y/N)\n"
                 << "  Enter Z to reselect rooms: ";
            string addOnChoice;
            getline(cin, addOnChoice);
            if (addOnChoice == "Z" || addOnChoice == "z")
            {
                stage = STAGE_ROOMS;
                break;
            }
            if (addOnChoice != "Y" && addOnChoice != "y")
            {
                stage = STAGE_SUMMARY;
                break;
            }

            while (true)
            {
                cout << "\n  -- Available Services --\n\n";
                for (int i = 0; i < NUM_ADDONS; i++)
                {
                    char label = 'A' + i;
                    cout << "  [" << label << "] "
                         << ADDONS[i].name << "\n";
                    cout << "      RM" << fixed
                         << setprecision(2)
                         << ADDONS[i].pricePerUnit << " / "
                         << ADDONS[i].unit
                         << " / day\n\n";
                }

                char choice = readLetterOrBack(
                    'A', 'D', "reselect rooms");
                if (choice == 'Z')
                {
                    stage = STAGE_ROOMS;  // Z → back to rooms
                    break;
                }

                int ai = choice - 'A';

                // Add-on quantity
                int qty = readQuantityOrBack(
                    99, "reselect add-on");
                if (qty == -1)
                {
                    continue;  // Z → re-select add-on
                }

                addonSelections.push_back({ ai, qty });

                cout << "\n  >> Selected: " << qty << "x "
                     << ADDONS[ai].name
                     << " (RM" << fixed << setprecision(2)
                     << ADDONS[ai].pricePerUnit << "/"
                     << ADDONS[ai].unit << "/day)\n";

                cout << "\n  -- Your add-ons --\n";
                double addonTotal = 0;
                for (const SelectedAddOn& sel : addonSelections)
                {
                    double cost = sel.quantity
                        * ADDONS[sel.addonIndex].pricePerUnit;
                    cout << "  " << sel.quantity << "x "
                         << ADDONS[sel.addonIndex].name
                         << " (RM" << fixed << setprecision(2)
                         << cost << "/day)\n";
                    addonTotal += cost;
                }
                cout << "  -----------------------\n";
                cout << "  Add-on total per day: RM"
                     << fixed << setprecision(2)
                     << addonTotal << "\n";

                cout << "\nAdd more services? (Y/N)\n"
                     << "  Enter Z to reselect add-on: ";
                string more;
                getline(cin, more);
                if (more == "Z" || more == "z")
                {
                    if (!addonSelections.empty())
                    {
                        addonSelections.pop_back();
                    }
                    continue;
                }
                if (more != "Y" && more != "y")
                {
                    stage = STAGE_SUMMARY;
                    break;
                }
            }
            break;
        }

        default:
            stage = STAGE_EXIT;
            break;
        }
    }

    // ── SUMMARY ──
    if (stage == STAGE_SUMMARY)
    {
        int nights = 0;
        {
            Date temp = checkInDate;
            while (compareDates(temp, checkOutDate) < 0)
            {
                nights++;
                temp = addDays(temp, 1);
            }
        }

        double roomPerNight = 0;
        int totalRooms = 0;
        for (const SelectedRoom& sel : selections)
        {
            roomPerNight += sel.quantity
                * ROOM_TYPES[sel.typeIndex].price;
            totalRooms += sel.quantity;
        }

        double addonPerDay = 0;
        for (const SelectedAddOn& sel : addonSelections)
        {
            addonPerDay += sel.quantity
                * ADDONS[sel.addonIndex].pricePerUnit;
        }

        double roomTotal = roomPerNight * nights;
        double addonTotal = addonPerDay * nights;
        double grandTotal = roomTotal + addonTotal;

        cout << "\n====================================\n";
        cout << "       BOOKING SUMMARY\n";
        cout << "====================================\n";
        cout << "  Check-in:   " << formatDate(checkInDate)
             << "\n";
        cout << "  Check-out:  " << formatDate(checkOutDate)
             << "\n";
        cout << "  Nights:     " << nights << "\n";
        cout << "  --------------------------------\n";
        cout << "  ROOMS:\n";

        for (const SelectedRoom& sel : selections)
        {
            double perNight = sel.quantity
                * ROOM_TYPES[sel.typeIndex].price;
            double lineTotal = perNight * nights;
            cout << "  " << sel.quantity << "x "
                 << ROOM_TYPES[sel.typeIndex].name
                 << "\n";
            cout << "     RM" << fixed << setprecision(2)
                 << perNight << "/night x " << nights
                 << " nights = RM" << lineTotal << "\n";
        }
        cout << "  --------------------------------\n";
        cout << "  Room total: RM" << fixed
             << setprecision(2) << roomTotal << "\n";

        if (!addonSelections.empty())
        {
            cout << "  --------------------------------\n";
            cout << "  ADD-ONS:\n";
            for (const SelectedAddOn& sel : addonSelections)
            {
                double perDay = sel.quantity
                    * ADDONS[sel.addonIndex].pricePerUnit;
                double lineTotal = perDay * nights;
                cout << "  " << sel.quantity << "x "
                     << ADDONS[sel.addonIndex].name
                     << "\n";
                cout << "     RM" << fixed << setprecision(2)
                     << perDay << "/day x " << nights
                     << " nights = RM" << lineTotal << "\n";
            }
            cout << "  --------------------------------\n";
            cout << "  Add-on total: RM" << fixed
                 << setprecision(2) << addonTotal << "\n";
        }

        cout << "  ================================\n";
        cout << "  GRAND TOTAL: RM" << fixed
             << setprecision(2) << grandTotal << "\n";
        cout << "  Total rooms: " << totalRooms << "\n";
        cout << "====================================\n";

        // ── Payment Method ──
        cout << "\n====================================\n";
        cout << "     PAYMENT METHOD\n";
        cout << "====================================\n";
        cout << "  [A] Online Banking\n";
        cout << "  [B] E-Wallet\n";

        char payMethod;
        while (true)
        {
            cout << "Select payment method (A/B): ";
            string input;
            getline(cin, input);
            if (input == "A" || input == "a")
            {
                payMethod = 'A'; break;
            }
            if (input == "B" || input == "b")
            {
                payMethod = 'B'; break;
            }
            cout << "Invalid. Please enter A or B.\n";
        }

        if (payMethod == 'B')
        {
            // ── E-Wallet Flow ──
            cout << "\n====================================\n";
            cout << "     E-WALLET PAYMENT\n";
            cout << "====================================\n";

            // Phone number
            while (true)
            {
                cout << "\nEnter phone number:\n";
                cout << "  Format: 01x-xxxxxxx "
                     << "(e.g. 012-3456789)\n";
                cout << "  or 011-xxxxxxxx "
                     << "(e.g. 011-12345678)\n";
                cout << "Phone: ";
                string phone;
                getline(cin, phone);

                string result = validatePhone(phone);
                if (result == "valid")
                {
                    cout << "  Phone number accepted: "
                         << phone << "\n";
                    break;
                }
                if (result == "format")
                {
                    cout << "  Invalid format. "
                         << "Use 01x-xxxxxxx or "
                         << "011-xxxxxxxx.\n";
                }
                else
                {
                    cout << "  Invalid phone number.\n";
                }
            }

            // Wallet PIN
            readSixDigitPin(
                "Enter 6-digit Wallet PIN: ");

            // Confirmation
            cout << "\n====================================\n";
            cout << "  Confirm payment of RM"
                 << fixed << setprecision(2)
                 << grandTotal << "? (Y/N): ";
            string confirm;
            getline(cin, confirm);

            if (confirm == "Y" || confirm == "y")
            {
                createBookingRecords(
                    selections,
                    checkInDate, checkOutDate,
                    rooms, bookings);
                cout << "\n  *** BOOKING SUCCESSFUL ***\n";
                cout << "  Payment via E-Wallet "
                     << "completed.\n";
                cout << "  Return to main menu to view\n";
                cout << "  your booking details.\n";
                cout << "====================================\n";
            }
            else
            {
                cout << "\n  Payment cancelled.\n";
                cout << "  Your booking was NOT "
                     << "confirmed.\n";
                cout << "====================================\n";
            }
        }
        else
        {
            // ── Online Banking Flow ──
            cout << "\n====================================\n";
            cout << "     ONLINE BANKING\n";
            cout << "====================================\n";

            const string BANKS[] = {
                "Maybank",
                "Hong Leong Bank",
                "Public Bank",
                "RHB Bank"
            };

            int bankIdx;
            while (true)
            {
                cout << "  Select your bank:\n\n";
                for (int i = 0; i < 4; i++)
                {
                    cout << "  [" << (char)('A' + i)
                         << "] " << BANKS[i] << "\n";
                }

                cout << "\nSelect bank (A-D): ";
                string bankInput;
                getline(cin, bankInput);
                if (bankInput.length() == 1
                    && bankInput[0] >= 'A'
                    && bankInput[0] <= 'D')
                {
                    bankIdx = bankInput[0] - 'A';
                    cout << "\n  Bank: "
                         << BANKS[bankIdx] << "\n";
                    break;
                }
                cout << "Invalid. Please enter A-D.\n";
            }

            // Username
            cout << "Enter your banking username: ";
            string username;
            getline(cin, username);

            // TAC/OTP (Z to reselect bank)
            string otp;
            while (true)
            {
                otp = readTacOtp(
                    "Enter 6-digit TAC/OTP "
                    "(Enter Z to reselect bank): ");
                if (otp == "Z")
                {
                    // Restart bank selection
                    while (true)
                    {
                        cout << "\n  Select your bank:\n\n";
                        for (int i = 0; i < 4; i++)
                        {
                            cout << "  ["
                                 << (char)('A' + i)
                                 << "] " << BANKS[i]
                                 << "\n";
                        }
                        cout << "\nSelect bank (A-D): ";
                        string bankInput;
                        getline(cin, bankInput);
                        if (bankInput.length() == 1
                            && bankInput[0] >= 'A'
                            && bankInput[0] <= 'D')
                        {
                            bankIdx = bankInput[0]
                                - 'A';
                            cout << "\n  Bank: "
                                 << BANKS[bankIdx]
                                 << "\n";
                            break;
                        }
                        cout << "Invalid. "
                             << "Please enter A-D.\n";
                    }

                    cout << "Enter your banking "
                         << "username: ";
                    getline(cin, username);
                    continue;
                }
                break;
            }

            // Confirmation
            cout << "\n====================================\n";
            cout << "  Confirm payment of RM"
                 << fixed << setprecision(2)
                 << grandTotal
                 << " via " << BANKS[bankIdx]
                 << "? (Y/N): ";
            string confirm;
            getline(cin, confirm);

            if (confirm == "Y" || confirm == "y")
            {
                createBookingRecords(
                    selections,
                    checkInDate, checkOutDate,
                    rooms, bookings);
                cout << "\n  *** BOOKING SUCCESSFUL ***\n";
                cout << "  Payment via "
                     << BANKS[bankIdx]
                     << " completed.\n";
                cout << "  Return to main menu to view\n";
                cout << "  your booking details.\n";
                cout << "====================================\n";
            }
            else
            {
                cout << "\n  Payment cancelled.\n";
                cout << "  Your booking was NOT "
                     << "confirmed.\n";
                cout << "====================================\n";
            }
        }
    }
}

void viewPreviousBookings()
{
    vector<Room> rooms;
    vector<Booking> bookings;
    loadSchedulerDemoData(rooms, bookings);
    vector<Booking> saved = loadSavedBookings();

    vector<Booking> myBookings;
    for (const Booking& b : bookings)
    {
        if (b.customerId == CUSTOMER_ID
            && !b.accessCode.empty())
        {
            myBookings.push_back(b);
        }
    }
    for (const Booking& b : saved)
    {
        if (b.customerId == CUSTOMER_ID
            && b.status != "CANCELLED")
        {
            myBookings.push_back(b);
        }
    }

    cout << "\n====================================\n";
    cout << "   PREVIOUS BOOKING RECORDS\n";
    cout << "====================================\n";
    cout << "  (Enter Z to return to main menu)\n";

    if (myBookings.empty())
    {
        cout << "\n  No booking records found.\n";
        cout << "====================================\n";
        return;
    }

    for (size_t i = 0; i < myBookings.size(); i++)
    {
        const Booking& b = myBookings[i];

        string roomType = "Unknown";
        for (const Room& r : rooms)
        {
            if (r.roomId == b.roomId)
            {
                roomType = r.roomType;
                break;
            }
        }

        cout << "\n  [" << (i + 1) << "] Booking "
             << b.bookingId << "\n";
        cout << "      Room: " << b.roomId
             << " (" << roomType << ")\n";
        cout << "      Check-in:  "
             << formatDate(b.checkInDate) << "\n";
        cout << "      Check-out: "
             << formatDate(b.checkOutDate) << "\n";
        cout << "      Status: " << b.status << "\n";
        cout << "      Room Access Code: "
             << b.accessCode << "\n";
        cout << "      (Enter this access code to\n";
        cout << "       enter the room during your\n";
        cout << "       stay)\n";
    }

    cout << "\n====================================\n";

    cout << "\nPress any key to continue "
         << "(Z to return): ";
    string input;
    getline(cin, input);
}

void cancelBooking()
{
    vector<Room> rooms;
    vector<Booking> bookings;
    loadSchedulerDemoData(rooms, bookings);
    vector<Booking> saved = loadSavedBookings();

    vector<Booking> myBookings;
    vector<int> savedIndices;

    for (const Booking& b : bookings)
    {
        if (b.customerId == CUSTOMER_ID
            && !b.accessCode.empty()
            && b.status != "CANCELLED"
            && b.status != "COMPLETED")
        {
            myBookings.push_back(b);
            savedIndices.push_back(-1);
        }
    }
    for (size_t i = 0; i < saved.size(); i++)
    {
        if (saved[i].customerId == CUSTOMER_ID
            && saved[i].status != "CANCELLED")
        {
            myBookings.push_back(saved[i]);
            savedIndices.push_back((int)i);
        }
    }

    cout << "\n====================================\n";
    cout << "     CANCEL BOOKING\n";
    cout << "====================================\n";

    if (myBookings.empty())
    {
        cout << "\n  No active bookings to cancel.\n";
        cout << "====================================\n";
        return;
    }

    for (size_t i = 0; i < myBookings.size(); i++)
    {
        const Booking& b = myBookings[i];

        string roomType = "Unknown";
        for (const Room& r : rooms)
        {
            if (r.roomId == b.roomId)
            {
                roomType = r.roomType;
                break;
            }
        }

        cout << "  [" << (i + 1) << "] Booking "
             << b.bookingId << "\n";
        cout << "      Room: " << b.roomId
             << " (" << roomType << ")\n";
        cout << "      Check-in:  "
             << formatDate(b.checkInDate) << "\n";
        cout << "      Check-out: "
             << formatDate(b.checkOutDate) << "\n";
        cout << "      Status: " << b.status << "\n\n";
    }

    int choice = readInteger(
        "Select booking to cancel (0 to go back): ",
        0, (int)myBookings.size());

    if (choice == 0) return;

    const Booking& selected = myBookings[choice - 1];

    cout << "\n  Cancel booking "
         << selected.bookingId
         << " (Room " << selected.roomId << ")?\n";
    cout << "  Confirm (Y/N): ";
    string confirm;
    getline(cin, confirm);

    if (confirm == "Y" || confirm == "y")
    {
        int savedIdx = savedIndices[choice - 1];
        if (savedIdx >= 0)
        {
            saved[savedIdx].status = "CANCELLED";
            saveAllBookings(saved);
        }
        else
        {
            vector<Booking> allSaved =
                loadSavedBookings();
            Booking cancelled = selected;
            cancelled.status = "CANCELLED";
            allSaved.push_back(cancelled);
            saveAllBookings(allSaved);
        }

        cout << "\n  *** CANCELLATION SUCCESSFUL ***\n";
        cout << "  Booking " << selected.bookingId
             << " has been cancelled.\n";
        cout << "  Room " << selected.roomId
             << " is now available.\n";
        cout << "====================================\n";
    }
    else
    {
        cout << "\n  Cancellation not confirmed.\n";
        cout << "  Returning to main menu.\n";
        cout << "====================================\n";
    }
}
