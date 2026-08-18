#pragma once

#include "Cosmology.h"
#include "Event.h"
#include "SimParams.h"

#include <string>

std::string formatTime(double seconds);

std::string formatAgeSince(double seconds, double t0);

void printEventStats(
    const Event& event,
    const Cosmology& cosmo,
    const SimParams& params,
    bool showHeader = true
);

void printFate(const FateResult& fate);

void printParams(const SimParams& params);