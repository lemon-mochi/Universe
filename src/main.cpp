// ============================================================================
//  UNIVERSE SIMULATOR
//  A terminal-based cosmological simulator.
//
//  Under the hood this integrates a simplified Friedmann equation for a flat
//  (or user-curved) FLRW universe to get a real, parameter-dependent scale
//  factor a(t), Hubble rate, redshift and temperature. Early-universe and
//  astrophysical milestones (nucleosynthesis, recombination, first stars,
//  solar system formation, life, humans, etc.) are placed at their standard
//  real-world cosmic times, since deriving *those* from first principles is
//  far beyond a hobby simulator. The long-term FATE of the universe (heat
//  death / Big Rip / Big Crunch), however, IS derived numerically from
//  whatever parameters you choose.
//
// ============================================================================

#include <iostream>
#include <string>

#include "SimParams.h"
#include "Simulation.h"
#include "UI.h"
#include "Formatting.h"
#include "Timeline.h"


int main() {
    SimParams params;
    printWelcome();

    while (true) {
        printDivider();
        std::cout << "MAIN MENU\n";
        printDivider();
        std::cout << "  1) Start simulation from the Big Bang\n"
                     "  2) Start simulation from when humans first appear\n"
                     "  3) Configure parameters\n"
                     "  4) View current parameters\n"
                     "  5) Quit\n"
                     "> ";
        std::string choice;
        std::getline(std::cin, choice);

        if (choice == "1") {
            runSimulation(params, false);
        } else if (choice == "2") {
            runSimulation(params, true);
        } else if (choice == "3") {
            configureParams(params);
        } else if (choice == "4") {
            printParams(params);
        } else if (choice == "5" || choice == "q") {
            std::cout << "Goodbye.\n";
            break;
        } else {
            std::cout << "  (unrecognized choice)\n";
        }
    }
    return 0;
}