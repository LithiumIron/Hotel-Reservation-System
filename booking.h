#pragma once

#include "models.h"
#include <vector>

std::vector<Booking> loadSavedBookings();
void saveAllBookings(const std::vector<Booking>& allBookings);

void bookingScreen();
void viewPreviousBookings();
void cancelBooking();