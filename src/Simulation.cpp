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

#include "Simulation.h"
#include "Cosmology.h"
#include "Timeline.h"
#include "Formatting.h"
#include "UI.h"

// ----------------------------------------------------------------------------
// Simulation runner
// ----------------------------------------------------------------------------
void runSimulation(SimParams& p, bool startAtHumans) {
    std::cout << "\nBuilding cosmology model for your parameters...\n";
    Cosmology cosmo;
    cosmo.build(p);
    std::cout << "Done. Computed age of the universe: " << formatTime(cosmo.t0Seconds) << "\n";

    std::vector<Event> timeline = buildBaseTimeline();
    std::vector<Event> future = buildFutureTimeline(cosmo.t0Seconds);
    timeline.insert(timeline.end(), future.begin(), future.end());

    size_t startIndex = 0;
    if (startAtHumans) {
        for (size_t i = 0; i < timeline.size(); ++i) {
            if (timeline[i].title == "Homo Sapiens Appear") { startIndex = i; break; }
        }
        std::cout << "\n(Fast-forwarding through " << formatTime(timeline[startIndex].tSeconds)
                  << " of cosmic history: Big Bang, star and galaxy formation, the solar\n"
                     "system, and the emergence of life on Earth, to reach this point.)\n";
    }

    size_t idx = startIndex;
    bool running = true;
    bool first = true;

    while (running) {
        if (idx < timeline.size()) {
            printEventStats(timeline[idx], cosmo, p);
        } else {
            printDivider();
            std::cout << "You have reached the end of the charted timeline.\n";
            printFate(cosmo.fate);
        }

        if (idx >= timeline.size()) {
            std::cout << "\n[m] main menu   [f] fate details   [p] params   [q] quit program\n> ";
            std::string cmd;
            std::getline(std::cin, cmd);
            if (cmd == "q") { std::exit(0); }
            if (cmd == "f") { printFate(cosmo.fate); continue; }
            if (cmd == "p") { printParams(p); continue; }
            return; // back to main menu
        }

        std::cout << "\n[Enter] next event   [a] auto-play   [j <N><unit>] jump ahead\n"
                     "   (units: s, min, hr, day, yr, kyr, myr, gyr)\n"
                     "[f] fate   [p] params   [m] main menu   [q] quit\n> ";
        std::string cmd;
        std::getline(std::cin, cmd);

        if (cmd.empty()) {
            idx++;
        } else if (cmd == "q") {
            std::exit(0);
        } else if (cmd == "m") {
            return;
        } else if (cmd == "f") {
            printFate(cosmo.fate);
        } else if (cmd == "p") {
            printParams(p);
        } else if (cmd == "a") {
            std::cout << "\nAuto-playing through remaining events (press Ctrl+C to stop)...\n";
            for (size_t i = idx; i <= timeline.size(); ++i) {
                if (i < timeline.size()) {
                    printEventStats(timeline[i], cosmo, p);
                } else {
                    printDivider();
                    std::cout << "You have reached the end of the charted timeline.\n";
                    printFate(cosmo.fate);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds((int)p.playSpeedMs));
            }
            idx = timeline.size();
        } else if (cmd.size() > 2 && cmd[0] == 'j') {
            // parse "j <number><unit>"
            std::istringstream iss(cmd.substr(1));
            double val; std::string unit;
            iss >> val >> unit;
            double factor = SEC_PER_YEAR;
            if (unit == "s") factor = 1.0;
            else if (unit == "min") factor = 60.0;
            else if (unit == "hr") factor = 3600.0;
            else if (unit == "day") factor = 86400.0;
            else if (unit == "yr") factor = SEC_PER_YEAR;
            else if (unit == "kyr") factor = SEC_PER_YEAR * 1e3;
            else if (unit == "myr") factor = SEC_PER_YEAR * 1e6;
            else if (unit == "gyr") factor = SEC_PER_YEAR * 1e9;
            else { std::cout << "  (unrecognized unit, try s/min/hr/day/yr/kyr/myr/gyr)\n"; continue; }

            double currentT = (idx < timeline.size()) ? timeline[idx].tSeconds : timeline.back().tSeconds;
            double targetT = currentT + val * factor;

            // find nearest event at or after targetT, but show a synthetic snapshot too
            Event snap;
            snap.tSeconds = targetT;
            snap.title = "Snapshot";
            snap.desc = "A snapshot of the universe at this moment in time (not a named\nhistorical milestone).";
            printEventStats(snap, cosmo, p);

            size_t newIdx = idx;
            while (newIdx < timeline.size() && timeline[newIdx].tSeconds < targetT) newIdx++;
            idx = newIdx;
        } else {
            std::cout << "  (unrecognized command)\n";
        }
        (void)first;
    }
}