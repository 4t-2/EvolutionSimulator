# EvolutionSimulator

## What is this
This project was inspired by [Bibits](https://www.youtube.com/@TheBibitesDigitalLife) and [this video](https://www.youtube.com/watch?v=N3tRFayqVtk). It creates some creatures with simulated brains (with a NEAT like neural network) that live and die with the successfull ones (living long enough to reproduce) being able to pass down their genetic information to the next generation and so on.

## How to compile
Run the makefile (currently linux only)

## TODO
1. Create a system for creating neural networks [DONE]
	- Variable amounts of hidden neurons
	- Variable amounts of neuron connections
	- Lots of different sense neuron types (e.g position in the world)
	- Lots of different action neuron types (e.g move foward)

2. Create a way of storing the network [DONE]
	- able to grow in size as needed
	- store the connections
	first part is the starting neuron (has to either be a sense or hidden neuron)
	second part is the ending neuron (has to either be a action of hidden neuron)
	third is the weight (store as an int but divide it to be a value from -1 to 1)

3. Create a way of controling creatures with the neural network [DONE]
	- Simple circle creatures, nothing complex yet

4. Mutate and crossbreed different creatures so they can adapt [DONE]

5. Have constant supply of plants growing
	- Plants dont need to be able to evolve yet
	- Temporary as will be replaced by a better system

5. Auto create a tree diagram of who the creatures are related to
	- Each creature has a "name" that is just a long list of numbers that are its parents (e.g a "name" of 12,42,15 means a creature 12 is the third generation and related to creature 42 which is related to creature 15), similar to a linked list
	- along side its name its genetic information is also stored
	- has to be quick to write data to a file to prevent stuttering

7. Allow for evolving of different body parts
	- Evolve bones that are attached together with joints that are moved with muscles
