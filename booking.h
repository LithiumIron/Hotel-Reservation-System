#pragma once

#include "models.h"
#include <vector>

std::vector<Booking> loadSavedBookings();

void bookingScreen();
void viewPreviousBookings();
void cancelBooking();