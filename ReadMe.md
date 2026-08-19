# Universe Simulator #
Program written in C++ to simulate the universe (only works in termianl. There is no frontend).
### File structure ###
```
universe-simulator/
├── include/ # header file
│   │
│   ├── Constants.h
│   │       Constants used throughout the simulator
│   │
│   ├── SimParams.h
│   │       Simulation configuration
│   │
│   ├── Cosmology.h
│   │       Friedmann equation, scale factor, fate
│   │
│   ├── Event.h
│   │       Event data structure
│   │
│   ├── Timeline.h
│   │       Timeline construction
│   │
│   ├── Formatting.h
│   │       Time/age formatting
│   │
│   ├── UI.h
│   │       Terminal UI
│   │
│   └── Simulation.h
│           Simulation execution
│
│
├── src/ # source file
│   │
│   ├── main.cpp
│   │       Main menu / program entry point
│   │
│   ├── Cosmology.cpp
│   │       Friedmann integration
│   │
│   ├── Timeline.cpp
│   │       Historical/future events
│   │
│   ├── Formatting.cpp
│   │       Time formatting
│   │
│   ├── UI.cpp
│   │       Terminal output/input
│   │
│   └── Simulation.cpp
│           Simulation loop
│
│
├── CMakeLists.txt # cmake file for compiling
├── .gitignore # file ignored by Git
├── ReadMe.md # this file
```
### Running the program ###
```
cmake -S . -B build # create build/ directory
cd build # move into build/ directory
make # compile the program
./universe_sim # run the C++ propgram
```
### The program ###
Initially, the terminal should look like this:
```
========================================================================
                          UNIVERSE SIMULATOR
========================================================================
A terminal-based, parameter-driven simulation of the universe's history,
from the Big Bang to its ultimate fate -- built on a real (simplified)
numerical integration of the Friedmann equation.
----------------------------------------------------------------------
MAIN MENU
----------------------------------------------------------------------
  1) Start simulation from the Big Bang
  2) Start simulation from when humans first appear
  3) Configure parameters
  4) View current parameters
  5) Quit
```
By pressing `3`, you can configure the parameters. Here are what the adjustable parameters are:\
Hubble constant H0 (km/s/Mpc) [67.4]\
Omega_matter (matter density, ~0.3 realistic) [0.315]\
Omega_dark_energy (~0.7 realistic) [0.685]\
Omega_radiation (~0.0001 realistic) [9.2e-05]\
Dark energy equation of state w (-1 = cosmological const;\
  w < -1 = phantom energy, can cause a Big Rip) [-1]\
Auto-play pace (milliseconds per event) [350]\