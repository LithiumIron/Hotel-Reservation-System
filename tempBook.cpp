#include "booking.h"
#include "scheduler.h"
#include "utilities.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
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
}

void bookingScreen()
{
    vector<Room> rooms;
    vector<Booking> bookings;
    loadSchedulerDemoData(rooms, bookings);

    Date checkInDate;
    Date checkOutDate;
    vector<SelectedRoom> selections;
    vector<SelectedAddOn> addonSelections;

    BookingStage stage = STAGE_CHECKIN;

    cout << "\n====================================\n";
    cout << "         ROOM BOOKING\n";
    cout << "====================================\n";
    cout << "Format: DD/MM/YYYY (e.g. 25/12/2026)\n";
    cout << "Enter Z at any prompt to go back.\n";
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

        // Payment method
        cout << "\n====================================\n";
        cout << "     PAYMENT METHOD\n";
        cout << "====================================\n";
        cout << "  [A] Online Banking\n";
        cout << "  [B] E-Wallet\n";

        while (true)
        {
            cout << "Select payment method (A/B): ";
            string input;
            getline(cin, input);
            if (input == "A" || input == "a")
            {
                cout << "\n  Payment method: Online Banking\n";
                break;
            }
            if (input == "B" || input == "b")
            {
                cout << "\n  Payment method: E-Wallet\n";
                break;
            }
            cout << "Invalid. Please enter A or B.\n";
        }
    }
}
