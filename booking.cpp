#include "booking.h"
#include "scheduler.h"
#include "utilities.h"
#include "vip.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <limits>
#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// Forward declarations for file I/O functions
static void saveBookingToFile(const Booking& b);
void saveAllBookings(const vector<Booking>& allBookings);

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
        const string& backLabel,
        const Date* defaultValue = nullptr)
    {
        while (true)
        {
            cout << '\n' << heading
                << "  (Enter 0 to "
                << backLabel << ")\n";
            if (defaultValue != nullptr)
            {
                cout << "Press Enter to keep "
                    << formatDate(*defaultValue) << "\n";
            }
            cout << "Enter date (DD/MM/YYYY): ";
            string input;
            getline(cin, input);

            if (isGoBackInput(input))
            {
                return GO_BACK;
            }

            // Keep the existing date when the user just presses Enter
            if (input.empty() && defaultValue != nullptr)
            {
                return *defaultValue;
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
        const string& backLabel,
        const Date* defaultValue = nullptr)
    {
        while (true)
        {
            Date date = readDate(heading, backLabel, defaultValue);
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
            cout << "Quantity (Enter 0 to "
                << backLabel << "): ";
            string input;
            getline(cin, input);
            if (isGoBackInput(input))
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
            cout << "Invalid quantity.\n";
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
                << ", Enter 0 to "
                << backLabel << "): ";
            string input;
            getline(cin, input);
            if (isGoBackInput(input))
            {
                return 'Z';
            }
            if (input.length() == 1)
            {
                char ch = toupper(input[0]);
                if (ch >= minCh && ch <= maxCh)
                {
                    return ch;
                }
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
        { "Gym & Pool VIP Pass", "person", 20.0 },
        { "High-Speed WiFi", "room", 40.0 }
    };
    const int NUM_ADDONS = 3;

    struct SelectedAddOn
    {
        int addonIndex;
        int quantity;
    };

    // Returns: "valid", "format", or "invalid"
    string validatePhone(const string& input)
    {
        // Regex patterns for Malaysian phone numbers
        regex pattern011("^011-\\d{8}$");
        regex patternOthers("^(010|012|013|014|016|017|018|019)-\\d{7}$");

        if (regex_match(input, pattern011) || regex_match(input, patternOthers))
        {
            return "valid";
        }

        // Check if it's format error (wrong length/structure) or invalid prefix
        if (input.length() < 5 || input[3] != '-')
        {
            return "format";
        }

        string prefix = input.substr(0, 3);
        if (prefix == "011")
        {
            return "format"; // Wrong length for 011
        }
        else
        {
            return "invalid"; // Invalid prefix
        }
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

    // Returns 6-digit OTP, or "0" to reselect bank
    string readTacOtp(const string& prompt)
    {
        while (true)
        {
            cout << prompt;
            string input;
            getline(cin, input);
            if (isGoBackInput(input)) return "0";
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

    string generateAccessCode()
    {
        static bool seeded = false;
        if (!seeded)
        {
            srand((unsigned)time(nullptr));
            seeded = true;
        }
        string code;
        for (int i = 0; i < 6; i++)
        {
            code += (char)('0' + rand() % 10);
        }
        return code;
    }

    void createBookingRecords(
        const vector<SelectedRoom>& selections,
        const Date& checkIn, const Date& checkOut,
        const vector<Room>& rooms,
        const vector<Booking>& existingBookings,
        const string& customerId,
        const string& addonsString)
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

        // One booking ID for the entire session
        maxNum++;
        string bookingId = "B" + to_string(maxNum);

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

                Booking newBooking;
                newBooking.bookingId = bookingId;
                newBooking.customerId = customerId;
                newBooking.roomId = room.roomId;
                newBooking.bookingDate = SYSTEM_DATE;
                newBooking.checkInDate = checkIn;
                newBooking.checkOutDate = checkOut;
                newBooking.expiryDate = checkOut;
                newBooking.status = "CONFIRMED";
                newBooking.paid = true;
                newBooking.accessCode =
                    generateAccessCode();
                newBooking.addons = addonsString;

                saveBookingToFile(newBooking);
                usedRooms.insert(room.roomId);
                assigned++;
            }
        }
    }

    void saveReceiptToFile(
        const string& customerId,
        const Date& checkInDate, const Date& checkOutDate,
        const string& paymentMethod,
        double subtotalBeforeDiscount, double vipDiscountAmount,
        double grandTotal,
        double amountPaid, double change, bool showAmountPaid,
        const vector<SelectedRoom>& selections,
        const vector<SelectedAddOn>& addonSelections,
        int nights, double roomTotal, double addonTotal)
    {
        cout << "\nWould you like to save a copy of this receipt to a file? (Y/N): ";
        string saveChoice;
        getline(cin, saveChoice);

        if (saveChoice != "Y" && saveChoice != "y")
        {
            return;
        }

        string filename = "Receipt_" + customerId + "_"
            + to_string(static_cast<long>(time(nullptr))) + ".txt";

        ofstream outFile(filename);

        if (!outFile)
        {
            cout << "\nError: Could not create the receipt file. "
                << "Please check folder permissions.\n";
            return;
        }

        outFile << "====================================\n";
        outFile << "           RECEIPT\n";
        outFile << "====================================\n";
        outFile << "  Guest: " << customerId << "\n";
        outFile << "  Check-in:  " << formatDate(checkInDate) << "\n";
        outFile << "  Check-out: " << formatDate(checkOutDate) << "\n";
        outFile << "  Payment: " << paymentMethod << "\n";
        if (vipDiscountAmount > 0.0)
        {
            outFile << "  Subtotal: RM" << fixed << setprecision(2) << subtotalBeforeDiscount << "\n";
            outFile << "  VIP Discount: - RM" << fixed << setprecision(2) << vipDiscountAmount << "\n";
        }
        outFile << "  Amount: RM" << fixed << setprecision(2) << grandTotal << "\n";

        if (showAmountPaid)
        {
            outFile << "  Amount Received: RM" << fixed << setprecision(2) << amountPaid << "\n";
            outFile << "  Change Due: RM" << fixed << setprecision(2) << change << "\n";
        }

        outFile << "  --------------------------------\n";
        outFile << "  ROOMS:\n";
        for (const SelectedRoom& sel : selections)
        {
            double perNight = sel.quantity * ROOM_TYPES[sel.typeIndex].price;
            double lineTotal = perNight * nights;
            outFile << "  " << sel.quantity << "x " << ROOM_TYPES[sel.typeIndex].name << "\n";
            outFile << "     RM" << fixed << setprecision(2) << lineTotal << "\n";
        }
        outFile << "  --------------------------------\n";
        outFile << "  Room total: RM" << fixed << setprecision(2) << roomTotal << "\n";

        if (!addonSelections.empty())
        {
            outFile << "  --------------------------------\n";
            outFile << "  ADD-ONS:\n";
            for (const SelectedAddOn& sel : addonSelections)
            {
                double perDay = sel.quantity * ADDONS[sel.addonIndex].pricePerUnit;
                double lineTotal = perDay * nights;
                outFile << "  " << sel.quantity << "x " << ADDONS[sel.addonIndex].name << "\n";
                outFile << "     RM" << fixed << setprecision(2) << lineTotal << "\n";
            }
            outFile << "  --------------------------------\n";
            outFile << "  Add-on total: RM" << fixed << setprecision(2) << addonTotal << "\n";
        }

        outFile << "  --------------------------------\n";
        outFile << "  Status: CONFIRMED\n";
        outFile << "====================================\n";

        outFile.close();

        cout << "\nReceipt saved successfully as \"" << filename << "\".\n";
    }

    // =========================================================================
    // PAYMENT METHOD FUNCTIONS 
    // =========================================================================

    bool processOnlineBankingPayment(
        const string& customerId, const Date& checkInDate, const Date& checkOutDate,
        const vector<SelectedRoom>& selections, const vector<SelectedAddOn>& addonSelections,
        const vector<Room>& rooms, const vector<Booking>& bookings, const string& addonsString,
        int nights, double roomTotal, double addonTotal, double subtotalBeforeDiscount,
        double vipDiscountAmount, double serviceAmount, double taxAmount, double grandTotal)
    {
        cout << "\n====================================\n";
        cout << "     ONLINE BANKING\n";
        cout << "====================================\n";

        const string BANKS[] = {
            "Maybank",
            "Hong Leong Bank",
            "Public Bank",
            "RHB Bank"
        };

        int bankIdx = 0;
        bool bankSelected = false;

        while (!bankSelected)
        {
            cout << "  Select your bank (Enter 0 to reselect payment method):\n\n";
            for (int i = 0; i < 4; i++)
            {
                cout << "  [" << (char)('A' + i) << "] " << BANKS[i] << "\n";
            }

            cout << "\nSelect bank (A-D/0): ";
            string bankInput;
            getline(cin, bankInput);

            if (isGoBackInput(bankInput))
            {
                return false; // Back to payment menu
            }

            if (bankInput.length() == 1)
            {
                char c = toupper(bankInput[0]);
                if (c >= 'A' && c <= 'D')
                {
                    bankIdx = c - 'A';
                    cout << "\n  Bank: " << BANKS[bankIdx] << "\n";
                    bankSelected = true;
                }
                else
                {
                    cout << "Invalid. Please enter A-D.\n";
                }
            }
            else
            {
                cout << "Invalid. Please enter A-D.\n";
            }
        }

        cout << "Enter your banking username: ";
        string username;
        getline(cin, username);

        string otp;
        while (true)
        {
            otp = readTacOtp("Enter 6-digit TAC/OTP (Enter 0 to reselect bank): ");
            if (otp == "0")
            {
                while (true)
                {
                    cout << "\n  Select your bank:\n\n";
                    for (int i = 0; i < 4; i++)
                    {
                        cout << "  [" << (char)('A' + i) << "] " << BANKS[i] << "\n";
                    }
                    cout << "\nSelect bank (A-D): ";
                    string bankInput;
                    getline(cin, bankInput);
                    if (bankInput.length() == 1)
                    {
                        char c = toupper(bankInput[0]);
                        if (c >= 'A' && c <= 'D')
                        {
                            bankIdx = c - 'A';
                            cout << "\n  Bank: " << BANKS[bankIdx] << "\n";
                            break;
                        }
                    }
                    cout << "Invalid. Please enter A-D.\n";
                }

                cout << "Enter your banking username: ";
                getline(cin, username);
                continue;
            }
            break;
        }

        cout << "\n====================================\n";
        cout << "  Confirm payment of RM" << fixed << setprecision(2)
            << grandTotal << " via " << BANKS[bankIdx] << "? (Y/N): ";
        string confirm;
        getline(cin, confirm);

        if (confirm == "Y" || confirm == "y")
        {
            createBookingRecords(selections, checkInDate, checkOutDate, rooms, bookings, customerId, addonsString);
            cout << "\n====================================\n";
            cout << "           RECEIPT\n";
            cout << "====================================\n";
            cout << "  Guest: " << customerId << "\n";
            cout << "  Check-in:  " << formatDate(checkInDate) << "\n";
            cout << "  Check-out: " << formatDate(checkOutDate) << "\n";
            cout << "  Payment: " << BANKS[bankIdx] << "\n";
            cout << "  --------------------------------\n";
            cout << "  ROOMS:\n";
            for (const SelectedRoom& sel : selections)
            {
                double perNight = sel.quantity * ROOM_TYPES[sel.typeIndex].price;
                double lineTotal = perNight * nights;
                cout << "  " << sel.quantity << "x " << ROOM_TYPES[sel.typeIndex].name << "\n";
                cout << "     RM" << lineTotal << "\n";
            }
            cout << "  --------------------------------\n";
            cout << "  Room total: RM" << fixed << setprecision(2) << roomTotal << "\n";

            if (!addonSelections.empty())
            {
                cout << "  --------------------------------\n";
                cout << "  ADD-ONS:\n";
                for (const SelectedAddOn& sel : addonSelections)
                {
                    double perDay = sel.quantity * ADDONS[sel.addonIndex].pricePerUnit;
                    double lineTotal = perDay * nights;
                    cout << "  " << sel.quantity << "x " << ADDONS[sel.addonIndex].name << "\n";
                    cout << "     RM" << lineTotal << "\n";
                }
                cout << "  --------------------------------\n";
                cout << "  Add-on total: RM" << fixed << setprecision(2) << addonTotal << "\n";
            }

            if (vipDiscountAmount > 0.0)
            {
                cout << "  --------------------------------\n";
                cout << "  Subtotal: RM" << fixed << setprecision(2) << subtotalBeforeDiscount << "\n";
                cout << "  VIP Discount: - RM" << fixed << setprecision(2) << vipDiscountAmount << "\n";
            }

            cout << "  --------------------------------\n";
            cout << "  Service Charge (8%) : RM" << fixed << setprecision(2) << serviceAmount << "\n";
            cout << "  Government Tax (6%) : RM" << fixed << setprecision(2) << taxAmount << "\n";
            cout << "  Amount: RM" << fixed << setprecision(2) << grandTotal << "\n";
            cout << "  --------------------------------\n";
            cout << "  Status: CONFIRMED\n";
            cout << "====================================\n";
            saveReceiptToFile(customerId, checkInDate, checkOutDate,
                BANKS[bankIdx], subtotalBeforeDiscount, vipDiscountAmount,
                grandTotal, 0.0, 0.0, false,
                selections, addonSelections, nights, roomTotal, addonTotal);

            cout << "\n  *** BOOKING SUCCESSFUL ***\n";
            cout << "  Press Enter to back to main menu: ";
            string dummy;
            getline(cin, dummy);
            return true;
        }

        cout << "\n  Payment cancelled.\n";
        cout << "  Your booking was NOT confirmed.\n";
        cout << "====================================\n";
        return false;
    }

    bool processEWalletPayment(
        const string& customerId, const Date& checkInDate, const Date& checkOutDate,
        const vector<SelectedRoom>& selections, const vector<SelectedAddOn>& addonSelections,
        const vector<Room>& rooms, const vector<Booking>& bookings, const string& addonsString,
        int nights, double roomTotal, double addonTotal, double subtotalBeforeDiscount,
        double vipDiscountAmount, double serviceAmount, double taxAmount, double grandTotal)
    {
        cout << "\n====================================\n";
        cout << "     E-WALLET PAYMENT\n";
        cout << "====================================\n";

        while (true)
        {
            cout << "\nEnter phone number (Enter 0 to reselect payment method)\n";
            cout << "  (e.g. 012-3456789): ";
            string phone;
            getline(cin, phone);

            if (isGoBackInput(phone))
            {
                return false; // Back to payment menu
            }

            string result = validatePhone(phone);
            if (result == "valid") break;
            if (result == "format")
            {
                cout << "  Invalid format. Use 01x-xxxxxxx or 011-xxxxxxxx.\n";
            }
            else
            {
                cout << "  Invalid phone number.\n";
            }
        }

        readSixDigitPin("Enter 6-digit Wallet PIN: ");

        cout << "\n====================================\n";
        cout << "  Confirm payment of RM" << fixed << setprecision(2) << grandTotal << "? (Y/N): ";
        string confirm;
        getline(cin, confirm);

        if (confirm == "Y" || confirm == "y")
        {
            createBookingRecords(selections, checkInDate, checkOutDate, rooms, bookings, customerId, addonsString);
            cout << "\n====================================\n";
            cout << "           RECEIPT\n";
            cout << "====================================\n";
            cout << "  Guest: " << customerId << "\n";
            cout << "  Check-in:  " << formatDate(checkInDate) << "\n";
            cout << "  Check-out: " << formatDate(checkOutDate) << "\n";
            cout << "  Payment: E-Wallet\n";
            cout << "  --------------------------------\n";
            cout << "  ROOMS:\n";
            for (const SelectedRoom& sel : selections)
            {
                double perNight = sel.quantity * ROOM_TYPES[sel.typeIndex].price;
                double lineTotal = perNight * nights;
                cout << "  " << sel.quantity << "x " << ROOM_TYPES[sel.typeIndex].name << "\n";
                cout << "     RM" << lineTotal << "\n";
            }
            cout << "  --------------------------------\n";
            cout << "  Room total: RM" << fixed << setprecision(2) << roomTotal << "\n";

            if (!addonSelections.empty())
            {
                cout << "  --------------------------------\n";
                cout << "  ADD-ONS:\n";
                for (const SelectedAddOn& sel : addonSelections)
                {
                    double perDay = sel.quantity * ADDONS[sel.addonIndex].pricePerUnit;
                    double lineTotal = perDay * nights;
                    cout << "  " << sel.quantity << "x " << ADDONS[sel.addonIndex].name << "\n";
                    cout << "     RM" << lineTotal << "\n";
                }
                cout << "  --------------------------------\n";
                cout << "  Add-on total: RM" << fixed << setprecision(2) << addonTotal << "\n";
            }

            if (vipDiscountAmount > 0.0)
            {
                cout << "  --------------------------------\n";
                cout << "  Subtotal: RM" << fixed << setprecision(2) << subtotalBeforeDiscount << "\n";
                cout << "  VIP Discount: - RM" << fixed << setprecision(2) << vipDiscountAmount << "\n";
            }

            cout << "  --------------------------------\n";
            cout << "  Service Charge (8%) : RM" << fixed << setprecision(2) << serviceAmount << "\n";
            cout << "  Government Tax (6%) : RM" << fixed << setprecision(2) << taxAmount << "\n";
            cout << "  Amount: RM" << fixed << setprecision(2) << grandTotal << "\n";
            cout << "  --------------------------------\n";
            cout << "  Status: CONFIRMED\n";
            cout << "====================================\n";

            saveReceiptToFile(customerId, checkInDate, checkOutDate,
                "E-Wallet", subtotalBeforeDiscount, vipDiscountAmount,
                grandTotal, 0.0, 0.0, false,
                selections, addonSelections, nights, roomTotal, addonTotal);

            cout << "\n  *** BOOKING SUCCESSFUL ***\n";
            cout << "  Press Enter to back to main menu: ";
            string dummy;
            getline(cin, dummy);
            return true;
        }

        cout << "\n  Payment cancelled.\n";
        cout << "  Your booking was NOT confirmed.\n";
        cout << "====================================\n";
        return false;
    }

    bool processCashPayment(
        const string& customerId, const Date& checkInDate, const Date& checkOutDate,
        const vector<SelectedRoom>& selections, const vector<Room>& rooms,
        const vector<Booking>& bookings, const string& addonsString)
    {
        cout << "\n====================================\n";
        cout << "        CASH PAYMENT\n";
        cout << "====================================\n";
        cout << "  Please pay at the counter. Thanks.\n";
        cout << "====================================\n";

        createBookingRecords(selections, checkInDate, checkOutDate, rooms, bookings, customerId, addonsString);

        cout << "  Press Enter to back to main menu: ";
        string dummy;
        getline(cin, dummy);
        return true;
    }

    bool processCardPayment(
        const string& customerId, const Date& checkInDate, const Date& checkOutDate,
        const vector<SelectedRoom>& selections, const vector<SelectedAddOn>& addonSelections,
        const vector<Room>& rooms, const vector<Booking>& bookings, const string& addonsString,
        int nights, double roomTotal, double addonTotal, double subtotalBeforeDiscount,
        double vipDiscountAmount, double serviceAmount, double taxAmount, double grandTotal)
    {
        cout << "\n====================================\n";
        cout << "     CREDIT/DEBIT CARD PAYMENT\n";
        cout << "====================================\n";

        string cardNumber, expiry, cvv, pin;
        regex cardPattern("[0-9]{4}-[0-9]{4}-[0-9]{4}-[0-9]{4}");
        regex pinPattern("[0-9]{6}");
        bool validCardPayment = false;

        while (!validCardPayment)
        {
            cout << "Enter Card Number (XXXX-XXXX-XXXX-XXXX): ";
            getline(cin, cardNumber);

            if (!regex_match(cardNumber, cardPattern))
            {
                cout << "Invalid card number format.\n";
                cout << "Please use XXXX-XXXX-XXXX-XXXX.\n";
                continue;
            }

            cout << "Expiry (MM/YY): ";
            getline(cin, expiry);

            if (expiry.length() != 5 || expiry[2] != '/'
                || !isdigit((unsigned char)expiry[0])
                || !isdigit((unsigned char)expiry[1])
                || !isdigit((unsigned char)expiry[3])
                || !isdigit((unsigned char)expiry[4]))
            {
                cout << "Invalid expiry format. Please use MM/YY.\n";
                continue;
            }

            int month = stoi(expiry.substr(0, 2));
            if (month < 1 || month > 12)
            {
                cout << "Invalid expiry month.\n";
                continue;
            }

            int year = stoi(expiry.substr(3, 2));
            if (year <= 25)
            {
                cout << "Card is expired.\n";
                continue;
            }

            cout << "CVV: ";
            getline(cin, cvv);

            bool validCVV = (cvv.length() == 3);
            for (char c : cvv)
            {
                if (!isdigit((unsigned char)c)) validCVV = false;
            }

            if (!validCVV)
            {
                cout << "Invalid CVV. CVV must contain 3 digits.\n";
                continue;
            }

            cout << "Enter card PIN: ";
            cin >> pin;

            if (!regex_match(pin, pinPattern) || pin.length() != 6)
            {
                cout << "Pin number should only 6-digits.\n";
                continue;
            }
            else
            {
                cout << "Processing card payment... \n";
            }
            validCardPayment = true;
        }

        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // flush leftover '\n'

        cout << "\n====================================\n";
        cout << "  Confirm payment of RM" << fixed << setprecision(2) << grandTotal << "? (Y/N): ";
        string confirm;
        getline(cin, confirm);

        if (confirm == "Y" || confirm == "y")
        {
            createBookingRecords(selections, checkInDate, checkOutDate, rooms, bookings, customerId, addonsString);
            cout << "\n====================================\n";
            cout << "           RECEIPT\n";
            cout << "====================================\n";
            cout << "  Guest: " << customerId << "\n";
            cout << "  Check-in:  " << formatDate(checkInDate) << "\n";
            cout << "  Check-out: " << formatDate(checkOutDate) << "\n";
            cout << "  Payment: Credit / Debit Card\n";
            cout << "  --------------------------------\n";
            cout << "  ROOMS:\n";
            for (const SelectedRoom& sel : selections)
            {
                double perNight = sel.quantity * ROOM_TYPES[sel.typeIndex].price;
                double lineTotal = perNight * nights;
                cout << "  " << sel.quantity << "x " << ROOM_TYPES[sel.typeIndex].name << "\n";
                cout << "     RM" << lineTotal << "\n";
            }
            cout << "  --------------------------------\n";
            cout << "  Room total: RM" << fixed << setprecision(2) << roomTotal << "\n";

            if (!addonSelections.empty())
            {
                cout << "  --------------------------------\n";
                cout << "  ADD-ONS:\n";
                for (const SelectedAddOn& sel : addonSelections)
                {
                    double perDay = sel.quantity * ADDONS[sel.addonIndex].pricePerUnit;
                    double lineTotal = perDay * nights;
                    cout << "  " << sel.quantity << "x " << ADDONS[sel.addonIndex].name << "\n";
                    cout << "     RM" << lineTotal << "\n";
                }
                cout << "  --------------------------------\n";
                cout << "  Add-on total: RM" << fixed << setprecision(2) << addonTotal << "\n";
            }

            if (vipDiscountAmount > 0.0)
            {
                cout << "  --------------------------------\n";
                cout << "  Subtotal: RM" << fixed << setprecision(2) << subtotalBeforeDiscount << "\n";
                cout << "  VIP Discount: - RM" << fixed << setprecision(2) << vipDiscountAmount << "\n";
            }

            cout << "  --------------------------------\n";
            cout << "  Service Charge (8%) : RM" << fixed << setprecision(2) << serviceAmount << "\n";
            cout << "  Government Tax (6%) : RM" << fixed << setprecision(2) << taxAmount << "\n";
            cout << "  Amount: RM" << fixed << setprecision(2) << grandTotal << "\n";
            cout << "  --------------------------------\n";
            cout << "  Status: CONFIRMED\n";
            cout << "====================================\n";

            saveReceiptToFile(customerId, checkInDate, checkOutDate,
                "Credit/Debit Card", subtotalBeforeDiscount, vipDiscountAmount,
                grandTotal, 0.0, 0.0, false,
                selections, addonSelections, nights, roomTotal, addonTotal);

            cout << "\n  *** BOOKING SUCCESSFUL ***\n";
            cout << "  Press Enter to back to main menu: ";
            string dummy;
            getline(cin, dummy);
            return true;
        }

        cout << "\n  Payment cancelled.\n";
        cout << "  Your booking was NOT confirmed.\n";
        cout << "====================================\n";
        return false;
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
        getline(iss, b.accessCode, '\t');
        getline(iss, b.addons);
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
        << b.accessCode << '\t'
        << b.addons << '\n';
    outFile.close();
}

void saveAllBookings(const vector<Booking>& allBookings)
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
            << b.accessCode << '\t'
            << b.addons << '\n';
    }
    outFile.close();
}


void bookingScreen(const string& customerId)
{
    clearScreen();
    vector<Room> rooms;
    vector<Booking> bookings;
    loadSchedulerDemoData(rooms, bookings);

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
    bool checkInSet = false;
    int reselectType = -1;

    int nights = 0;
    double roomTotal = 0, addonTotal = 0;
    double subtotalBeforeDiscount = 0, vipDiscountAmount = 0;
    double grandTotal = 0;
    double serviceAmount = 0, taxAmount = 0;

    while (stage != STAGE_SUMMARY && stage != STAGE_EXIT)
    {
        switch (stage)
        {
        case STAGE_CHECKIN:
        {
            clearScreen();
            cout << "\n====================================\n";
            cout << "         ROOM BOOKING\n";
            cout << "====================================\n";
            cout << "  Welcome, " << customerId << "!\n";
            cout << "====================================\n";
            cout << "Format: DD/MM/YYYY (e.g. 25/12/2026)\n";
            cout << "====================================\n";

            checkInDate = readCurrentOrFutureDate("Check-in Date", "main menu", checkInSet ? &checkInDate : nullptr);
            if (isGoBack(checkInDate))
            {
                stage = STAGE_EXIT;
            }
            else
            {
                checkInSet = true;
                stage = STAGE_CHECKOUT;
            }
            break;
        }

        case STAGE_CHECKOUT:
        {
            checkOutDate = readCurrentOrFutureDate("Check-out Date", "re-enter check-in date");
            if (isGoBack(checkOutDate))
            {
                stage = STAGE_CHECKIN;
                break;
            }
            if (compareDates(checkOutDate, checkInDate) <= 0)
            {
                cout << "Check-out must be later than check-in (" << formatDate(checkInDate) << ").\n";
                break;
            }
            selections.clear();
            addonSelections.clear();
            stage = STAGE_ROOMS;
            break;
        }

        case STAGE_ROOMS:
        {
            while (true)
            {
                clearScreen();
                bookings.erase(
                    remove_if(bookings.begin(), bookings.end(),
                        [](const Booking& b) { return b.bookingId == "HOLD"; }),
                    bookings.end());
                for (const SelectedRoom& sel : selections)
                {
                    int held = 0;
                    for (const Room& room : rooms)
                    {
                        if (room.roomType != ROOM_TYPES[sel.typeIndex].name) continue;
                        if (!isRoomAvailable(bookings, room.roomId, checkInDate, checkOutDate)) continue;

                        Booking hold;
                        hold.bookingId = "HOLD";
                        hold.customerId = customerId;
                        hold.roomId = room.roomId;
                        hold.bookingDate = SYSTEM_DATE;
                        hold.checkInDate = checkInDate;
                        hold.checkOutDate = checkOutDate;
                        hold.expiryDate = checkOutDate;
                        hold.status = "CONFIRMED";
                        hold.paid = false;
                        bookings.push_back(hold);
                        held++;
                        if (held >= sel.quantity) break;
                    }
                }

                cout << "\n====================================\n";
                cout << "  Stay: " << formatDate(checkInDate) << " to " << formatDate(checkOutDate) << "\n";
                cout << "====================================\n";

                if (!selections.empty())
                {
                    cout << "\n  --- Previous Selection ---\n";
                    double prevTotal = 0;
                    for (const SelectedRoom& sel : selections)
                    {
                        double cost = sel.quantity * ROOM_TYPES[sel.typeIndex].price;
                        cout << "  " << sel.quantity << "x " << ROOM_TYPES[sel.typeIndex].name
                            << " (RM" << fixed << setprecision(2) << cost << "/night)\n";
                        prevTotal += cost;
                    }
                    cout << "  ----------------------\n";
                    cout << "  Total per night: RM" << fixed << setprecision(2) << prevTotal << "\n";
                }

                cout << "\n  Select room type:\n\n";

                int avail[NUM_ROOM_TYPES];
                for (int i = 0; i < NUM_ROOM_TYPES; i++)
                {
                    avail[i] = countAvailableByType(rooms, bookings, ROOM_TYPES[i].name, checkInDate, checkOutDate);
                    char label = 'A' + i;
                    cout << "  [" << label << "] " << ROOM_TYPES[i].name << "\n";
                    cout << "      Recommended Occupancy: " << ROOM_TYPES[i].capacity << " Guest(s)\n";
                    cout << "      Price: RM" << fixed << setprecision(2) << ROOM_TYPES[i].price << " / night\n";
                    cout << "      Available: " << avail[i] << " room(s)\n\n";
                }

                int typeIndex = 0;
                if (reselectType >= 0)
                {
                    typeIndex = reselectType;
                    reselectType = -1;
                    cout << "  >> Reselect quantity for: " << ROOM_TYPES[typeIndex].name << "\n\n";
                }
                else
                {
                    if (selections.empty())
                    {
                        char choice = readLetterOrBack('A', 'E', "reselect dates");
                        if (choice == 'Z')
                        {
                            clearScreen();
                            stage = STAGE_CHECKIN;
                            break;
                        }
                        typeIndex = choice - 'A';
                    }
                    else
                    {
                        char choice = readLetterOrBack('A', 'E', "reselect the quantity of the room type");
                        if (choice == 'Z')
                        {
                            reselectType = selections.back().typeIndex;
                            selections.pop_back();
                            continue;
                        }
                        typeIndex = choice - 'A';
                    }
                }

                if (avail[typeIndex] == 0)
                {
                    cout << "\nSorry, no " << ROOM_TYPES[typeIndex].name << " rooms available.\n";
                    cout << "\nPress Enter to continue...";
                    string dummy;
                    getline(cin, dummy);
                    continue;
                }

                int qty = readQuantityOrBack(avail[typeIndex], "reselect room type");
                if (qty == -1) continue;

                selections.push_back({ typeIndex, qty });

                cout << "\n  >> Selected: " << qty << "x " << ROOM_TYPES[typeIndex].name << "\n";
                cout << "\n  -- Your selections --\n";
                double total = 0;
                for (const SelectedRoom& sel : selections)
                {
                    double cost = sel.quantity * ROOM_TYPES[sel.typeIndex].price;
                    cout << "  " << sel.quantity << "x " << ROOM_TYPES[sel.typeIndex].name
                        << " (RM" << fixed << setprecision(2) << cost << "/night)\n";
                    total += cost;
                }
                cout << "  ----------------------\n";
                cout << "  Total per night: RM" << fixed << setprecision(2) << total << "\n";

                cout << "\nBook more rooms? (Y/N) (Enter 0 to reselect the quantity of the room type): ";
                string more;
                getline(cin, more);
                if (isGoBackInput(more))
                {
                    if (!selections.empty())
                    {
                        reselectType = selections.back().typeIndex;
                        selections.pop_back();
                    }
                    continue;
                }
                if (more != "Y" && more != "y")
                {
                    stage = STAGE_ADDONS;
                    break;
                }

                int totalLeft = 0;
                for (int i = 0; i < NUM_ROOM_TYPES; i++)
                {
                    totalLeft += countAvailableByType(rooms, bookings, ROOM_TYPES[i].name, checkInDate, checkOutDate);
                }
                if (totalLeft == 0)
                {
                    cout << "\n  *** All rooms have been selected. ***\n";
                    cout << "  Proceeding to add-on services...\n";
                    stage = STAGE_ADDONS;
                    break;
                }
            }

            if (stage == STAGE_ADDONS && selections.empty())
            {
                cout << "\nNo rooms selected. Booking cancelled.\n";
                stage = STAGE_EXIT;
            }
            break;
        }

        case STAGE_ADDONS:
        {
            while (true)
            {
                clearScreen();
                cout << "\n====================================\n";
                cout << "     ADD-ON SERVICES\n";
                cout << "====================================\n";
                cout << "  Enhance your stay with extras!\n";
                cout << "  (Prices are per unit per day)\n";

                cout << "\n  --- Previous Selection ---\n";
                double roomPrevTotal = 0;
                for (const SelectedRoom& sel : selections)
                {
                    double cost = sel.quantity * ROOM_TYPES[sel.typeIndex].price;
                    cout << "  " << sel.quantity << "x " << ROOM_TYPES[sel.typeIndex].name
                        << " (RM" << fixed << setprecision(2) << cost << "/night)\n";
                    roomPrevTotal += cost;
                }
                if (!addonSelections.empty())
                {
                    for (const SelectedAddOn& sel : addonSelections)
                    {
                        double cost = sel.quantity * ADDONS[sel.addonIndex].pricePerUnit;
                        cout << "  " << sel.quantity << "x " << ADDONS[sel.addonIndex].name
                            << " (RM" << fixed << setprecision(2) << cost << "/day)\n";
                    }
                }
                double addonPrevTotal = 0;
                for (const SelectedAddOn& sel : addonSelections)
                {
                    addonPrevTotal += sel.quantity * ADDONS[sel.addonIndex].pricePerUnit;
                }
                cout << "  ----------------------\n";
                cout << "  Room total/night: RM" << fixed << setprecision(2) << roomPrevTotal << "\n";
                cout << "  Add-on total/day: RM" << fixed << setprecision(2) << addonPrevTotal << "\n";
                cout << "  ----------------------\n";

                cout << "\n  -- Available Services --\n\n";
                for (int i = 0; i < NUM_ADDONS; i++)
                {
                    char label = 'A' + i;
                    cout << "  [" << label << "] " << ADDONS[i].name << "\n";
                    cout << "      RM" << fixed << setprecision(2) << ADDONS[i].pricePerUnit
                        << " / " << ADDONS[i].unit << " / day\n\n";
                }

                if (addonSelections.empty())
                {
                    cout << "Select (A-C, Enter N to skip add-ons, Enter 0 to reselect the quantity of the room type): ";
                }
                else
                {
                    cout << "Select (A-C, Enter N to skip add-ons, Enter 0 to remove the last add-on): ";
                }
                string input;
                getline(cin, input);

                if (input == "N" || input == "n")
                {
                    stage = STAGE_SUMMARY;
                    break;
                }
                if (isGoBackInput(input))
                {
                    if (addonSelections.empty())
                    {
                        if (!selections.empty())
                        {
                            reselectType = selections.back().typeIndex;
                            selections.pop_back();
                        }
                        stage = STAGE_ROOMS;
                        break;
                    }
                    addonSelections.pop_back();
                    continue;
                }
                if (input.length() != 1)
                {
                    cout << "Invalid. Please enter A-C, N, or 0.\n";
                    cout << "\nPress Enter to try again...";
                    string dummy;
                    getline(cin, dummy);
                    continue;
                }

                char upperInput = toupper(input[0]);
                if (upperInput < 'A' || upperInput > 'C')
                {
                    cout << "Invalid. Please enter A-C, N, or 0.\n";
                    cout << "\nPress Enter to try again...";
                    string dummy;
                    getline(cin, dummy);
                    continue;
                }

                int ai = upperInput - 'A';
                bool alreadyPicked = false;
                for (const SelectedAddOn& sel : addonSelections)
                {
                    if (sel.addonIndex == ai)
                    {
                        alreadyPicked = true;
                        break;
                    }
                }
                if (alreadyPicked)
                {
                    cout << "\n  " << ADDONS[ai].name << " has already been selected. Choose another service,\n"
                        << "  N to skip, or 0 to go back.\n";
                    cout << "\nPress Enter to continue...";
                    string dummy;
                    getline(cin, dummy);
                    continue;
                }

                int qty = 0;
                for (const SelectedRoom& sel : selections)
                {
                    if (ADDONS[ai].unit == "room")
                    {
                        qty += sel.quantity;
                    }
                    else
                    {
                        qty += sel.quantity * ROOM_TYPES[sel.typeIndex].capacity;
                    }
                }

                addonSelections.push_back({ ai, qty });

                cout << "\n  >> Selected: " << qty << "x " << ADDONS[ai].name
                    << " (RM" << fixed << setprecision(2) << ADDONS[ai].pricePerUnit << "/" << ADDONS[ai].unit << "/day)\n";

                cout << "\n  -- Your add-ons --\n";
                double currentAddonTotal = 0;
                for (const SelectedAddOn& sel : addonSelections)
                {
                    double cost = sel.quantity * ADDONS[sel.addonIndex].pricePerUnit;
                    cout << "  " << sel.quantity << "x " << ADDONS[sel.addonIndex].name
                        << " (RM" << fixed << setprecision(2) << cost << "/day)\n";
                    currentAddonTotal += cost;
                }
                cout << "  -----------------------\n";
                cout << "  Add-on total per day: RM" << fixed << setprecision(2) << currentAddonTotal << "\n";

                bool allSelected = true;
                for (int i = 0; i < NUM_ADDONS; i++)
                {
                    bool found = false;
                    for (const SelectedAddOn& sel : addonSelections)
                    {
                        if (sel.addonIndex == i) { found = true; break; }
                    }
                    if (!found) { allSelected = false; break; }
                }
                if (allSelected)
                {
                    stage = STAGE_SUMMARY;
                    break;
                }

                cout << "\nAdd more services? (Y/N) (Enter 0 to remove the last add-on): ";
                string more;
                getline(cin, more);
                if (isGoBackInput(more))
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

    const double SERVICE_CHARGE_RATE = 0.08;
    const double TAX_RATE = 0.06;

    // ── SUMMARY ──
    if (stage == STAGE_SUMMARY)
    {
        while (stage == STAGE_SUMMARY)
        {
            clearScreen();
            nights = 0;
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
                roomPerNight += sel.quantity * ROOM_TYPES[sel.typeIndex].price;
                totalRooms += sel.quantity;
            }

            double addonPerDay = 0;
            for (const SelectedAddOn& sel : addonSelections)
            {
                addonPerDay += sel.quantity * ADDONS[sel.addonIndex].pricePerUnit;
            }

            roomTotal = roomPerNight * nights;
            addonTotal = addonPerDay * nights;
            double total = roomTotal + addonTotal;

            subtotalBeforeDiscount = total;
            applyVIPDiscount(total, customerId);
            vipDiscountAmount = subtotalBeforeDiscount - total;

            serviceAmount = total * SERVICE_CHARGE_RATE;
            taxAmount = total * TAX_RATE;
            grandTotal = total + serviceAmount + taxAmount;

            cout << "\n====================================\n";
            cout << "       BOOKING SUMMARY\n";
            cout << "====================================\n";
            cout << "  Check-in:   " << formatDate(checkInDate) << "\n";
            cout << "  Check-out:  " << formatDate(checkOutDate) << "\n";
            cout << "  Nights:     " << nights << "\n";
            cout << "  --------------------------------\n";
            cout << "  ROOMS:\n";

            for (const SelectedRoom& sel : selections)
            {
                double perNight = sel.quantity * ROOM_TYPES[sel.typeIndex].price;
                double lineTotal = perNight * nights;
                cout << "  " << sel.quantity << "x " << ROOM_TYPES[sel.typeIndex].name << "\n";
                cout << "     RM" << fixed << setprecision(2) << perNight << "/night x "
                    << nights << " nights = RM" << lineTotal << "\n";
            }
            cout << "  --------------------------------\n";
            cout << "  Room total: RM" << fixed << setprecision(2) << roomTotal << "\n";

            if (!addonSelections.empty())
            {
                cout << "  --------------------------------\n";
                cout << "  ADD-ONS:\n";
                for (const SelectedAddOn& sel : addonSelections)
                {
                    double perDay = sel.quantity * ADDONS[sel.addonIndex].pricePerUnit;
                    double lineTotal = perDay * nights;
                    cout << "  " << sel.quantity << "x " << ADDONS[sel.addonIndex].name << "\n";
                    cout << "     RM" << fixed << setprecision(2) << perDay << "/day x "
                        << nights << " day(s) = RM" << lineTotal << "\n";
                }
                cout << "  --------------------------------\n";
                cout << "  Add-on total: RM" << fixed << setprecision(2) << addonTotal << "\n";
            }

            if (vipDiscountAmount > 0.0)
            {
                cout << "  --------------------------------\n";
                cout << "  Subtotal: RM" << fixed << setprecision(2) << subtotalBeforeDiscount << "\n";
                cout << "  VIP Discount: - RM" << fixed << setprecision(2) << vipDiscountAmount << "\n";
            }

            cout << "  ================================\n";
            cout << "  Service Charge (8%) : RM" << fixed << setprecision(2) << serviceAmount << "\n";
            cout << "  Government Tax (6%) : RM" << fixed << setprecision(2) << taxAmount << "\n";
            cout << "  GRAND TOTAL: RM" << fixed << setprecision(2) << grandTotal << "\n";
            cout << "  Total rooms: " << totalRooms << "\n";
            cout << "====================================\n";

            if (addonSelections.empty())
            {
                cout << "\n  Enter 0 to reselect the quantity of the room type, or press Enter to continue: ";
            }
            else
            {
                cout << "\n  Enter 0 to remove the last add-on, or press Enter to continue: ";
            }
            string zInput;
            getline(cin, zInput);
            if (isGoBackInput(zInput))
            {
                if (addonSelections.empty())
                {
                    if (!selections.empty())
                    {
                        reselectType = selections.back().typeIndex;
                        selections.pop_back();
                    }
                    stage = STAGE_ROOMS;
                }
                else
                {
                    addonSelections.pop_back();
                    stage = (addonSelections.empty() && !selections.empty()) ? STAGE_ROOMS : STAGE_ADDONS;
                }
            }
            else
            {
                break;
            }
        }
    }

    if (stage == STAGE_SUMMARY)
    {
        string addonsString;
        for (size_t i = 0; i < addonSelections.size(); i++)
        {
            if (i > 0) addonsString += ";";
            addonsString += to_string(addonSelections[i].quantity) + "x " + ADDONS[addonSelections[i].addonIndex].name;
        }

        // ── Payment Method Section (Dispatched via clean helper calls) ──
        while (true)
        {
            cout << "\n====================================\n";
            cout << "     PAYMENT METHOD\n";
            cout << "====================================\n";
            cout << "  [A] Online Banking\n";
            cout << "  [B] E-Wallet\n";
            cout << "  [C] Cash\n";
            cout << "  [D] Credit / Debit Card\n";

            cout << "Select payment method (A-D): ";
            string input;
            getline(cin, input);
            char payMethod = toupper(input.empty() ? ' ' : input[0]);

            bool paymentSuccess = false;

            if (payMethod == 'A')
            {
                paymentSuccess = processOnlineBankingPayment(
                    customerId, checkInDate, checkOutDate, selections, addonSelections,
                    rooms, bookings, addonsString, nights, roomTotal, addonTotal,
                    subtotalBeforeDiscount, vipDiscountAmount, serviceAmount, taxAmount, grandTotal);
            }
            else if (payMethod == 'B')
            {
                paymentSuccess = processEWalletPayment(
                    customerId, checkInDate, checkOutDate, selections, addonSelections,
                    rooms, bookings, addonsString, nights, roomTotal, addonTotal,
                    subtotalBeforeDiscount, vipDiscountAmount, serviceAmount, taxAmount, grandTotal);
            }
            else if (payMethod == 'C')
            {
                paymentSuccess = processCashPayment(
                    customerId, checkInDate, checkOutDate, selections, rooms, bookings, addonsString);
            }
            else if (payMethod == 'D')
            {
                paymentSuccess = processCardPayment(
                    customerId, checkInDate, checkOutDate, selections, addonSelections,
                    rooms, bookings, addonsString, nights, roomTotal, addonTotal,
                    subtotalBeforeDiscount, vipDiscountAmount, serviceAmount, taxAmount, grandTotal);
            }
            else
            {
                cout << "Invalid. Please enter between A - D.\n";
                continue;
            }

            if (paymentSuccess)
            {
                return;
            }
        }
    }
}

// Helper: print a box line "| content              |"
static void printBoxLine(const string& content)
{
    const int width = 34;
    cout << "  |";
    cout << left << setw(width) << (" " + content);
    cout << "|\n";
}

static void printBoxBorder()
{
    cout << "  +----------------------------------+\n";
}

void viewPreviousBookings(const string& customerId)
{
    clearScreen();


    cout << "\n====================================\n";
    cout << "   PREVIOUS BOOKING RECORDS\n";
    cout << "====================================\n";

    vector<Room> rooms;
    vector<Booking> bookings;
    loadSchedulerDemoData(rooms, bookings);
    vector<Booking> saved = loadSavedBookings();

    vector<string> bookingIds;
    map<string, vector<Booking>> bookingsById;

    for (const Booking& b : saved)
    {
        if (b.customerId == customerId && b.status != "CANCELLED")
        {
            if (bookingsById.find(b.bookingId) == bookingsById.end())
            {
                bookingIds.push_back(b.bookingId);
            }
            bookingsById[b.bookingId].push_back(b);
        }
    }

    if (bookingIds.empty())
    {
        cout << "\n  No booking records found.\n";
        cout << "====================================\n";
        EnterToContinue();
        return;
    }

    for (size_t i = 0; i < bookingIds.size(); i++)
    {
        const string& bid = bookingIds[i];
        const vector<Booking>& group = bookingsById[bid];
        const Booking& first = group[0];

        cout << "\n";
        printBoxBorder();
        printBoxLine("Booking " + bid);
        printBoxBorder();
        printBoxLine("Check-in:  " + formatDate(first.checkInDate));
        printBoxLine("Check-out: " + formatDate(first.checkOutDate));
        printBoxLine("Rooms:");
        for (const Booking& b : group)
        {
            string roomType = "Unknown";
            for (const Room& r : rooms)
            {
                if (r.roomId == b.roomId)
                {
                    roomType = r.roomType;
                    break;
                }
            }
            printBoxLine("  - " + b.roomId + " (" + roomType + ")");
            printBoxLine("    Access Code: " + b.accessCode);
        }
        if (!first.addons.empty())
        {
            printBoxLine("Add-ons:");
            istringstream addonStream(first.addons);
            string addonItem;
            while (getline(addonStream, addonItem, ';'))
            {
                printBoxLine("  - " + addonItem);
            }
        }
        printBoxBorder();
    }

    cout << "\n====================================\n";
    cout << "\nPress any key to continue: ";
    string input;
    getline(cin, input);
}

void cancelBooking(const string& customerId)
{
    clearScreen();

    cout << "\n====================================\n";
    cout << "         CANCEL BOOKING\n";
    cout << "====================================\n";

    vector<Room> rooms;
    vector<Booking> bookings;
    loadSchedulerDemoData(rooms, bookings);
    vector<Booking> saved = loadSavedBookings();

    vector<string> bookingIds;
    map<string, vector<Booking>> bookingsById;

    for (const Booking& b : saved)
    {
        if (b.customerId == customerId && b.status != "CANCELLED" && b.status != "COMPLETED")
        {
            if (bookingsById.find(b.bookingId) == bookingsById.end())
            {
                bookingIds.push_back(b.bookingId);
            }
            bookingsById[b.bookingId].push_back(b);
        }
    }

    if (bookingIds.empty())
    {
        cout << "\n  No active bookings to cancel.\n";
        cout << "====================================\n";
        EnterToContinue();
        return;
    }

    for (size_t i = 0; i < bookingIds.size(); i++)
    {
        const string& bid = bookingIds[i];
        const vector<Booking>& group = bookingsById[bid];
        const Booking& first = group[0];

        cout << "\n";
        printBoxBorder();
        printBoxLine("Booking " + bid);
        printBoxBorder();
        printBoxLine("Check-in:  " + formatDate(first.checkInDate));
        printBoxLine("Check-out: " + formatDate(first.checkOutDate));
        printBoxLine("Rooms:");
        for (const Booking& b : group)
        {
            string roomType = "Unknown";
            for (const Room& r : rooms)
            {
                if (r.roomId == b.roomId)
                {
                    roomType = r.roomType;
                    break;
                }
            }
            printBoxLine("  - " + b.roomId + " (" + roomType + ")");
            printBoxLine("    Access Code: " + b.accessCode);
        }
        if (!first.addons.empty())
        {
            printBoxLine("Add-ons:");
            istringstream addonStream(first.addons);
            string addonItem;
            while (getline(addonStream, addonItem, ';'))
            {
                printBoxLine("  - " + addonItem);
            }
        }
        printBoxBorder();
    }

    cout << "\n====================================\n";

    string selectedId = "";
    while (selectedId.empty())
    {
        cout << "\n  Enter booking ID to cancel (e.g. B1)\n";
        cout << "  Enter 0 to go back: ";
        string input;
        getline(cin, input);

        if (isGoBackInput(input))
        {
            return;
        }

        string upperInput = input;
        for (char& c : upperInput)
        {
            c = static_cast<char>(toupper(static_cast<unsigned char>(c)));
        }

        for (const string& bid : bookingIds)
        {
            if (bid == upperInput)
            {
                selectedId = bid;
                break;
            }
        }

        if (selectedId.empty())
        {
            cout << "Invalid booking ID. Please try again.\n";
        }
    }
    const vector<Booking>& group = bookingsById[selectedId];

    cout << "\n  Cancel entire booking " << selectedId << " (" << group.size() << " room(s))?\n";
    cout << "  Confirm (Y/N): ";
    string confirm;
    getline(cin, confirm);

    if (confirm == "Y" || confirm == "y")
    {
        for (Booking& b : saved)
        {
            if (b.bookingId == selectedId)
            {
                b.status = "CANCELLED";
            }
        }

        bool foundInSaved = false;
        for (const Booking& b : saved)
        {
            if (b.bookingId == selectedId)
            {
                foundInSaved = true;
                break;
            }
        }
        if (!foundInSaved)
        {
            for (const Booking& b : group)
            {
                Booking cancelled = b;
                cancelled.status = "CANCELLED";
                saved.push_back(cancelled);
            }
        }
        saveAllBookings(saved);

        cout << "\n  *** CANCELLATION SUCCESSFUL ***\n";
        cout << "  Booking " << selectedId << " (" << group.size() << " room(s)) has been cancelled.\n";
        cout << "====================================\n";
        EnterToContinue();
    }
    else
    {
        cout << "\n  Cancellation was not executed.\n";
        cout << "  Returning to main menu.\n";
        cout << "====================================\n";
        EnterToContinue();
    }
}
