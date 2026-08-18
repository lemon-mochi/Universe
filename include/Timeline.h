#pragma once

#include <vector>

#include "Event.h"

std::vector<Event> buildBaseTimeline();
std::vector<Event> buildFutureTimeline(double t0Seconds);