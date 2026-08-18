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

# include "UI.h"

// ----------------------------------------------------------------------------
// UI helpers
// ----------------------------------------------------------------------------
void printDivider() {
    std::cout << "----------------------------------------------------------------------\n";
}

// Prompt for a double, keeping the current value if the user just presses Enter.
double promptDouble(const std::string& label, double current) {
    std::cout << label << " [" << current << "]: ";
    std::string line;
    std::getline(std::cin, line);
    if (line.empty()) return current;
    try {
        return std::stod(line);
    } catch (...) {
        std::cout << "  (invalid number, keeping " << current << ")\n";
        return current;
    }
}

void configureParams(SimParams& p) {
    std::cout << "\nLeave a field blank and press Enter to keep its current value.\n\n";
    p.H0 = promptDouble("Hubble constant H0 (km/s/Mpc)", p.H0);
    p.Omega_m = promptDouble("Omega_matter (matter density, ~0.3 realistic)", p.Omega_m);
    p.Omega_L = promptDouble("Omega_dark_energy (~0.7 realistic)", p.Omega_L);
    p.Omega_r = promptDouble("Omega_radiation (~0.0001 realistic)", p.Omega_r);
    std::cout << "\nNote: Omega_matter + Omega_dark_energy + Omega_radiation should sum to\n"
                 "~1.0 for a spatially flat universe. If they don't, the leftover becomes\n"
                 "spatial curvature (Omega_curvature = 1 - sum), which affects the fate.\n\n";
    p.w = promptDouble("Dark energy equation of state w (-1 = cosmological const;\n"
                        "  w < -1 = phantom energy, can cause a Big Rip)", p.w);
    p.playSpeedMs = promptDouble("Auto-play pace (milliseconds per event)", p.playSpeedMs);
    std::cout << "\nParameters updated.\n";
}

// ----------------------------------------------------------------------------
// Main menu
// ----------------------------------------------------------------------------
void printWelcome() {
    std::cout <<
        "========================================================================\n"
        "                          UNIVERSE SIMULATOR\n"
        "========================================================================\n"
        "A terminal-based, parameter-driven simulation of the universe's history,\n"
        "from the Big Bang to its ultimate fate -- built on a real (simplified)\n"
        "numerical integration of the Friedmann equation.\n";
}