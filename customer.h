
#pragma once

bool signup(std::string& loggedInUser);

void custHomeScreen(std::string& loggedInUser);
void viewCustomerProfile(std::string& loggedInUser);
void editCustomerProfile(std::string& loggedInUser);

void displayCustomerInfo(const std::string& loggedInUser);
void displayVIPStatus(const std::string& loggedInUser);

void displayBookingStatistics(const std::string& loggedInUser);
void displayBookingHistory(const std::string& loggedInUser);

void displayProfileMenu(std::string& loggedInUser);