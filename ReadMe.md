# Universe Simulator #
Program written in C++ to simulate the universe.
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
