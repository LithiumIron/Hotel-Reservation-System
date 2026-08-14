#include "utilities.h"

#include <cctype>
#include <iostream>
#include <string>

using namespace std;

int numberValidation(const string& userInput, int limit)
{
    if (userInput.empty())
    {
        cout << "Error: Input cannot be empty.\n";
        return 1;
    }

    for (char character : userInput)
    {
        if (!isdigit(static_cast<unsigned char>(character)))
        {
            cout << "Error: Please enter a number from 1 to "
                << limit << ".\n";
            return 1;
        }
    }

    try
    {
        int number = stoi(userInput);

        if (number < 1 || number > limit)
        {
            cout << "Error: Please enter a number from 1 to "
                << limit << ".\n";
            return 1;
        }
    }
    catch (...)
    {
        cout << "Error: The number entered is too large.\n";
        return 1;
    }

    return 0;
}