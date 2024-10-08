#pragma once

// sets the max FPS, set to 0 to disable
// disable vsync to go above monitor refresh rate
#define TARGETFPS 0
// sets if vysnc should be on
#define VSYNC true

// output av pop, size, speed, sight in a format interprettable be something
// like gnuplot
#define LOGCREATUREDATA

// food avoids other food, value is pressure multiplier
#define FOODPRESSURE 20

// food slowly drifts to a random location, value is max range that the location
// is
// #define ACTIVEFOOD 500

// food experiences drag, value is drag coeficient
#define FOODDRAG 0.0005f

// food experiences a pushback force at the border of the simulation
#define FOODBORDER

#define VITEDENS (1. / 144.)

#define THREADS	  1
#define EATRADIUS 20
#define DAMAGE	  4

#define FOODVOL	 40
#define MEATVOL	 60
#define LEACHVOL 1

#define FOODCAP

#define BITEDELAY 20

#define FOODENERGY 1
#define MEATENERGY 1

#define RAY_LENGTH 1000

#define CONSTANT_INPUT		0
#define X_INPUT				1
#define Y_INPUT				2
#define ROTATION_INPUT		3
#define SPEED_INPUT			4
#define FOOD_DISTANCE		5
#define FOOD_ROTATION		6
#define CREATURE_DISTANCE	7
#define CREATURE_ROTATION	8
#define ENERGY_INPUT		9
#define HEALTH_INPUT		10
#define LIFE_INPUT			11
#define MEAT_DISTANCE		12
#define MEAT_ROTATION		13
#define CREATURE_PREFERENCE 14

#define PREGNANCY_COST 1
#define METABOLISM	   1

#define MENU_PADDING 1

#define BASE_B_VALUE 63

#define WIDTH  1300
#define HEIGHT 700
//#define WIDTH  1920
//#define HEIGHT 1080

#define CLEARCOLOR \
	{              \
		20, 20, 30 \
	}

#define NETWORK_PADDING 20

#define EXTRA_BYTES 6

#define MENU_BORDERTHICKNESS  (float)6
#define MENU_DECORATIONHEIGHT (float)(4 + (MENU_BORDERTHICKNESS * 4))

#define MENU_BORDEREDGE (float)2

#define ENERGYCOSTMULTIPLIER 0.0003f

#define ENVTYPES ViteSeg, Food, Meat, JointConstraint
