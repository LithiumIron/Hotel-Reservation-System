#include "vip.h"
#include "utilities.h"
#include "scheduler.h"
#include "booking.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <cctype>

using namespace std;

// Store VIP membership data file
const string VIP_FILE = "vipData.txt";

// ============================================================
// VIP TIER DEFINITIONS
// ============================================================

// Store available VIP tiers and their benefits
const VIPTier VIP_TIERS[] =
{
    {
        "Silver",
        50.0,
        "* 10% off room bookings\n"
        "* Free WiFi\n"
        "* Priority check-in\n",
        3
    },
    {
        "Gold",
        100.0,
        "* 20% off room bookings\n"
        "* Free WiFi & Breakfast\n"
        "* Priority check-in\n"
        "* Late check-out (2pm)\n"
        "* Room upgrade when available\n",
        6
    },
    {
        "Platinum",
        200.0,
        "* 30% off room bookings\n"
        "* Free WiFi, Breakfast & Parking\n"
        "* Priority check-in & Late check-out (4pm)\n"
        "* Guaranteed room upgrade\n"
        "* Personal concierge service\n"
        "* Exclusive member events\n"
        "* Spa access (2 sessions/month)\n",
        12
    }
};

// Number of available VIP tiers
const int NUM_TIERS = 3;

// ============================================================
// DATE FUNCTIONS
// ============================================================

// Add a number of months to a date
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

// Save a new VIP membership to the file
void saveVIPMembership(const VIPMembership& membership)
{
    ofstream outFile(VIP_FILE, ios::app);

    if (!outFile)
    {
        cout << "Error: Could not save VIP membership.\n";
        return;
    }

    // Write membership details to the file
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

// Load all VIP memberships from the file
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

    // Read each membership record
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
// ============================================================

// Rewrite the file with updated membership records
void saveAllVIPMemberships(const vector<VIPMembership>& memberships)
{
    ofstream outFile(VIP_FILE);

    if (!outFile)
    {
        cout << "Error: Could not update VIP membership data.\n";
        return;
    }

    // Write all memberships back to the file
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

// Check whether a customer has an active VIP membership
bool hasActiveVIP(const string& customerId)
{
    vector<VIPMembership> memberships = loadVIPMemberships();
    Date today = getCurrentSystemDate();

    for (const VIPMembership& m : memberships)
    {
        if (m.customerId == customerId && m.isActive)
        {
            // Check if the membership has not expired
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

// Get the customer's current VIP tier
string getVIPStatus(const string& customerId)
{
    vector<VIPMembership> memberships = loadVIPMemberships();
    Date today = getCurrentSystemDate();

    for (const VIPMembership& m : memberships)
    {
        if (m.customerId == customerId && m.isActive)
        {
            // Check if the membership is still valid
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

// Apply the discount based on the customer's VIP tier
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
// PURCHASE / UPGRADE VIP MEMBERSHIP
// ============================================================

// Allow customers to purchase or upgrade their VIP membership
void purchaseVIPMembership(const string& customerId)
{
    clearScreen();

    cout << "\n====================================\n";
    cout << "         VIP MEMBERSHIP\n";
    cout << "====================================\n";

    // Check if a customer is logged in
    if (customerId.empty())
    {
        cout << "  Please login first.\n";
        cout << "====================================\n";
        EnterToContinue();
        return;
    }

    // Check current membership
    string currentTier = getVIPStatus(customerId);

    if (currentTier != "None")
    {
        cout << "\nYou currently have: " << currentTier << " VIP\n";

        int currentIndex = -1;

        // Find the customer's current tier
        for (int i = 0; i < NUM_TIERS; i++)
        {
            if (VIP_TIERS[i].name == currentTier)
            {
                currentIndex = i;
                break;
            }
        }

        // Upgrade available
        if (currentIndex >= 0 && currentIndex < NUM_TIERS - 1)
        {
            const VIPTier& currentVIP = VIP_TIERS[currentIndex];
            const VIPTier& nextVIP = VIP_TIERS[currentIndex + 1];

            double upgradePrice = nextVIP.price - currentVIP.price;

            cout << "\nYou can upgrade to: "
                 << nextVIP.name << " VIP\n";

            cout << "Upgrade Price: RM "
                 << fixed << setprecision(2)
                 << upgradePrice << "\n";

            cout << "\nUpgrade now? (Y/N): ";

            string upgrade;
            getline(cin, upgrade);

            // Cancel upgrade if user does not confirm
            if (upgrade.empty() ||
                toupper(static_cast<unsigned char>(upgrade[0])) != 'Y')
            {
                cout << "\nUpgrade cancelled.\n";
                cout << "====================================\n";
                EnterToContinue();
                return;
            }

            // Process payment
            cout << "\nProcessing upgrade...\n";
            cout << "Payment successful!\n";

            // Load existing memberships
            vector<VIPMembership> memberships = loadVIPMemberships();

            // Deactivate old membership
            for (VIPMembership& m : memberships)
            {
                if (m.customerId == customerId && m.isActive)
                {
                    m.isActive = false;
                }
            }

            // Create new membership
            VIPMembership membership;

            membership.customerId = customerId;
            membership.tier = nextVIP.name;
            membership.purchaseDate = getCurrentSystemDate();
            membership.expiryDate =
                addMonths(membership.purchaseDate,
                          nextVIP.durationMonths);
            membership.price = upgradePrice;
            membership.isActive = true;

            memberships.push_back(membership);

            // Save updated membership data
            saveAllVIPMemberships(memberships);

            cout << "\n  CONGRATULATIONS!\n";
            cout << "You've been upgraded to "
                 << membership.tier << " VIP!\n";
            cout << "Valid until: "
                 << formatDate(membership.expiryDate) << "\n";
            cout << "====================================\n";

            EnterToContinue();
            return;
        }
        else
        {
            // Customer already has the highest tier
            cout << "\nYou already have the highest VIP tier!\n";
            cout << "Thank you for being a Platinum VIP member!\n";
            cout << "====================================\n";

            EnterToContinue();
            return;
        }
    }

    // Show available VIP tiers
    cout << "\nAvailable VIP Tiers:\n";
    cout << string(50, '-') << "\n";

    for (int i = 0; i < NUM_TIERS; i++)
    {
        cout << "[" << (i + 1) << "] "
             << VIP_TIERS[i].name << " VIP\n";

        cout << "    Price: RM "
             << fixed << setprecision(2)
             << VIP_TIERS[i].price << "\n";

        cout << "    Duration: "
             << VIP_TIERS[i].durationMonths
             << " months\n";

        cout << string(50, '-') << "\n";
    }

    cout << "[0] Cancel\n";

    int choice = readInteger("Select tier: ", 0, NUM_TIERS);

    // Cancel purchase
    if (choice == 0)
    {
        cout << "\nPurchase cancelled.\n";
        cout << "====================================\n";
        EnterToContinue();
        return;
    }

    int tierIndex = choice - 1;
    const VIPTier& selectedTier = VIP_TIERS[tierIndex];

    // Confirm purchase
    cout << "\nYou are about to purchase: "
         << selectedTier.name << " VIP\n";

    cout << "  Price: RM "
         << fixed << setprecision(2)
         << selectedTier.price << "\n";

    cout << "  Duration: "
         << selectedTier.durationMonths
         << " months\n";

    cout << "\nConfirm purchase? (Y/N): ";

    string confirm;
    getline(cin, confirm);

    // Cancel if the user does not confirm
    if (confirm.empty() ||
        toupper(static_cast<unsigned char>(confirm[0])) != 'Y')
    {
        cout << "\nPurchase cancelled.\n";
        cout << "====================================\n";
        EnterToContinue();
        return;
    }

    // Process payment
    cout << "\nProcessing payment...\n";
    cout << "Please enter your payment details:\n";

    string card;
    string expiry;
    string cvv;
    bool validPayment = false;

    // Validate payment details
    while (!validPayment)
    {
        // Card number
        cout << "\nCard Number (XXXX-XXXX-XXXX-XXXX): ";
        getline(cin, card);

        if (card.length() != 19 ||
            card[4] != '-' ||
            card[9] != '-' ||
            card[14] != '-')
        {
            cout << "Invalid card number format.\n";
            cout << "Please use XXXX-XXXX-XXXX-XXXX.\n";
            continue;
        }

        bool validCard = true;

        // Check card number contains only digits and hyphens
        for (int i = 0; i < 19; i++)
        {
            if (i == 4 || i == 9 || i == 14)
                continue;

            if (!isdigit(static_cast<unsigned char>(card[i])))
            {
                validCard = false;
                break;
            }
        }

        if (!validCard)
        {
            cout << "Invalid card number. Please enter digits only.\n";
            continue;
        }

        // Expiry date
        cout << "Expiry (MM/YY): ";
        getline(cin, expiry);

        if (expiry.length() != 5 ||
            expiry[2] != '/' ||
            !isdigit(static_cast<unsigned char>(expiry[0])) ||
            !isdigit(static_cast<unsigned char>(expiry[1])) ||
            !isdigit(static_cast<unsigned char>(expiry[3])) ||
            !isdigit(static_cast<unsigned char>(expiry[4])))
        {
            cout << "Invalid expiry format.\n";
            cout << "Please use MM/YY.\n";
            continue;
        }

        int month = stoi(expiry.substr(0, 2));

        // Validate expiry month
        if (month < 1 || month > 12)
        {
            cout << "Invalid expiry month.\n";
            continue;
        }

        // CVV
        cout << "CVV: ";
        getline(cin, cvv);

        if (cvv.length() != 3)
        {
            cout << "Invalid CVV. CVV must contain 3 digits.\n";
            continue;
        }

        bool validCVV = true;

        // Check CVV contains only digits
        for (char c : cvv)
        {
            if (!isdigit(static_cast<unsigned char>(c)))
            {
                validCVV = false;
                break;
            }
        }

        if (!validCVV)
        {
            cout << "Invalid CVV. Please enter 3 digits.\n";
            continue;
        }

        validPayment = true;
    }

    cout << "\nPayment successful!\n";

    // Create membership
    VIPMembership membership;

    membership.customerId = customerId;
    membership.tier = selectedTier.name;
    membership.purchaseDate = getCurrentSystemDate();
    membership.expiryDate =
        addMonths(membership.purchaseDate,
                  selectedTier.durationMonths);
    membership.price = selectedTier.price;
    membership.isActive = true;

    saveVIPMembership(membership);

    // Display successful membership purchase
    cout << "\nCONGRATULATIONS!\n";
    cout << "You are now a "
         << selectedTier.name << " VIP member!\n";

    cout << "Your membership is valid until: "
         << formatDate(membership.expiryDate) << "\n";

    cout << "\nEnjoy your exclusive benefits!\n";
    cout << "====================================\n";

    EnterToContinue();
}

// ============================================================
// VIEW VIP BENEFITS
// ============================================================

// Display the benefits of each VIP tier
void viewVIPBenefits(const string& customerId)
{
    clearScreen();

    cout << "\n====================================\n";
    cout << "        VIP TIER BENEFITS\n";
    cout << "====================================\n";

    // Display all VIP tier information
    for (int i = 0; i < NUM_TIERS; i++)
    {
        const VIPTier& tier = VIP_TIERS[i];

        cout << string(48, '-') << "\n";
        cout << tier.name << " VIP\n";
        cout << string(48, '-') << "\n";

        cout << "Price: RM "
             << fixed << setprecision(2)
             << tier.price << "\n";

        cout << "Duration: "
             << tier.durationMonths << " months\n";

        cout << "Benefits:\n";
        cout << tier.benefits;

        cout << string(48, '-') << "\n";
    }

    // Display the current customer's VIP status
    if (!customerId.empty())
    {
        string currentTier = getVIPStatus(customerId);

        cout << "\nYour current status: ";

        if (currentTier == "None")
        {
            cout << "No active VIP membership\n";
            cout << "Purchase one today to enjoy exclusive benefits!\n";
        }
        else
        {
            cout << currentTier << " VIP Member\n";
            cout << "Enjoy your exclusive benefits!\n";
        }
    }

    cout << "====================================\n";

    EnterToContinue();
}