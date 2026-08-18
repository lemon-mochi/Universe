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

#include "Formatting.h"
#include "UI.h"

// ----------------------------------------------------------------------------
// Formatting helpers
// ----------------------------------------------------------------------------
std::string formatTime(double seconds) {
    std::ostringstream os;
    os << std::setprecision(4);
    double s = seconds;
    if (s < 0) { os << "0 s"; return os.str(); }
    if (s < 60.0) { os << s << " s"; return os.str(); }
    if (s < 3600.0) { os << (s / 60.0) << " min"; return os.str(); }
    if (s < 86400.0) { os << (s / 3600.0) << " hours"; return os.str(); }
    double years = s / SEC_PER_YEAR;
    if (years < 1.0) { os << (s / 86400.0) << " days"; return os.str(); }
    if (years < 1e3)  { os << years << " years"; return os.str(); }
    if (years < 1e6)  { os << (years / 1e3) << " thousand years"; return os.str(); }
    if (years < 1e9)  { os << (years / 1e6) << " million years"; return os.str(); }
    if (years < 1e12) { os << (years / 1e9) << " billion years"; return os.str(); }
    if (years < 1e15) { os << (years / 1e12) << " trillion years"; return os.str(); }
    os << std::scientific << std::setprecision(3) << years << " years";
    return os.str();
}

std::string formatAgeSince(double seconds, double t0) {
    double diff = seconds - t0;
    if (std::fabs(diff) < 1.0) return "now";
    if (diff < 0) return formatTime(-diff) + " before present";
    return formatTime(diff) + " from now";
}

void printEventStats(const Event& e, const Cosmology& cosmo, const SimParams& p, bool showHeader) {
    double a = cosmo.a_of_t(e.tSeconds);
    double z = (a > 0) ? (1.0 / a - 1.0) : std::numeric_limits<double>::infinity();
    double H0s = p.H0_per_sec();
    double H = H0s * std::sqrt(std::max(E2(a, p), 0.0));
    double H_kmsMpc = H * MPC_IN_METERS / 1000.0;
    double T = T_CMB0 / a;

    if (showHeader) {
        printDivider();
        std::cout << ">> " << e.title << "\n";
        printDivider();
    }
    std::cout << e.desc << "\n\n";
    std::cout << std::left << std::setw(28) << "  Cosmic time:"      << formatTime(e.tSeconds)
              << "  (" << formatAgeSince(e.tSeconds, cosmo.t0Seconds) << ")\n";
    std::cout << std::left << std::setw(28) << "  Scale factor a:"   << std::scientific << std::setprecision(4) << a << "\n";
    std::cout << std::left << std::setw(28) << "  Redshift z:"       << std::scientific << std::setprecision(4) << z << "\n";
    std::cout << std::left << std::setw(28) << "  Hubble parameter:" << std::fixed << std::setprecision(3) << H_kmsMpc << " km/s/Mpc\n";
    std::cout << std::left << std::setw(28) << "  Approx. temperature:" << std::scientific << std::setprecision(4) << T << " K\n";
    std::cout << std::defaultfloat;
}

void printFate(const FateResult& f) {
    printDivider();
    std::cout << "ULTIMATE FATE OF THE UNIVERSE (given current parameters)\n";
    printDivider();
    switch (f.fate) {
        case Fate::HEAT_DEATH:
            std::cout << "Heat Death / Big Freeze: expansion continues forever. Dark energy\n"
                         "keeps accelerating the expansion, matter thins out, stars burn out,\n"
                         "and the universe asymptotically approaches maximum entropy -- a cold,\n"
                         "dark, near-empty state. There is no finite end time.\n";
            break;
        case Fate::BIG_RIP:
            std::cout << "Big Rip: your dark energy equation of state (w < -1, \"phantom\"\n"
                         "energy) causes the dark energy density to grow without bound. The\n"
                         "expansion accelerates so violently that it eventually tears apart\n"
                         "galaxies, stars, planets, and finally atoms themselves.\n"
                         "Approx. time from now: " << formatTime(f.timeFromNowYears * SEC_PER_YEAR) << "\n";
            break;
        case Fate::BIG_CRUNCH:
            std::cout << "Big Crunch: your parameters give too little dark energy (and/or\n"
                         "enough positive spatial curvature / matter) for expansion to continue\n"
                         "forever. Expansion will eventually halt and reverse, with the universe\n"
                         "collapsing back down toward a hot, dense state.\n"
                         "Approx. time from now: " << formatTime(f.timeFromNowYears * SEC_PER_YEAR) << " (rough estimate)\n";
            break;
        default:
            std::cout << "Undetermined.\n";
    }
    printDivider();
}

void printParams(const SimParams& p) {
    printDivider();
    std::cout << "CURRENT PARAMETERS\n";
    printDivider();
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "  H0 (Hubble constant):        " << p.H0 << " km/s/Mpc\n";
    std::cout << "  Omega_matter:                " << p.Omega_m << "\n";
    std::cout << "  Omega_dark_energy:           " << p.Omega_L << "\n";
    std::cout << "  Omega_radiation:             " << p.Omega_r << "\n";
    std::cout << "  Omega_curvature (derived):   " << p.Omega_k()
              << (std::fabs(p.Omega_k()) < 1e-4 ? "  (flat)" :
                  (p.Omega_k() > 0 ? "  (open)" : "  (closed)")) << "\n";
    std::cout << "  Dark energy w:               " << p.w
              << (p.w < -1.0 ? "  (phantom energy)" : (p.w == -1.0 ? "  (cosmological constant)" : "  (quintessence-like)")) << "\n";
    std::cout << "  Auto-play pace:              " << p.playSpeedMs << " ms/event\n";
    printDivider();
}
