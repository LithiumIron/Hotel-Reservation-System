#pragma once

#include "models.h"
#include <vector>
#include <string>

// VIP tiers and pricing
struct VIPTier
{
    string name;
    double price;
    string benefits;
    int durationMonths;  // How many months the membership lasts
};

// Function declarations
void viewVIPMembership();
void purchaseVIPMembership();
void viewVIPBenefits();
bool hasActiveVIP(const std::string& customerId);
std::string getVIPStatus(const std::string& customerId);
void saveVIPMembership(const VIPMembership& membership);
std::vector<VIPMembership> loadVIPMemberships();
void applyVIPDiscount(double& total, const std::string& customerId);