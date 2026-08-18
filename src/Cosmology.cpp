#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <limits>
#include <algorithm>
#include <thread>
#include <chrono>
#include <cstdlib>

#include "Cosmology.h"
#include "SimParams.h"

// E(a)^2 = Omega_r a^-4 + Omega_m a^-3 + Omega_k a^-2 + Omega_L a^{-3(1+w)}
double E2(double a, const SimParams& p) {
    double ok = p.Omega_k();
    double de_exp = -3.0 * (1.0 + p.w);
    return p.Omega_r * std::pow(a, -4.0)
         + p.Omega_m * std::pow(a, -3.0)
         + ok        * std::pow(a, -2.0)
         + p.Omega_L * std::pow(a, de_exp);
}

// ----------------------------------------------------------------------------
// Cosmology engine: builds a lookup table of (time since Big Bang in seconds)
// vs (scale factor a), by integrating dt/d(ln a) = 1 / H(a).
// ----------------------------------------------------------------------------


void Cosmology::build(const SimParams& p) {
    tSeconds.clear();
    aVals.clear();
    double H0s = p.H0_per_sec();

    // ---- Past: integrate from a_start (deep radiation era) up to a=1 ----
    const double aStart = 1e-10;
    const int    Npast  = 30000;

    double tAtStart;
    if (p.Omega_r > 0) {
        // Radiation-domination analytic approx: t = a^2 / (2 H0 sqrt(Omega_r))
        tAtStart = (aStart * aStart) / (2.0 * H0s * std::sqrt(p.Omega_r));
    } else {
        tAtStart = 1e-20; // fallback, negligible
    }

    double xStart = std::log(aStart);
    double xEnd   = 0.0; // ln(1) = 0
    double dx     = (xEnd - xStart) / Npast;

    double t = tAtStart;
    double x = xStart;
    tSeconds.push_back(t);
    aVals.push_back(std::exp(x));

    for (int i = 0; i < Npast; ++i) {
        double a1 = std::exp(x);
        double a2 = std::exp(x + dx);
        double H1 = H0s * std::sqrt(std::max(E2(a1, p), 1e-300));
        double H2 = H0s * std::sqrt(std::max(E2(a2, p), 1e-300));
        // dt/dx = 1/H(a) -> trapezoidal rule
        double dt = 0.5 * (1.0 / H1 + 1.0 / H2) * dx;
        t += dt;
        x += dx;
        tSeconds.push_back(t);
        aVals.push_back(std::exp(x));
    }
    t0Seconds = t; // time at a = 1, i.e. today

    // ---- Future: integrate from a=1 forward, watching for Crunch/Rip ----
    const double aFutureCap = 1e8;
    const int    Nfuture    = 30000;
    double xF0 = 0.0;
    double xF1 = std::log(aFutureCap);
    double dxF = (xF1 - xF0) / Nfuture;

    double tf = t0Seconds;
    double xf = xF0;
    bool crunched = false;
    double crunchTurnaroundT = -1, crunchTurnaroundA = -1;

    // track increments to detect convergence (Big Rip) vs divergence (Heat Death)
    double tAt_1e4 = -1, tAt_1e6 = -1, tAt_1e8 = -1;

    for (int i = 0; i < Nfuture; ++i) {
        double a1 = std::exp(xf);
        double e2_1 = E2(a1, p);
        if (e2_1 <= 0.0) {
            crunched = true;
            crunchTurnaroundT = tf;
            crunchTurnaroundA = a1;
            break;
        }
        double a2 = std::exp(xf + dxF);
        double e2_2 = E2(a2, p);
        if (e2_2 <= 0.0) {
            crunched = true;
            crunchTurnaroundT = tf;
            crunchTurnaroundA = a1;
            break;
        }
        double H1 = H0s * std::sqrt(e2_1);
        double H2 = H0s * std::sqrt(e2_2);
        double dt = 0.5 * (1.0 / H1 + 1.0 / H2) * dxF;
        tf += dt;
        xf += dxF;
        tSeconds.push_back(tf);
        aVals.push_back(std::exp(xf));

        double aNow = std::exp(xf);
        if (tAt_1e4 < 0 && aNow >= 1e4) tAt_1e4 = tf;
        if (tAt_1e6 < 0 && aNow >= 1e6) tAt_1e6 = tf;
        if (tAt_1e8 < 0 && aNow >= 1e8) tAt_1e8 = tf;
    }

    if (crunched) {
        fate.fate = Fate::BIG_CRUNCH;
        // crude symmetric-collapse approximation: total lifetime ~ 2x time to turnaround
        double lifetime = 2.0 * (crunchTurnaroundT);
        fate.timeFromNowYears = (lifetime - t0Seconds) / SEC_PER_YEAR;
    } else if (tAt_1e4 > 0 && tAt_1e6 > 0 && tAt_1e8 > 0) {
        double seg1 = tAt_1e6 - tAt_1e4; // time to grow a from 1e4 to 1e6
        double seg2 = tAt_1e8 - tAt_1e6; // time to grow a from 1e6 to 1e8
        if (seg1 > 0 && seg2 / seg1 < 0.15) {
            // increments shrinking fast -> converging to a finite time -> Big Rip
            fate.fate = Fate::BIG_RIP;
            fate.timeFromNowYears = (tAt_1e8 - t0Seconds) / SEC_PER_YEAR;
        } else {
            fate.fate = Fate::HEAT_DEATH;
            fate.timeFromNowYears = -1.0;
        }
    } else {
        // never reached a=1e4 within the integration cap -> extremely slow/eternal expansion
        fate.fate = Fate::HEAT_DEATH;
        fate.timeFromNowYears = -1.0;
    }
}

// interpolate scale factor a at a given cosmic time t (seconds since Big Bang)
double Cosmology::a_of_t(double t) const {
    if (t <= tSeconds.front()) return aVals.front();
    if (t >= tSeconds.back())  return aVals.back();
    auto it = std::upper_bound(tSeconds.begin(), tSeconds.end(), t);
    size_t i = std::distance(tSeconds.begin(), it);
    if (i == 0) return aVals.front();
    double t1 = tSeconds[i - 1], t2 = tSeconds[i];
    double a1 = aVals[i - 1], a2 = aVals[i];
    double frac = (t2 > t1) ? (t - t1) / (t2 - t1) : 0.0;
    // interpolate in log(a) for smoothness across huge dynamic range
    double la1 = std::log(std::max(a1, 1e-300));
    double la2 = std::log(std::max(a2, 1e-300));
    return std::exp(la1 + frac * (la2 - la1));
}
