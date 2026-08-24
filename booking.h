#pragma once

#include "models.h"
#include <vector>

std::vector<Booking> loadSavedBookings();
void saveAllBookings(const std::vector<Booking>& allBookings);

void bookingScreen(const std::string& customerId);
void viewPreviousBookings(const std::string& customerId);
void cancelBooking(const std::string& customerId);