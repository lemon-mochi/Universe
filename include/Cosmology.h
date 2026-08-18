#pragma once

#include <vector>

#include "SimParams.h"

enum class Fate {
    HEAT_DEATH,
    BIG_RIP,
    BIG_CRUNCH,
    UNKNOWN
};

struct FateResult {
    Fate fate = Fate::UNKNOWN;
    double timeFromNowYears = -1.0;
};

double E2(double a, const SimParams& p);

class Cosmology {
public:
    std::vector<double> tSeconds;
    std::vector<double> aVals;

    double t0Seconds = 0.0;
    FateResult fate;

    void build(const SimParams& p);

    double a_of_t(double t) const;
};