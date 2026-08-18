#include "employee.h"
#include "scheduler.h"
#include "utilities.h"

#include <iostream>
#include <limits>
#include <vector>

using namespace std;
void empHomeScreen()
{
    const string EMPLOYEE_PASSCODE = "1234";
    string enteredPasscode;

    while (true)
    {
        cout << "\nEmployee Access\n";
        cout << "Enter employee passcode [999 to go back]: ";
        getline(cin, enteredPasscode);

        if (enteredPasscode == "999")
        {
            return;
        }

        if (enteredPasscode == EMPLOYEE_PASSCODE)
        {
            cout << "\nEmployee passcode accepted.\n";
            return;
        }

        cout << "Invalid, Wrong employee passcode. Please try again.\n";
    }
}