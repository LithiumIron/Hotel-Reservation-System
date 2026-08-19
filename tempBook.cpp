#include "booking.h"
#include "scheduler.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

namespace
{
    const Date SYSTEM_DATE{ 19, 8, 2026 };

    Date readDate(const string& heading)
    {
        while (true)
        {
            cout << '\n' << heading << "\n";
            cout << "Enter date (DD/MM/YYYY): ";
            string input;
            getline(cin, input);

            // Parse DD/MM/YYYY
            if (input.length() != 10 || input[2] != '/' || input[5] != '/')
            {
                cout << "Invalid format. Please use DD/MM/YYYY.\n";
                continue;
            }

            istringstream parser(input);
            int day, month, year;
            char slash1, slash2;

            if (!(parser >> day >> slash1 >> month >> slash2 >> year))
            {
                cout << "Invalid date. Please enter numeric values.\n";
                continue;
            }

            Date date{ day, month, year };

            if (!isValidDate(date))
            {
                cout << "Invalid date. Please check day/month/year.\n";
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

    Date readCurrentOrFutureDate(const string& heading)
    {
        while (true)
        {
            Date date = readDate(heading);
            if (compareDates(date, SYSTEM_DATE) >= 0)
            {
                return date;
            }
            cout << "Past dates are not allowed. Earliest date: "
                 << formatDate(SYSTEM_DATE) << ".\n";
        }
    }
}

void bookingScreen()
{
    cout << "\n====================================\n";
    cout << "         ROOM BOOKING\n";
    cout << "====================================\n";
    cout << "Format: DD/MM/YYYY (e.g. 25/12/2026)\n";
    cout << "====================================\n";

    Date checkInDate = readCurrentOrFutureDate("Check-in Date");

    Date checkOutDate;
    while (true)
    {
        checkOutDate = readCurrentOrFutureDate("Check-out Date");
        if (compareDates(checkOutDate, checkInDate) > 0)
        {
            break;
        }
        cout << "Check-out date must be later than check-in date ("
             << formatDate(checkInDate) << ").\n";
    }

    cout << "\n====================================\n";
    cout << "  Check-in:  " << formatDate(checkInDate) << "\n";
    cout << "  Check-out: " << formatDate(checkOutDate) << "\n";
    cout << "====================================\n";
}
