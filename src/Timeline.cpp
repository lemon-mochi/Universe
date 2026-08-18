#include "Timeline.h"
#include "Constants.h"
#include "Event.h"

// ----------------------------------------------------------------------------
// Timeline events (fixed real-world cosmic chronology; times are seconds
// since the Big Bang, computed from standard astrophysical estimates).
// ----------------------------------------------------------------------------

double yr(double years) { return years * SEC_PER_YEAR; }

std::vector<Event> buildBaseTimeline() {
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
std::vector<Event> buildFutureTimeline(double t0Seconds) {
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