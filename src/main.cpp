#include <AGL/agl.hpp>

#include "../inc/MenuBar.hpp"
#include "../inc/NewCreature.hpp"
#include "../inc/Simulation.hpp"
#include "AGL/include/math.hpp"

#include <cctype>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <math.h>
#include <string>
#include <thread>

agl::Circle	   *PhyCircle::circle;
agl::Rectangle *PhyRect::rect;
b2World		   *NewCreature::world;
Environment	   *NewCreature::env;

class Listener
{
	private:
		std::function<void()> first;
		std::function<void()> hold;
		std::function<void()> last;
		bool				  pastState = false;

	public:
		Listener(std::function<void()> first, std::function<void()> hold, std::function<void()> last);
		void update(bool state);
};

Listener::Listener(std::function<void()> first, std::function<void()> hold, std::function<void()> last)
{
	this->first = first;
	this->hold	= hold;
	this->last	= last;
}

void Listener::update(bool state)
{
	if (state)
	{
		if (pastState)
		{
			hold();
		}
		else
		{
			first();

			pastState = true;
		}
	}
	else if (pastState)
	{
		last();
		pastState = false;
	}
}

int getMillisecond()
{
	auto timepoint = std::chrono::system_clock::now().time_since_epoch();
	return std::chrono::duration_cast<std::chrono::milliseconds>(timepoint).count();
}

agl::Vec<float, 2> getCursorScenePosition(agl::Vec<float, 2> cursorWinPos, agl::Vec<float, 2> winSize, float winScale,
										  agl::Vec<float, 2> cameraPos)
{
	return ((cursorWinPos - (winSize * .5)) * winScale) + cameraPos;
}

void printConnections(CreatureData creatureData)
{
	for (int i = 0; i < creatureData.totalConnections; i++)
	{
		printf("connection %d going from %d to %d with weight %f\n", i, creatureData.connection[i].startNode,
			   creatureData.connection[i].endNode, creatureData.connection[i].weight);
	}

	return;
}

void loadRules(std::string path, SimulationRules *simulationRules)
{
	// read in order in .hpp
	const int	  bufLength = 12;
	std::string	  buffer[bufLength];
	std::ifstream stream(path);

	for (int i = 0; i < bufLength; i++)
	{
		std::getline(stream, buffer[i]);
		std::getline(stream, buffer[i]);
	}

	simulationRules->size.x			   = stoi(buffer[0]);
	simulationRules->size.y			   = stoi(buffer[1]);
	simulationRules->gridResolution.x  = stoi(buffer[2]);
	simulationRules->gridResolution.y  = stoi(buffer[3]);
	simulationRules->startingCreatures = stoi(buffer[4]);
	simulationRules->foodEnergy		   = stoi(buffer[5]);
	simulationRules->foodCap		   = stoi(buffer[7]);

	stream.close();

	return;
}

template <typename T> bool contains(std::list<BaseEntity *> &list, T *p)
{
	return std::find_if(list.begin(), list.end(), [&](BaseEntity *&d) { return d == (BaseEntity *)(DoNotUse *)p; }) !=
		   list.end();
}

int main()
{
	printf("Starting AGL\n");

	agl::RenderWindow window;
	window.setup({WIDTH, HEIGHT}, "EvolutionSimulator");
	window.setClearColor(CLEARCOLOR);
	window.setFPS(0);

	agl::Vec<float, 2> windowSize;

	glDisable(GL_DEPTH_TEST);

	// window.GLEnable(GL_ALPHA_TEST);
	// glAlphaFunc(GL_GREATER, 0.1f);

	window.setSwapInterval(1);

	agl::Event event;
	event.setWindow(window);

	agl::Shader simpleShader;
	simpleShader.loadFromFile("./shader/vert.glsl", "./shader/frag.glsl");

	agl::Shader gridShader;
	gridShader.loadFromFile("./shader/gridvert.glsl", "./shader/grid.glsl");

	agl::Camera camera;
	camera.setOrthographicProjection(0, WIDTH, HEIGHT, 0, 0.1, 100);
	agl::Vec<float, 2> cameraPosition = {0, 0};
	camera.setView({cameraPosition.x, cameraPosition.y, 50}, {0, 0, 0}, {0, 1, 0});

	agl::Camera guiCamera;
	guiCamera.setOrthographicProjection(0, WIDTH, HEIGHT, 0, 0.1, 100);
	guiCamera.setView({0, 0, 50}, {0, 0, 0}, {0, 1, 0});

	agl::Texture foodTexture;
	foodTexture.loadFromFile("./img/food.png");

	agl::Texture creatureBodyTexture;
	creatureBodyTexture.loadFromFile("./img/creatureBody.png");

	agl::Texture creatureExtraTexture;
	creatureExtraTexture.loadFromFile("./img/creatureExtra.png");

	agl::Texture eggTexture;
	eggTexture.loadFromFile("./img/egg.png");

	agl::Texture meatTexture;
	meatTexture.loadFromFile("./img/meat.png");

	agl::Texture blank;
	blank.setBlank();

	agl::Font font;
	font.setup("./font/font.ttf", 16);

	agl::Font smallFont;
	smallFont.setup("./font/font.ttf", 12);

	agl::Rectangle background;
	background.setTexture(&blank);
	background.setColor(CLEARCOLOR);
	background.setPosition({0, 0, 0});

	agl::Rectangle foodShape;
	foodShape.setTexture(&foodTexture);
	foodShape.setColor(agl::Color::Green);
	foodShape.setSize(agl::Vec<float, 3>{-10, -10, 0});
	foodShape.setOffset({5, 5, 30});

	agl::Rectangle creatureShape;
	creatureShape.setTexture(&creatureBodyTexture);
	creatureShape.setColor(agl::Color::White);
	creatureShape.setSize(agl::Vec<float, 3>{25, 60, 0});
	creatureShape.setOffset(agl::Vec<float, 3>{-12.5, -12.5, 30});

	agl::Rectangle eggShape;
	eggShape.setTexture(&eggTexture);
	eggShape.setColor(agl::Color::White);
	eggShape.setSize(agl::Vec<float, 3>{15, 15, 0});
	eggShape.setOffset({7.5, 7.5, 30});

	agl::Rectangle meatShape;
	meatShape.setTexture(&meatTexture);
	meatShape.setColor(agl::Color::White);
	meatShape.setSize(agl::Vec<float, 3>{-10, -10, 0});
	meatShape.setOffset({5, 5, 30});

	agl::Rectangle rayShape;
	rayShape.setTexture(&blank);
	rayShape.setColor(agl::Color::White);
	rayShape.setSize(agl::Vec<float, 3>{1, RAY_LENGTH});
	rayShape.setOffset(agl::Vec<float, 3>{-0.5, 0, 0});

	std::string nodeNames[TOTAL_NODES];
	nodeNames[CONSTANT_INPUT]	 = "Constant";
	nodeNames[X_INPUT]			 = "X Position";
	nodeNames[Y_INPUT]			 = "Y Position";
	nodeNames[ROTATION_INPUT]	 = "Rotation";
	nodeNames[SPEED_INPUT]		 = "Speed";
	nodeNames[FOOD_DISTANCE]	 = "Distance To Food";
	nodeNames[FOOD_ROTATION]	 = "Rotation To Food";
	nodeNames[CREATURE_DISTANCE] = "Distance To Creature";
	nodeNames[CREATURE_ROTATION] = "Rotation To Creature";
	nodeNames[ENERGY_INPUT]		 = "Energy";
	nodeNames[HEALTH_INPUT]		 = "Health";
	nodeNames[LIFE_INPUT]		 = "Life Left";

	for (int i = TOTAL_INPUT; i < TOTAL_INPUT + TOTAL_HIDDEN; i++)
	{
		nodeNames[i] = "Hidden";
	}

	nodeNames[FOWARD_OUTPUT] = "Move Foward";
	nodeNames[RIGHT_OUTPUT]	 = "Turn Right";
	nodeNames[LEFT_OUTPUT]	 = "Turn Left";
	nodeNames[EAT_OUTPUT]	 = "Eat";
	nodeNames[LAYEGG_OUTPUT] = "Lay Egg";
	nodeNames[MEAT_ROTATION] = "Rotation To Meat";
	nodeNames[MEAT_DISTANCE] = "Distance To Meat";

	printf("loading simulation rules from sim.conf\n");

	SimulationRules simulationRules;

	loadRules("./conf/sim.conf", &simulationRules);

	std::cout << "startingCreatures - " << simulationRules.startingCreatures << '\n';
	std::cout << "foodEnergy - " << simulationRules.foodEnergy << '\n';
	std::cout << "maxFood - " << simulationRules.foodCap << '\n';
	std::cout << "size - " << simulationRules.size << '\n';
	std::cout << "gridResolution - " << simulationRules.gridResolution << '\n';

	background.setSize(simulationRules.size);

	printf("starting sim\n");

	// background.setPosition(simulationRules.size * .5);

	Simulation simulation;
	// simulation.create(simulationRules, 0);

	Creature *focusCreature = nullptr;

	float		fps = 0;
	std::string nodeName;

	bool leftClick;
	bool previousLeftClick;

	Listener leftClickListener([&]() { leftClick = true; }, [&]() { leftClick = false; }, [&]() { leftClick = false; });

	// menu shit

	MenuShare::init(&blank, &font, &smallFont, &event, &leftClick);

	// simulation info

	struct
	{
			ValueElement<int>	*creatures;
			ValueElement<int>	*eggs;
			ValueElement<int>	*food;
			ValueElement<int>	*meat;
			ValueElement<int>	*frame;
			ValueElement<float> *fps;
	} simulationInfoPointers;

	struct
	{
			int creatures;
			int eggs;
			int food;
			int meat;
	} statsForSimInfo;

	Menu simulationInfo("SimInfo", 125,													 //
						ValueElement<int>{"Frame", [&]() { return &simulation.frame; }}, //
						ValueElement<float>{"FPS", [&]() { return &fps; }}				 //
	);

	simulationInfo.bindPointers(&simulationInfoPointers);

	simulationInfo.requirement = [&]() { return simulation.active; };

	// creatureNetwork

	struct
	{
			NetworkGraph			  *network;
			ValueElement<std::string> *node;
	} creatureNetworkPointers;

	Menu creatureNetwork("CreatureNetwork", 325,										//
						 NetworkGraph{[&]() { return &focusCreature->network; }},		//
						 ValueElement<std::string>{"Node", [&]() { return &nodeName; }} //
	);

	creatureNetwork.bindPointers(&creatureNetworkPointers);

	creatureNetwork.requirement = [&]() {
		if (focusCreature != nullptr)
		{
			return true;
		}
		else
		{
			return false;
		}
	};

	// creatureVectors

	struct
	{
			TextElement			*posText;
			ValueElement<float> *posX;
			ValueElement<float> *posY;
			TextElement			*velText;
			ValueElement<float> *velX;
			ValueElement<float> *velY;
			TextElement			*forText;
			ValueElement<float> *forX;
			ValueElement<float> *fooY;

	} creatureVectorsPointers;

	// Menu creatureVectors("CreatureVectors", 200,
	// // 					 TextElement{"- Position -"},
	// // 					 ValueElement<float>{"X", [&]()
	// { return &focusCreature->position.x; }},
	// // 					 ValueElement<float>{"Y", [&]()
	// { return &focusCreature->position.y; }},
	// // 					 TextElement{"- Velocity -"},
	// // 					 ValueElement<float>{"X", [&]()
	// { return &focusCreature->velocity.x; }},
	// // 					 ValueElement<float>{"Y", [&]()
	// { return &focusCreature->velocity.y; }},
	// // 					 TextElement{"- Force -"},
	// // 					 ValueElement<float>{"X", [&]()
	// { return &focusCreature->force.x; }},
	// // 					 ValueElement<float>{"Y", [&]()
	// { return &focusCreature->force.y; }}
	// //
	// );

	// creatureVectors.bindPointers(&creatureVectorsPointers);

	// creatureVectors.requirement = creatureNetwork.requirement;

	// creatureMisc

	struct
	{
			ValueElement<bool>	*eating;
			ValueElement<bool>	*layingEgg;
			SpacerElement		*sp1;
			ValueElement<float> *health;
			ValueElement<float> *energy;
			ValueElement<float> *lifeLeft;
			SpacerElement		*sp2;
			ValueElement<float> *sight;
			ValueElement<float> *speed;
			ValueElement<float> *size;
			ValueElement<int>	*hue;
			SpacerElement		*sp3;
			ValueElement<float> *biomass;
			ValueElement<float> *energyDensity;
			ValueElement<float> *metabolism;
			ValueElement<float> *preference;
	} creatureMiscPointers;

	Menu creatureMisc("CreatureMisc", 200,																	  //
					  ValueElement<bool>{"Eating", [&]() { return &focusCreature->eating; }},				  //
					  ValueElement<bool>{"Laying Egg", [&]() { return &focusCreature->layingEgg; }},		  //
					  SpacerElement{},																		  //
					  ValueElement<float>{"Health", [&]() { return &focusCreature->health; }},				  //
					  ValueElement<float>{"Energy", [&]() { return &focusCreature->energy; }},				  //
					  ValueElement<int>{"Life Left", [&]() { return &focusCreature->life; }},				  //
					  SpacerElement{},																		  //
					  ValueElement<float>{"Sight", [&]() { return &focusCreature->sight; }},				  //
					  ValueElement<float>{"Speed", [&]() { return &focusCreature->speed; }},				  //
					  ValueElement<float>{"Size", [&]() { return &focusCreature->size; }},					  //
					  ValueElement<int>{"Hue", [&]() { return &focusCreature->hue; }},						  //
					  SpacerElement{},																		  //
					  ValueElement<float>{"Biomass", [&]() { return &focusCreature->biomass; }},			  //
					  ValueElement<float>{"Energy Density", [&]() { return &focusCreature->energyDensity; }}, //
					  ValueElement<float>{"Metabolism", [&]() { return &focusCreature->metabolism; }},		  //
					  ValueElement<float>{"Preference", [&]() { return &focusCreature->preference; }}		  //
	);

	creatureMisc.bindPointers(&creatureMiscPointers);

	// creatureMisc.requirement = creatureVectors.requirement;

	// leftMenu

	struct
	{
			ButtonElement<Toggle> *food;
			ButtonElement<Toggle> *meat;
			ButtonElement<Toggle> *select;
			ButtonElement<Toggle> *kill;
			SpacerElement		  *sp1;
			ButtonElement<Toggle> *forceFood;
			ButtonElement<Toggle> *forceMeat;
			ButtonElement<Toggle> *forceCreature;
			FieldElement<float>	  *forceMultiplier;
			SpacerElement		  *sp2;
			ButtonElement<Toggle> *sendTo;
	} leftMenuPointers;

	Menu leftMenu("LeftMenu", 200,						  //
				  ButtonElement<Toggle>{"Food"},		  //
				  ButtonElement<Toggle>{"Meat"},		  //
				  ButtonElement<Toggle>{"Select"},		  //
				  ButtonElement<Toggle>{"Kill"},		  //
				  SpacerElement{},						  //
				  ButtonElement<Toggle>{"ForceFood"},	  //
				  ButtonElement<Toggle>{"ForceMeat"},	  //
				  ButtonElement<Toggle>{"ForceCreature"}, //
				  FieldElement<float>{"ForceMult", 1.0},  //
				  SpacerElement{},						  //
				  ButtonElement<Toggle>{"SendTo"}		  //
	);

	leftMenu.bindPointers(&leftMenuPointers);

	// simRules

	struct
	{
			FieldElement<float> *gravityX;
			FieldElement<float> *gravityY;
			FieldElement<float> *nextMass;
	} simRulesPointers;

	simulation.foodCap = simulationRules.foodCap;

	Menu simRules("SimRules", 200,				   //
				  FieldElement<float>{"GravX", 0}, //
				  FieldElement<float>{"GravY", .3}, //
				  FieldElement<float>{"Mass", 1}   //
	);

	simRules.bindPointers(&simRulesPointers);

	// quitmenu

	struct
	{
			TextElement			*text;
			ButtonElement<Hold> *confirm;
			ButtonElement<Hold> *cancel;
	} quitMenuPointers;

	Menu quitMenu("Quit", 100,					  //
				  TextElement{"Quit"},			  //
				  ButtonElement<Hold>{"Confirm"}, //
				  ButtonElement<Hold>{"Cancel"}	  //
	);
	quitMenu.bindPointers(&quitMenuPointers);

	// simMenu

	struct
	{
			FieldElement<int>	  *sizeX;
			FieldElement<int>	  *sizeY;
			FieldElement<int>	  *gridX;
			FieldElement<int>	  *gridY;
			FieldElement<int>	  *startingCreatures;
			FieldElement<int>	  *seed;
			FieldElement<float>	  *simCycles;
			ButtonElement<Hold>	  *start;
			ButtonElement<Hold>	  *kill;
			ButtonElement<Toggle> *pause;
	} simMenuPointers;

	Menu simMenu("simMenu", 175,														  //
				 FieldElement<int>{"sizeX", (simulationRules.size.x)},					  //
				 FieldElement<int>{"sizeY", (simulationRules.size.y)},					  //
				 FieldElement<int>{"gridX", (simulationRules.gridResolution.x)},		  //
				 FieldElement<int>{"gridY", (simulationRules.gridResolution.y)},		  //
				 FieldElement<int>{"startCreature", (simulationRules.startingCreatures)}, //
				 FieldElement<int>{"seed", 0},											  //
				 FieldElement<float>{"simCycles", 1.0},									  //
				 ButtonElement<Hold>{"START"},											  //
				 ButtonElement<Hold>{"KILL"},											  //
				 ButtonElement<Toggle>{"PAUSE"}											  //
	);

	simMenu.bindPointers(&simMenuPointers);

	// debugLog

	struct
	{
			TextElement *t1;
			TextElement *t2;
			TextElement *t3;
			TextElement *t4;
			TextElement *t5;
			TextElement *t6;
			TextElement *t7;
			TextElement *t8;
	} debugLogPointers;

	Menu debugLog("DebugLog", 400, TextElement{""}, TextElement{""}, TextElement{""}, TextElement{""}, TextElement{""},
				  TextElement{""}, TextElement{""}, TextElement{""});

	debugLog.bindPointers(&debugLogPointers);

	auto sendDebugLog = [&](std::string str) {
		debugLogPointers.t1->str = debugLogPointers.t2->str;
		debugLogPointers.t2->str = debugLogPointers.t3->str;
		debugLogPointers.t3->str = debugLogPointers.t4->str;
		debugLogPointers.t4->str = debugLogPointers.t5->str;
		debugLogPointers.t5->str = debugLogPointers.t6->str;
		debugLogPointers.t6->str = debugLogPointers.t7->str;
		debugLogPointers.t7->str = debugLogPointers.t8->str;

		debugLogPointers.t8->str = str;
	};

	// menubar

	MenuBar menuBar(&quitMenu,		 //
					&simulationInfo, //
					&simMenu,		 //
					&simRules,		 //
					&debugLog		 //
	);

	bool mHeld		= false;
	bool b1Held		= false;
	bool ReturnHeld = false;

	bool skipRender = false;

	float sizeMultiplier = 1;

	printf("entering sim loop\n");

	bool quiting = false;

	agl::Circle circleShape(50);
	circleShape.setTexture(&foodTexture);
	agl::Rectangle rectShape;
	rectShape.setTexture(&blank);

	PhyCircle::circle = &circleShape;
	PhyRect::rect	  = &rectShape;

	NewCreature::world = &simulation.phyWorld;
	NewCreature::env   = &simulation.env;

	{
		SimulationRules simulationRules;
		simulationRules.size.x			  = simMenuPointers.sizeX->value;
		simulationRules.size.y			  = simMenuPointers.sizeY->value;
		simulationRules.gridResolution.x  = simMenuPointers.gridX->value;
		simulationRules.gridResolution.y  = simMenuPointers.gridY->value;
		simulationRules.startingCreatures = simMenuPointers.startingCreatures->value;

		simulation.create(simulationRules, simMenuPointers.seed->value);

		background.setSize(simulationRules.size);
	}

	simMenuPointers.pause->state = true;

	NewCreature creature;

	while (!event.windowClose())
	{
		static int milliDiff = 0;
		int		   start	 = getMillisecond();

		event.poll();

		if (!simMenuPointers.pause->state && simulation.active)
		{
			static float cycle = 0;

			cycle += simMenuPointers.simCycles->value;

			for (int i = 0; i < cycle; i++)
			{
				cycle--;
				simulation.update();
			}
		}

		agl::Vec<int, 2> topLeftGrid;
		agl::Vec<int, 2> bottomRightGrid;

		if (skipRender)
		{
			goto skipRendering;
		}

		window.clear();

		// Simulation rendering

		window.getShaderUniforms(gridShader);
		gridShader.use();

		window.updateMvp(camera);

		glUniform1f(gridShader.getUniformLocation("scale"), sizeMultiplier);

		window.drawShape(background);

		window.getShaderUniforms(simpleShader);
		simpleShader.use();

		if (!simulation.active)
		{
			goto skipSimRender;
		}

		window.updateMvp(camera);

		{
			agl::Vec<float, 2> tlPos = getCursorScenePosition({0, 0}, windowSize, sizeMultiplier, cameraPosition);
			topLeftGrid				 = simulation.env.toGridPosition(tlPos);

			agl::Vec<float, 2> brPos = getCursorScenePosition(windowSize, windowSize, sizeMultiplier, cameraPosition);
			bottomRightGrid			 = simulation.env.toGridPosition(brPos);
		}

		simulation.env.view<PhyCircle>([&window = window](PhyCircle &dt, auto) { dt.drawFunc(window); });
		simulation.env.view<PhyRect>([&window = window](PhyRect &dt, auto) { dt.drawFunc(window); });

		{
			static bool				  drawing = false;
			static agl::Vec<float, 2> start;
			static agl::Vec<float, 2> end;
			static float			  width = 2;

			end = getCursorScenePosition(event.getPointerWindowPosition(), windowSize, sizeMultiplier, cameraPosition);

			if (event.isKeyPressed(agl::Key::Q))
			{
				width -= .5;
			}
			if (event.isKeyPressed(agl::Key::E))
			{
				width += .5;
			}

			if (width < 2)
			{
				width = 2;
			}

			if (event.keybuffer.find('w') != -1 &&
				(creature.touchingSelected(getCursorScenePosition(event.getPointerWindowPosition(), windowSize,
																  sizeMultiplier, cameraPosition)) ||
				 drawing))
			{
				if (!drawing)
				{
					start = getCursorScenePosition(event.getPointerWindowPosition(), windowSize, sizeMultiplier,
												   cameraPosition);
				}

				drawing = !drawing;

				if (!drawing)
				{
					float			   rotation = (start - end).angle();
					float			   height	= (start - end).length();
					agl::Vec<float, 2> normal	= (end - start).normalized();

					creature.createPart({width, height}, start + (normal * height / 2), rotation, start);
				}
			}

			if (drawing)
			{
				float			   rotation = -(start - end).angle();
				float			   height	= (start - end).length();
				agl::Vec<float, 2> normal	= (end - start).normalized();

				rectShape.setColor(agl::Color::Gray);
				rectShape.setSize(agl::Vec<float, 2>{width, height});
				rectShape.setPosition(start + (normal * height / 2));
				rectShape.setOffset(agl::Vec<float, 2>{width, height} * -.5);
				rectShape.setRotation({0, 0, agl::radianToDegree(rotation)});

				window.drawShape(rectShape);
			}
		}

		if (event.isPointerButtonPressed(agl::Button::Left))
		{
			auto pos =
				getCursorScenePosition(event.getPointerWindowPosition(), windowSize, sizeMultiplier, cameraPosition);
			creature.selectRect(pos);
		}

	skipSimRender:;

		// gui rendering

		fps = (1000. / milliDiff);

		window.updateMvp(guiCamera);

		if (focusCreature != nullptr)
		{
			if (contains(simulation.env.getList<Creature>(), focusCreature))
			{
				nodeName =
					nodeNames[creatureNetworkPointers.network->selectedID] + " " +
					std::to_string(focusCreature->network->getNode(creatureNetworkPointers.network->selectedID).value);
			}
			else
			{
				focusCreature = nullptr;
			}
		}

		for (std::string &msgLog : Debug::log)
		{
			debugLog.get<0>().str = debugLog.get<1>().str;
			debugLog.get<1>().str = debugLog.get<2>().str;
			debugLog.get<2>().str = debugLog.get<3>().str;
			debugLog.get<3>().str = debugLog.get<4>().str;
			debugLog.get<4>().str = debugLog.get<5>().str;
			debugLog.get<5>().str = debugLog.get<6>().str;
			debugLog.get<6>().str = debugLog.get<7>().str;

			debugLog.get<7>().str = msgLog;
		}

		Debug::log.clear();

		window.draw(menuBar);

		window.display();

	skipRendering:;

		// quit?

		if (quitMenuPointers.confirm->state)
		{
			break;
		}
		if (quitMenuPointers.cancel->state)
		{
			quitMenu.close();
			quitMenuPointers.cancel->state = false;
		}

		if (simMenuPointers.kill->state && simulation.active)
		{
			simulation.destroy();
			focusCreature = nullptr;
		}
		else if (simMenuPointers.start->state && !simulation.active)
		{
			// SimulationRules simulationRules;
			// simulationRules.size.x			  =
			// simMenuPointers.sizeX->value;
			// simulationRules.size.y			  =
			// simMenuPointers.sizeY->value; simulationRules.gridResolution.x  =
			// simMenuPointers.gridX->value; simulationRules.gridResolution.y  =
			// simMenuPointers.gridY->value; simulationRules.startingCreatures =
			// simMenuPointers.startingCreatures->value;
			//
			// simulation.create(simulationRules, simMenuPointers.seed->value);
			//
			// background.setSize(simulationRules.size);
		}

		if (event.keybuffer.find('h') != std::string::npos && FocusableElement::focusedField == nullptr)
		{
			menuBar.exists = !menuBar.exists;
		}

		if (!simulation.active)
		{
			goto deadSim;
		}

		// funky new shit

		if (menuBar.exists)
		{
			for (int i = 0; i < menuBar.length; i++)
			{
				SimpleMenu *menu = menuBar.menu[i];

				if (pointInArea(event.getPointerWindowPosition(), menu->position, menu->size) && menu->exists)
				{
					goto endif;
				}
			}
		}

		// input

		if (event.isPointerButtonPressed(agl::Button::Right))
		{
			agl::Vec<int, 2> cursorRelPos =
				getCursorScenePosition(event.getPointerWindowPosition(), windowSize, sizeMultiplier, cameraPosition);

			simulation.env.getArea<PhyRect>(
				[&](PhyRect &obj) {
					agl::Vec<float, 2> offset	= obj.position - cursorRelPos;
					float			   distance = offset.length();

					float forceScalar = leftMenuPointers.forceMultiplier->value / distance * distance;

					agl::Vec<float, 2> force = offset.normalized() * forceScalar * 1000;

					obj.phyBody->ApplyForceToCenter(PhysicsObj::scaleForce(force), true);
				},
				simulation.env.toGridPosition(cursorRelPos));
		}

	endif:;

		simulation.gravity.x = simRulesPointers.gravityX->value;
		simulation.gravity.y = simRulesPointers.gravityY->value;

	deadSim:;

		// camera movement

		static agl::Vec<float, 2> cameraOffset;
		static agl::Vec<float, 2> startPos;

		if (event.isPointerButtonPressed(agl::Button::Middle))
		{
			if (b1Held) // holding click
			{
				cameraPosition = cameraPosition - cameraOffset;

				cameraOffset = startPos - event.getPointerWindowPosition();
				cameraOffset.x *= sizeMultiplier;
				cameraOffset.y *= sizeMultiplier;

				cameraPosition.x += cameraOffset.x;
				cameraPosition.y += cameraOffset.y;
			}
			else // first click
			{
				window.setCursorShape(agl::CursorType::Arrow);
				startPos = event.getPointerWindowPosition();
				b1Held	 = true;
			}
		}
		else if (b1Held) // let go
		{
			window.setCursorShape(agl::CursorType::Arrow);
			cameraOffset = {0, 0};
			b1Held		 = false;
		}

		static float cameraSpeed = 4;

		const float sizeDelta = .2;

		if (event.scroll == agl::Up)
		{
			float scale = sizeDelta * sizeMultiplier;

			agl::Vec<float, 2> oldPos =
				getCursorScenePosition(event.getPointerWindowPosition(), windowSize, sizeMultiplier, cameraPosition);

			sizeMultiplier -= scale;

			agl::Vec<float, 2> newPos =
				getCursorScenePosition(event.getPointerWindowPosition(), windowSize, sizeMultiplier, cameraPosition);

			agl::Vec<float, 2> offset = oldPos - newPos;

			cameraPosition = cameraPosition + offset;
		}
		if (event.scroll == agl::Down)
		{
			float scale = sizeDelta * sizeMultiplier;

			agl::Vec<float, 2> oldPos =
				getCursorScenePosition(event.getPointerWindowPosition(), windowSize, sizeMultiplier, cameraPosition);

			sizeMultiplier += scale;

			agl::Vec<float, 2> newPos =
				getCursorScenePosition(event.getPointerWindowPosition(), windowSize, sizeMultiplier, cameraPosition);

			agl::Vec<float, 2> offset = oldPos - newPos;

			cameraPosition = cameraPosition + offset;
		}

		window.setViewport(0, 0, windowSize.x, windowSize.y);

		camera.setOrthographicProjection(-((windowSize.x / 2.) * sizeMultiplier),
										 ((windowSize.x / 2.) * sizeMultiplier), ((windowSize.y / 2.) * sizeMultiplier),
										 -((windowSize.y / 2.) * sizeMultiplier), 0.1, 100);
		camera.setView({cameraPosition.x, cameraPosition.y, 50}, {cameraPosition.x, cameraPosition.y, 0}, {0, 1, 0});

		guiCamera.setOrthographicProjection(0, windowSize.x, windowSize.y, 0, 0.1, 100);

		windowSize = window.getState().size;

		leftClickListener.update(event.isPointerButtonPressed(agl::Button::Left));

		milliDiff = getMillisecond() - start;
	}

	if (simulation.active)
	{
		simulation.destroy();
	}

	MenuShare::destroy();

	font.deleteFont();

	foodTexture.deleteTexture();
	creatureBodyTexture.deleteTexture();
	creatureExtraTexture.deleteTexture();
	eggTexture.deleteTexture();
	blank.deleteTexture();

	simpleShader.deleteProgram();
	gridShader.deleteProgram();

	window.close();

	return 0;
}
