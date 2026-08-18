#pragma once

#include "Constants.h"

struct SimParams {
    double H0 = 67.4;
    double Omega_m = 0.315;
    double Omega_L = 0.685;
    double Omega_r = 9.2e-5;
    double w = -1.0;
    double playSpeedMs = 350.0;

    double Omega_k() const {
        return 1.0 - Omega_m - Omega_L - Omega_r;
    }

    double H0_per_sec() const {
        return H0 * 1000.0 / MPC_IN_METERS;
    }
};