# EvolutionSimulator 1.1

## What is this
This project was inspired by [Bibits](https://www.youtube.com/@TheBibitesDigitalLife) and [this video](https://www.youtube.com/watch?v=N3tRFayqVtk). It creates some creatures with simulated brains (with a NEAT like neural network) that live and die with the successfull ones (living long enough to reproduce) being able to pass down their genetic information to the next generation and so on.

## How to compile
Run
```bash
# download latest AGL - doesnt need to be ran every time
cd lib
./updateAGL.sh
cd ..

# compile
cmake -S . -B build
cmake --build build

```

## TODO
- Add a tree diagram of how the creatures evolved
- Evolving plants (probably make plants evolve into creatures)
- Evolving bodies (not just size, speed, etc but actual limbs and shape)
- Saving and loading
- Better UI
- Predation
- Better AI?
