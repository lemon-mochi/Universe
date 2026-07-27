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
//  Build:   g++ -O2 -std=c++17 -o universe_sim universe_sim.cpp
//  Run:     ./universe_sim
// ============================================================================

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

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------
static const double SEC_PER_YEAR = 31557600.0;              // Julian year
static const double MPC_IN_METERS = 3.0856775814913673e22;  // meters per Mpc
static const double T_CMB0 = 2.725;                         // K, today's CMB temp

// ----------------------------------------------------------------------------
// Simulation parameters (user-adjustable)
// ----------------------------------------------------------------------------
struct SimParams {
    double H0        = 67.4;    // Hubble constant, km/s/Mpc
    double Omega_m    = 0.315;  // matter density parameter (incl. dark matter)
    double Omega_L    = 0.685;  // dark energy density parameter
    double Omega_r    = 9.2e-5; // radiation density parameter
    double w          = -1.0;   // dark energy equation of state (-1 = cosmological constant)
    double playSpeedMs = 350.0; // ms pause between events in auto-play mode

    double Omega_k() const { return 1.0 - Omega_m - Omega_L - Omega_r; }

    double H0_per_sec() const { return H0 * 1000.0 / MPC_IN_METERS; }
};

// ----------------------------------------------------------------------------
// Fate of the universe
// ----------------------------------------------------------------------------
enum class Fate { HEAT_DEATH, BIG_RIP, BIG_CRUNCH, UNKNOWN };

struct FateResult {
    Fate fate = Fate::UNKNOWN;
    double timeFromNowYears = -1.0; // finite time for RIP/CRUNCH, -1 if unbounded
};

// E(a)^2 = Omega_r a^-4 + Omega_m a^-3 + Omega_k a^-2 + Omega_L a^{-3(1+w)}
static double E2(double a, const SimParams& p) {
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
class Cosmology {
public:
    std::vector<double> tSeconds; // monotonically increasing
    std::vector<double> aVals;    // scale factor at each tSeconds entry
    double t0Seconds = 0.0;       // age of the universe "today" (a=1)
    FateResult fate;

    void build(const SimParams& p) {
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
    double a_of_t(double t) const {
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
};

// ----------------------------------------------------------------------------
// Timeline events (fixed real-world cosmic chronology; times are seconds
// since the Big Bang, computed from standard astrophysical estimates).
// ----------------------------------------------------------------------------
struct Event {
    double tSeconds;
    std::string title;
    std::string desc;
};

static double yr(double years) { return years * SEC_PER_YEAR; }

static std::vector<Event> buildBaseTimeline() {
    std::vector<Event> ev;
    ev.push_back({1e-43, "The Big Bang",
        "All of space, time, matter and energy begin expanding from an\n"
        "extraordinarily hot, dense state. Known physics breaks down before\n"
        "this point (the Planck epoch)."});
    ev.push_back({1e-36, "Cosmic Inflation",
        "A brief but colossal exponential expansion smooths and flattens the\n"
        "universe, stretching quantum fluctuations into the seeds of future\n"
        "galaxies."});
    ev.push_back({1e-32, "Inflation Ends / Reheating",
        "Inflation's energy converts into a hot soup of particles: the\n"
        "universe is filled with a quark-gluon plasma."});
    ev.push_back({1e-12, "Electroweak Symmetry Breaking",
        "The electromagnetic and weak nuclear forces separate into distinct\n"
        "forces; fundamental particles gain mass via the Higgs mechanism."});
    ev.push_back({1e-6, "Hadron Epoch Begins",
        "Quarks bind together into protons and neutrons as the universe\n"
        "cools below the quark confinement temperature."});
    ev.push_back({1.0, "Neutrino Decoupling",
        "Neutrinos stop interacting with other matter and stream freely\n"
        "through the universe -- the cosmic neutrino background is born."});
    ev.push_back({10.0, "Lepton Epoch Ends",
        "Electrons and positrons annihilate each other; only a small excess\n"
        "of electrons survives to later form atoms."});
    ev.push_back({180.0, "Big Bang Nucleosynthesis Begins",
        "The universe is now cool enough for protons and neutrons to fuse\n"
        "into light nuclei: hydrogen, helium, and traces of lithium."});
    ev.push_back({1020.0, "Big Bang Nucleosynthesis Ends",
        "The primordial abundances of light elements are locked in --\n"
        "roughly 75% hydrogen, 25% helium by mass."});
    ev.push_back({yr(47000.0), "Matter-Radiation Equality",
        "Matter overtakes radiation as the dominant component of the\n"
        "universe's energy density, allowing gravity to begin clumping\n"
        "matter together."});
    ev.push_back({yr(370000.0), "Recombination / Cosmic Microwave Background",
        "Electrons combine with nuclei to form neutral atoms. Light can\n"
        "finally travel freely -- this afterglow is observed today as the\n"
        "Cosmic Microwave Background."});
    ev.push_back({yr(370000.0) * 1.01, "The Dark Ages Begin",
        "With no stars yet formed, the universe is a dark expanse of\n"
        "neutral hydrogen and helium gas, slowly collapsing under gravity."});
    ev.push_back({yr(180e6), "The First Stars Ignite",
        "Population III stars -- the first stars -- ignite from primordial\n"
        "gas, ending the cosmic dark ages and flooding the universe with\n"
        "the first starlight."});
    ev.push_back({yr(400e6), "First Galaxies Form",
        "Gravity pulls gas and early star clusters together into the first\n"
        "small galaxies."});
    ev.push_back({yr(1.0e9), "Reionization Complete",
        "Ultraviolet light from stars and quasars has re-ionized nearly all\n"
        "of the hydrogen gas in the universe."});
    ev.push_back({yr(3.7e9), "The Milky Way Takes Shape",
        "Our home galaxy's early disk begins forming from merging\n"
        "protogalactic clumps."});
    ev.push_back({yr(9.2e9), "The Solar System Forms",
        "A giant molecular cloud collapses, forming the Sun and, from the\n"
        "surrounding protoplanetary disk, the planets -- including Earth."});
    ev.push_back({yr(10.2e9), "First Life on Earth",
        "Simple single-celled microorganisms appear in Earth's oceans, the\n"
        "earliest known life in the universe."});
    ev.push_back({yr(12.6e9), "The Great Oxidation Event",
        "Photosynthetic microbes flood Earth's atmosphere with oxygen,\n"
        "transforming the planet's chemistry and enabling complex life."});
    ev.push_back({yr(13.26e9), "The Cambrian Explosion",
        "A rapid diversification of complex, multicellular animal life\n"
        "occurs in Earth's oceans."});
    ev.push_back({yr(13.75e9), "Age of Dinosaurs",
        "Dinosaurs rise to dominate life on Earth."});
    ev.push_back({yr(13.7935e9), "Dinosaur Extinction",
        "An asteroid impact (and/or massive volcanism) wipes out the\n"
        "non-avian dinosaurs, opening the way for mammals to diversify."});
    ev.push_back({yr(13.7997e9), "Homo Sapiens Appear",
        "Anatomically modern humans emerge in Africa -- the first beings in\n"
        "the observable universe capable of asking how it all began."});
    ev.push_back({yr(13.797e9), "The Present Day",
        "You are here. The universe is about 13.8 billion years old, still\n"
        "expanding, and (currently) accelerating due to dark energy."});
    return ev;
}

// Future events relative to the present (t0), independent of exact fate.
static std::vector<Event> buildFutureTimeline(double t0Seconds) {
    std::vector<Event> ev;
    ev.push_back({t0Seconds + yr(4.5e9), "Andromeda Collision",
        "The Andromeda Galaxy collides and begins merging with the Milky\n"
        "Way, eventually forming a single elliptical galaxy."});
    ev.push_back({t0Seconds + yr(5.0e9), "The Sun Becomes a Red Giant",
        "Having exhausted hydrogen in its core, the Sun swells into a red\n"
        "giant, likely engulfing Mercury and Venus and rendering Earth\n"
        "uninhabitable."});
    ev.push_back({t0Seconds + yr(5.4e9), "The Sun Becomes a White Dwarf",
        "The Sun sheds its outer layers as a planetary nebula, leaving\n"
        "behind a slowly cooling white dwarf remnant."});
    ev.push_back({t0Seconds + yr(1.0e14), "Star Formation Ceases",
        "The universe's supply of star-forming gas is exhausted. The last\n"
        "red dwarf stars will keep burning for trillions of years more, but\n"
        "no new stars will be born -- the Degenerate Era begins."});
    ev.push_back({t0Seconds + yr(1.0e20), "Galaxies Disperse",
        "Gravitational encounters gradually eject most stars from galaxies\n"
        "or send them spiraling into central supermassive black holes."});
    ev.push_back({t0Seconds + yr(1.0e37), "(Hypothetical) Proton Decay",
        "If protons are unstable (unconfirmed), ordinary matter -- planets,\n"
        "dead stars, stellar remnants -- gradually disintegrates into\n"
        "radiation, marking the transition to the Black Hole Era."});
    ev.push_back({t0Seconds + yr(1.0e100), "Black Holes Evaporate",
        "Via Hawking radiation, even the largest supermassive black holes\n"
        "finally evaporate, leaving behind a cold, dilute bath of photons\n"
        "and particles: the Dark Era."});
    return ev;
}

// ----------------------------------------------------------------------------
// Formatting helpers
// ----------------------------------------------------------------------------
static std::string formatTime(double seconds) {
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

static std::string formatAgeSince(double seconds, double t0) {
    double diff = seconds - t0;
    if (std::fabs(diff) < 1.0) return "now";
    if (diff < 0) return formatTime(-diff) + " before present";
    return formatTime(diff) + " from now";
}

// ----------------------------------------------------------------------------
// UI helpers
// ----------------------------------------------------------------------------
static void printDivider() {
    std::cout << "----------------------------------------------------------------------\n";
}

static void printEventStats(const Event& e, const Cosmology& cosmo, const SimParams& p, bool showHeader = true) {
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

static void printFate(const FateResult& f) {
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

static void printParams(const SimParams& p) {
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

// Prompt for a double, keeping the current value if the user just presses Enter.
static double promptDouble(const std::string& label, double current) {
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

static void configureParams(SimParams& p) {
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
// Simulation runner
// ----------------------------------------------------------------------------
static void runSimulation(SimParams& p, bool startAtHumans) {
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

// ----------------------------------------------------------------------------
// Main menu
// ----------------------------------------------------------------------------
static void printWelcome() {
    std::cout <<
        "========================================================================\n"
        "                          UNIVERSE SIMULATOR\n"
        "========================================================================\n"
        "A terminal-based, parameter-driven simulation of the universe's history,\n"
        "from the Big Bang to its ultimate fate -- built on a real (simplified)\n"
        "numerical integration of the Friedmann equation.\n";
}

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