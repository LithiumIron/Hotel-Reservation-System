#include "vip.h"
#include "utilities.h"
#include "scheduler.h"
#include "booking.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <ctime>
#include <cctype>

using namespace std;

const string VIP_FILE = "vipData.txt";

// ============================================================
// VIP TIER DEFINITIONS
// ============================================================

const VIPTier VIP_TIERS[] = {
    {
        "Silver",
        50.0,
        "* 10\% off room bookings\n"
        "* Free WiFi\n"
        "* Priority check-in\n",
        3
    },

    {
        "Gold",
        100.0,
        "* 20\% off room bookings\n"
        "* Free WiFi & Breakfast\n"
        "* Priority check-in\n"
        "* Late check-out (2pm)\n"
        "* Room upgrade when available\n",
        6
    },

    {
        "Platinum",
        200.0,
        "* 30\% off room bookings\n"
        "* Free WiFi, Breakfast & Parking\n"
        "* Priority check-in & Late check-out (4pm)\n"
        "* Guaranteed room upgrade\n"
        "* Personal concierge service\n"
        "* Exclusive member events\n"
        "* Spa access (2 sessions/month)\n",
        12
    }
};

const int NUM_TIERS = 3;


// ============================================================
// DATE FUNCTIONS
// ============================================================

Date addMonths(const Date& date, int months)
{
    Date result = date;

    result.month += months;

    while (result.month > 12)
    {
        result.month -= 12;
        result.year++;
    }

    return result;
}


// ============================================================
// SAVE VIP MEMBERSHIP
// ============================================================

void saveVIPMembership(const VIPMembership& membership)
{
    ofstream outFile(VIP_FILE, ios::app);

    if (!outFile)
    {
        cout << "Error: Could not save VIP membership.\n";
        return;
    }

    outFile << membership.customerId << '\t'
            << membership.tier << '\t'
            << membership.purchaseDate.day << '\t'
            << membership.purchaseDate.month << '\t'
            << membership.purchaseDate.year << '\t'
            << membership.expiryDate.day << '\t'
            << membership.expiryDate.month << '\t'
            << membership.expiryDate.year << '\t'
            << membership.price << '\t'
            << (membership.isActive ? "1" : "0")
            << '\n';

    outFile.close();
}


// ============================================================
// LOAD VIP MEMBERSHIPS
// ============================================================

vector<VIPMembership> loadVIPMemberships()
{
    vector<VIPMembership> memberships;

    ifstream inFile(VIP_FILE);

    if (!inFile)
    {
        return memberships;
    }

    VIPMembership m;
    string active;

    while (inFile >> m.customerId
                  >> m.tier
                  >> m.purchaseDate.day
                  >> m.purchaseDate.month
                  >> m.purchaseDate.year
                  >> m.expiryDate.day
                  >> m.expiryDate.month
                  >> m.expiryDate.year
                  >> m.price
                  >> active)
    {
        m.isActive = (active == "1");

        memberships.push_back(m);
    }

    inFile.close();

    return memberships;
}


// ============================================================
// SAVE ALL VIP MEMBERSHIPS
// Used when upgrading/deactivating old memberships
// ============================================================

void saveAllVIPMemberships(const vector<VIPMembership>& memberships)
{
    ofstream outFile(VIP_FILE);

    if (!outFile)
    {
        cout << "Error: Could not update VIP membership data.\n";
        return;
    }

    for (const VIPMembership& membership : memberships)
    {
        outFile << membership.customerId << '\t'
                << membership.tier << '\t'
                << membership.purchaseDate.day << '\t'
                << membership.purchaseDate.month << '\t'
                << membership.purchaseDate.year << '\t'
                << membership.expiryDate.day << '\t'
                << membership.expiryDate.month << '\t'
                << membership.expiryDate.year << '\t'
                << membership.price << '\t'
                << (membership.isActive ? "1" : "0")
                << '\n';
    }

    outFile.close();
}


// ============================================================
// CHECK ACTIVE VIP
// ============================================================

bool hasActiveVIP(const string& customerId)
{
    vector<VIPMembership> memberships = loadVIPMemberships();

    Date today = getCurrentSystemDate();

    for (const VIPMembership& m : memberships)
    {
        if (m.customerId == customerId && m.isActive)
        {
            if (compareDates(today, m.expiryDate) <= 0)
            {
                return true;
            }
        }
    }

    return false;
}


// ============================================================
// GET VIP STATUS
// ============================================================

string getVIPStatus(const string& customerId)
{
    vector<VIPMembership> memberships = loadVIPMemberships();

    Date today = getCurrentSystemDate();

    for (const VIPMembership& m : memberships)
    {
        if (m.customerId == customerId && m.isActive)
        {
            if (compareDates(today, m.expiryDate) <= 0)
            {
                return m.tier;
            }
        }
    }

    return "None";
}


// ============================================================
// APPLY VIP DISCOUNT
// ============================================================

void applyVIPDiscount(double& total, const string& customerId)
{
    string tier = getVIPStatus(customerId);

    if (tier == "Silver")
    {
        total *= 0.90;

        cout << "  Silver VIP discount applied! (10% off)\n";
    }
    else if (tier == "Gold")
    {
        total *= 0.80;

        cout << "  Gold VIP discount applied! (20% off)\n";
    }
    else if (tier == "Platinum")
    {
        total *= 0.70;

        cout << "  Platinum VIP discount applied! (30% off)\n";
    }
}


// ============================================================
// VIEW VIP MEMBERSHIP
// ============================================================

void viewVIPMembership()
{
    cout << "\n====================================\n";
    cout << "         MY VIP MEMBERSHIP\n";
    cout << "====================================\n";

    if (loggedInUser.empty())
    {
        cout << "  No user is currently logged in.\n";
        cout << "====================================\n";

        EnterToContinue();
        return;
    }

    string tier = getVIPStatus(loggedInUser);

    Date today = getCurrentSystemDate();

    if (tier == "None")
    {
        cout << "\n  You do not have an active VIP membership.\n";
        cout << "  Purchase one today to enjoy exclusive benefits!\n";
        cout << "====================================\n";

        EnterToContinue();
        return;
    }

    vector<VIPMembership> memberships = loadVIPMemberships();

    for (const VIPMembership& m : memberships)
    {
        if (m.customerId == loggedInUser &&
            m.isActive &&
            compareDates(today, m.expiryDate) <= 0)
        {
            cout << "\n  " << string(40, '-') << "\n";
            cout << "  VIP TIER: " << m.tier << "\n";
            cout << "  " << string(40, '-') << "\n";

            cout << "  Purchase Date: " << formatDate(m.purchaseDate) << "\n";
            cout << "  Expiry Date:   " << formatDate(m.expiryDate) << "\n";

            // Calculate remaining days
            int daysRemaining = 0;

            Date temp = today;

            while (compareDates(temp, m.expiryDate) < 0)
            {
                daysRemaining++;

                temp = addDays(temp, 1);
            }

            cout << "  Days Remaining: " << daysRemaining << " days\n";

            cout << "  Price Paid:    RM "
                 << fixed
                 << setprecision(2)
                 << m.price
                 << "\n";

            cout << "\n  --- Your VIP Benefits ---\n";

            for (int i = 0; i < NUM_TIERS; i++)
            {
                if (VIP_TIERS[i].name == m.tier)
                {
                    cout << VIP_TIERS[i].benefits;
                    break;
                }
            }

            cout << "\n  " << string(40, '-') << "\n";

            break;
        }
    }

    cout << "====================================\n";

    EnterToContinue();
}


// ============================================================
// PURCHASE / UPGRADE VIP MEMBERSHIP
// ============================================================

void purchaseVIPMembership()
{
    cout << "\n====================================\n";
    cout << "     VIP MEMBERSHIP\n";
    cout << "====================================\n";

    if (loggedInUser.empty())
    {
        cout << "  Please login first.\n";
        cout << "====================================\n";

        EnterToContinue();
        return;
    }

    // --------------------------------------------------------
    // CHECK CURRENT MEMBERSHIP
    // --------------------------------------------------------

    string currentTier = getVIPStatus(loggedInUser);

    if (currentTier != "None")
    {
        cout << "\n  You currently have: "
             << currentTier
             << " VIP\n";

        int currentIndex = -1;

        for (int i = 0; i < NUM_TIERS; i++)
        {
            if (VIP_TIERS[i].name == currentTier)
            {
                currentIndex = i;
                break;
            }
        }

        // ----------------------------------------------------
        // UPGRADE AVAILABLE
        // ----------------------------------------------------

        if (currentIndex >= 0 &&
            currentIndex < NUM_TIERS - 1)
        {
            const VIPTier& currentVIP =
                VIP_TIERS[currentIndex];

            const VIPTier& nextVIP =
                VIP_TIERS[currentIndex + 1];

            double upgradePrice =
                nextVIP.price - currentVIP.price;

            cout << "\n  You can upgrade to: "
                 << nextVIP.name
                 << " VIP\n";

            cout << "  Upgrade Price: RM "
                 << fixed
                 << setprecision(2)
                 << upgradePrice
                 << "\n";

            cout << "\n  Upgrade now? (Y/N): ";

            string upgrade;
            getline(cin, upgrade);

            if (upgrade.empty() ||
                toupper(static_cast<unsigned char>(upgrade[0])) != 'Y')
            {
                cout << "\n  Upgrade cancelled.\n";
                cout << "====================================\n";

                EnterToContinue();
                return;
            }

            // ------------------------------------------------
            // PROCESS PAYMENT
            // ------------------------------------------------

            cout << "\n  Processing upgrade...\n";
            cout << "  Payment successful!\n";

            // ------------------------------------------------
            // LOAD EXISTING MEMBERSHIPS
            // ------------------------------------------------

            vector<VIPMembership> memberships =
                loadVIPMemberships();

            // ------------------------------------------------
            // DEACTIVATE OLD MEMBERSHIP
            // ------------------------------------------------

            for (VIPMembership& m : memberships)
            {
                if (m.customerId == loggedInUser &&
                    m.isActive)
                {
                    m.isActive = false;
                }
            }

            // ------------------------------------------------
            // CREATE NEW MEMBERSHIP
            // ------------------------------------------------

            VIPMembership membership;

            membership.customerId = loggedInUser;

            membership.tier = nextVIP.name;

            membership.purchaseDate =
                getCurrentSystemDate();

            membership.expiryDate =
                addMonths(
                    membership.purchaseDate,
                    nextVIP.durationMonths
                );

            membership.price = upgradePrice;

            membership.isActive = true;

            memberships.push_back(membership);

            // ------------------------------------------------
            // SAVE UPDATED MEMBERSHIP DATA
            // ------------------------------------------------

            saveAllVIPMemberships(memberships);

            cout << "\n  CONGRATULATIONS!\n";

            cout << "  You've been upgraded to "
                 << membership.tier
                 << " VIP!\n";

            cout << "  Valid until: "
                 << formatDate(membership.expiryDate)
                 << "\n";

            cout << "====================================\n";

            EnterToContinue();

            return;
        }
        else
        {
            cout << "\n  You already have the highest VIP tier!\n";
            cout << "  Thank you for being a Platinum VIP member!\n";
            cout << "====================================\n";

            EnterToContinue();

            return;
        }
    }

    // ========================================================
    // NO CURRENT MEMBERSHIP
    // SHOW PURCHASE OPTIONS
    // ========================================================

    cout << "\n  Available VIP Tiers:\n";
    cout << "  " << string(50, '-') << "\n";

    for (int i = 0; i < NUM_TIERS; i++)
    {
        cout << "  [" << (i + 1) << "] "
             << VIP_TIERS[i].name
             << " VIP\n";

        cout << "      Price: RM "
             << fixed
             << setprecision(2)
             << VIP_TIERS[i].price
             << "\n";

        cout << "      Duration: "
             << VIP_TIERS[i].durationMonths
             << " months\n";

        cout << "  "
             << string(50, '-')
             << "\n";
    }

    cout << "  [0] Cancel\n";

    int choice =
        readInteger(
            "Select tier: ",
            0,
            NUM_TIERS
        );

    if (choice == 0)
    {
        cout << "\n  Purchase cancelled.\n";
        cout << "====================================\n";

        EnterToContinue();
        return;
    }

    int tierIndex = choice - 1;

    const VIPTier& selectedTier =
        VIP_TIERS[tierIndex];

    // ========================================================
    // CONFIRM PURCHASE
    // ========================================================

    cout << "\n  You are about to purchase: "
         << selectedTier.name
         << " VIP\n";

    cout << "  Price: RM "
         << fixed
         << setprecision(2)
         << selectedTier.price
         << "\n";

    cout << "  Duration: "
         << selectedTier.durationMonths
         << " months\n";

    cout << "\n  Confirm purchase? (Y/N): ";

    string confirm;

    getline(cin, confirm);

    if (confirm.empty() ||
        toupper(static_cast<unsigned char>(confirm[0])) != 'Y')
    {
        cout << "\n  Purchase cancelled.\n";
        cout << "====================================\n";

        EnterToContinue();
        return;
    }

    // ========================================================
    // PAYMENT
    // ========================================================

    cout << "\n  Processing payment...\n";

    cout << "  Please enter your payment details:\n";

    cout << "  Card Number (XXXX-XXXX-XXXX-XXXX): ";

    string card;
    getline(cin, card);

    cout << "  Expiry (MM/YY): ";

    string expiry;
    getline(cin, expiry);

    cout << "  CVV: ";

    string cvv;
    getline(cin, cvv);

    // NOTE:
    // This is only a simulated payment system.
    // No actual payment processing is performed.

    cout << "\n  Payment successful!\n";

    // ========================================================
    // CREATE MEMBERSHIP
    // ========================================================

    VIPMembership membership;

    membership.customerId = loggedInUser;

    membership.tier = selectedTier.name;

    membership.purchaseDate =
        getCurrentSystemDate();

    membership.expiryDate =
        addMonths(
            membership.purchaseDate,
            selectedTier.durationMonths
        );

    membership.price = selectedTier.price;

    membership.isActive = true;

    saveVIPMembership(membership);

    // ========================================================
    // SUCCESS MESSAGE
    // ========================================================

    cout << "\n  CONGRATULATIONS!\n";

    cout << "  You are now a "
         << selectedTier.name
         << " VIP member!\n";

    cout << "  Your membership is valid until: "
         << formatDate(membership.expiryDate)
         << "\n";

    cout << "\n  Enjoy your exclusive benefits!\n";

    cout << "====================================\n";

    EnterToContinue();
}


// ============================================================
// VIEW VIP BENEFITS
// ============================================================

void viewVIPBenefits()
{
    cout << "\n====================================\n";
    cout << "         VIP BENEFITS\n";
    cout << "====================================\n";

    cout << "\n  "
         << string(50, '=')
         << "\n";

    cout << "  VIP MEMBERSHIP TIERS\n";

    cout << "  "
         << string(50, '=')
         << "\n\n";

    for (int i = 0; i < NUM_TIERS; i++)
    {
        const VIPTier& tier =
            VIP_TIERS[i];

        cout << "  "
             << string(48, '-')
             << "\n";

        cout << "  "
             << tier.name
             << " VIP\n";

        cout << "  "
             << string(48, '-')
             << "\n";

        cout << "  Price: RM "
             << fixed
             << setprecision(2)
             << tier.price
             << "\n";

        cout << "  Duration: "
             << tier.durationMonths
             << " months\n";

        cout << "  Benefits:\n";

        cout << tier.benefits;

        cout << "  "
             << string(48, '-')
             << "\n\n";
    }

    // ========================================================
    // SHOW CURRENT USER STATUS
    // ========================================================

    if (!loggedInUser.empty())
    {
        string currentTier =
            getVIPStatus(loggedInUser);

        cout << "\n  Your current status: ";

        if (currentTier == "None")
        {
            cout << "No active VIP membership\n";

            cout << "  Purchase one today to enjoy exclusive benefits!\n";
        }
        else
        {
            cout << currentTier
                 << " VIP Member\n";

            cout << "  🎉 Enjoy your exclusive benefits!\n";
        }
    }

    cout << "====================================\n";

    EnterToContinue();
}