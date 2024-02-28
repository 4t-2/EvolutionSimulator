# EvolutionSimulator 1.2

## What is this
This project was inspired by [Bibits](https://www.youtube.com/@TheBibitesDigitalLife) and [this video](https://www.youtube.com/watch?v=N3tRFayqVtk). It creates some creatures with simulated brains (with a NEAT like neural network) that live and die with the successfull ones (living long enough to reproduce) being able to pass down their genetic information to the next generation and so on.

## How to compile
Run
```bash
# download libraries - only needed to be done once
cd lib
./updateAGL.sh
./updatePHY.sh
./updateIN.sh
cd ..

# compile
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

```

## Roadmap of stuff done
1.0 - Basics, Initial release <br>
1.1 - Predation, digestion and tweaks <br>
1.1.1 - Windows port, UI overhaul <br>
1.1.2 - AI Overhaul (Policy Gradients) <br>
1.1.3 - The Big Optimise/Optimize <br>
1.2 - Body overhaul<br>

## TODO
- Add a tree diagram of how the creatures evolved
- Evolving plants (probably make plants evolve into creatures)
- Saving and loading
- Add a help menu or something to make understanding the UI easier (tooltip on hover?)
