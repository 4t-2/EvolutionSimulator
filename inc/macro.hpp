#pragma once

// output av pop, size, speed, sight in a format interprettable be something
// like gnuplot
#define LOGCREATUREDATA

// food avoids other food, value is pressure multiplier
#define FOODPRESSURE 20

// food slowly drifts to a random location, value is max range that the location
// is
// #define ACTIVEFOOD 500

// food experiences drag, value is drag coeficient
#define FOODDRAG 0.0005

// food experiences a pushback force at the border of the simulation
#define FOODBORDER

#define THREADS	  2
#define EATRADIUS 5
#define DAMAGE	  10

#define FOODVOL	 25
#define MEATVOL	 25
#define LEACHVOL .25

#define BITEDELAY 20

#define MAXMEAT	   2000
#define FOODENERGY .5
#define MEATENERGY 2

#define RAY_LENGTH 1000

#define TOTAL_INPUT	 15
#define TOTAL_OUTPUT 5
#define TOTAL_HIDDEN 4
#define TOTAL_NODES	 (TOTAL_INPUT + TOTAL_OUTPUT + TOTAL_HIDDEN)

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

#define FOWARD_OUTPUT (TOTAL_INPUT + 0)
#define RIGHT_OUTPUT  (TOTAL_INPUT + 1)
#define LEFT_OUTPUT	  (TOTAL_INPUT + 2)
#define EAT_OUTPUT	  (TOTAL_INPUT + 3)
#define LAYEGG_OUTPUT (TOTAL_INPUT + 4)

#define PREGNANCY_COST 1
#define METABOLISM	   .25

#define MENU_PADDING 1

#define BASE_B_VALUE 63

#define WIDTH  1920
#define HEIGHT 1080

#define CLEARCOLOR \
	{              \
		12, 12, 24 \
	}

#define NETWORK_PADDING 20

#define EXTRA_BYTES 4

#define MENU_BORDERTHICKNESS  (float)6
#define MENU_DECORATIONHEIGHT (float)(4 + (MENU_BORDERTHICKNESS * 4))

#define MENU_BORDEREDGE (float)2

#define MENU_BORDERCOLOR \
	{                    \
		140, 140, 140    \
	}
#define MENU_SHADOWCOLOR \
	{                    \
		100, 100, 100    \
	}
#define MENU_BODYCOLOR \
	{                  \
		210, 210, 210  \
	}
