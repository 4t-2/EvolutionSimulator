#include <AGL/agl.hpp>

#include "../inc/MenuBar.hpp"
#include "../inc/Simulation.hpp"

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
	simulationRules->foodCap = stoi(buffer[7]);

	stream.close();

	return;
}

template <typename T> bool contains(std::list<EntityData> &list, T *p)
{
	return std::find_if(list.begin(), list.end(), [&](EntityData &d) { return d.data == p; }) != list.end();
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

	Menu simulationInfo("SimInfo", 125, //
						ValueElement<int>{"Creatures",
										  [&]() {
											  statsForSimInfo.creatures = simulation.env.getList<Creature>().size();
											  return &statsForSimInfo.creatures;
										  }}, //
						ValueElement<int>{"Eggs",
										  [&]() {
											  statsForSimInfo.eggs = simulation.env.getList<Egg>().size();
											  return &statsForSimInfo.eggs;
										  }}, //
						ValueElement<int>{"Food",
										  [&]() {
											  statsForSimInfo.food = simulation.env.getList<Food>().size();
											  return &statsForSimInfo.food;
										  }}, //
						ValueElement<int>{"Meat",
										  [&]() {
											  statsForSimInfo.meat = simulation.env.getList<Meat>().size();
											  return &statsForSimInfo.meat;
										  }},											 //
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

	Menu creatureVectors("CreatureVectors", 200,												 //
						 TextElement{"- Position -"},											 //
						 ValueElement<float>{"X", [&]() { return &focusCreature->position.x; }}, //
						 ValueElement<float>{"Y", [&]() { return &focusCreature->position.y; }}, //
						 TextElement{"- Velocity -"},											 //
						 ValueElement<float>{"X", [&]() { return &focusCreature->velocity.x; }}, //
						 ValueElement<float>{"Y", [&]() { return &focusCreature->velocity.y; }}, //
						 TextElement{"- Force -"},												 //
						 ValueElement<float>{"X", [&]() { return &focusCreature->force.x; }},	 //
						 ValueElement<float>{"Y", [&]() { return &focusCreature->force.y; }}	 //
	);

	creatureVectors.bindPointers(&creatureVectorsPointers);

	creatureVectors.requirement = creatureNetwork.requirement;

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

	creatureMisc.requirement = creatureVectors.requirement;

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
			FieldElement<float> *foodDen;
			FieldElement<float> *foodVol;
			FieldElement<float> *meatDen;
			FieldElement<float> *leachVol;
			FieldElement<int>	*maxFood;
			FieldElement<float> *damage;
			FieldElement<float> *energyCostMultiplier;
	} simRulesPointers;

	simulation.foodCap = simulationRules.foodCap;

	Menu simRules("SimRules", 200,												   //
				  FieldElement<float>{"FdEnDn", simulation.foodEnergyDensity},	   //
				  FieldElement<float>{"FdVol", (simulation.foodVol)},			   //
				  FieldElement<float>{"MtEnDn", (simulation.meatEnergyDensity)},   //
				  FieldElement<float>{"LeVol", (simulation.leachVol)},			   //
				  FieldElement<int>{"maxFd", (simulation.foodCap)},				   //
				  FieldElement<float>{"dmg", (simulation.damage)},				   //
				  FieldElement<float>{"EnCoMu", (simulation.energyCostMultiplier)} //
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

	MenuBar menuBar(&quitMenu,		  //
					&simulationInfo,  //
					&simMenu,		  //
					&leftMenu,		  //
					&simRules,		  //
					&creatureNetwork, //
					&creatureVectors, //
					&creatureMisc,	  //
					&debugLog		  //
	);

	bool mHeld		= false;
	bool b1Held		= false;
	bool ReturnHeld = false;

	bool skipRender = false;

	float sizeMultiplier = 1;

	printf("entering sim loop\n");

	bool quiting = false;

	while (!event.windowClose())
	{
		static int milliDiff = 0;
		int		   start	 = getMillisecond();

		event.poll();

		// if (event.isKeyPressed(agl::Key::M))
		// {
		// 	mHeld = true;
		// }
		// else if (mHeld)
		// {
		// 	mHeld = false;
		//
		// 	CreatureData creatureData(1, 1, 1, 60, 15);
		//
		// 	creatureData.setConnection(0, CONSTANT_INPUT, FOWARD_OUTPUT, 1);
		// 	creatureData.setConnection(1, CONSTANT_INPUT, EAT_OUTPUT, 1);
		// 	creatureData.setConnection(2, CONSTANT_INPUT, LAYEGG_OUTPUT, 1);
		// 	creatureData.setConnection(3, CREATURE_ROTATION, LEFT_OUTPUT, 1);
		// 	creatureData.setConnection(4, CREATURE_ROTATION, RIGHT_OUTPUT, -1);
		// 	creatureData.setConnection(5, CONSTANT_INPUT, RIGHT_OUTPUT, -1);
		// 	creatureData.setConnection(6, CONSTANT_INPUT, LEFT_OUTPUT, -1);
		// 	creatureData.setConnection(7, CREATURE_PREFERENCE, RIGHT_OUTPUT, 1);
		// 	creatureData.setConnection(8, CREATURE_PREFERENCE, LEFT_OUTPUT, 1);
		//
		// 	creatureData.preference = 0;
		// 	creatureData.metabolism = METABOLISM;
		//
		// 	agl::Vec<float, 2> position;
		// 	position.x = (rand() / (float)RAND_MAX) * simulationRules.size.x;
		// 	position.y = (rand() / (float)RAND_MAX) * simulationRules.size.y;
		//
		// 	simulation.addCreature(creatureData, position);
		// }

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

		// Draw food
		simulation.env.view<Food>([&](auto &food, auto)
		{
			agl::Vec<float, 2> position = food.position;
			foodShape.setPosition(position);
			window.drawShape(foodShape);
		});

		simulation.env.view<Meat>([&](auto &meat, auto)
		{
			meatShape.setPosition(meat.position);
			meatShape.setSize({meat.radius * 2, meat.radius * 2});
			meatShape.setOffset({-meat.radius, -meat.radius});
			meatShape.setRotation({0, 0, meat.rotation});
			window.drawShape(meatShape);
		});

		// draw eggs
		simulation.env.view<Egg>([&](auto &egg, auto)
		{
			eggShape.setPosition(egg.position);
			window.drawShape(eggShape);
		});

		// draw rays
		if (contains(simulation.env.getList<Creature>(), focusCreature))
		{
			rayShape.setSize(agl::Vec<float, 3>{1, RAY_LENGTH * focusCreature->sight});
			rayShape.setPosition(focusCreature->position);

			{
				float angleOffset = focusCreature->network->getNode(CREATURE_ROTATION).value * 180;
				angleOffset += 180;

				float rayAngle = angleOffset - agl::radianToDegree(focusCreature->rotation);

				float weight = focusCreature->network->getNode(CREATURE_DISTANCE).value;

				rayShape.setColor({0, (unsigned char)(weight * 255), BASE_B_VALUE});

				rayShape.setRotation(agl::Vec<float, 3>{0, 0, rayAngle});
				window.drawShape(rayShape);
			}
			{
				float angleOffset = focusCreature->network->getNode(FOOD_ROTATION).value * 180;
				angleOffset += 180;

				float rayAngle = angleOffset - agl::radianToDegree(focusCreature->rotation);

				float weight = focusCreature->network->getNode(FOOD_DISTANCE).value;

				rayShape.setColor({0, (unsigned char)(weight * 255), BASE_B_VALUE});

				rayShape.setRotation(agl::Vec<float, 3>{0, 0, rayAngle});
				window.drawShape(rayShape);
			}
			{
				float angleOffset = focusCreature->network->getNode(MEAT_ROTATION).value * 180;
				angleOffset += 180;

				float rayAngle = angleOffset - agl::radianToDegree(focusCreature->rotation);

				float weight = focusCreature->network->getNode(MEAT_DISTANCE).value;

				rayShape.setColor({0, (unsigned char)(weight * 255), BASE_B_VALUE});

				rayShape.setRotation(agl::Vec<float, 3>{0, 0, rayAngle});
				window.drawShape(rayShape);
			}
		}

		// draw creature
		simulation.env.view<Creature>([&](auto &creature, auto)
		{
			creatureShape.setPosition(creature.position);
			creatureShape.setRotation(agl::Vec<float, 3>{0, 0, -float(creature.rotation * 180 / PI)});

			float speed = creature.velocity.length();

			creatureShape.setTexture(&creatureBodyTexture);

			int textureFrame = int(simulation.frame * (speed / 8)) % 6;

			if (textureFrame > 2)
			{
				textureFrame -= 2;

				creatureShape.setTextureScaling({-(1. / 3.), 1});
			}
			else
			{
				creatureShape.setTextureScaling({1. / 3., 1});
			}

			creatureShape.setTextureTranslation({float(1. / 3.) * textureFrame, 0});

			if (event.isKeyPressed(agl::Key::Z))
			{
				agl::Vec<float, 3> blue	  = agl::Vec<float, 3>{0, 0, 255} * creature.creatureData.usePG;
				agl::Vec<float, 3> yellow = agl::Vec<float, 3>{255, 255, 0} * creature.creatureData.useNEAT;

				creatureShape.setColor({(unsigned char)(blue.x + yellow.x), (unsigned char)(blue.y + yellow.y),
										(unsigned char)(blue.z + yellow.z)});
			}
			else
			{
				creatureShape.setColor(hueToRGB(creature.hue));
			}

			float size = creature.size;

			creatureShape.setSize(agl::Vec<float, 3>{25 * size, 60 * size, 0});
			creatureShape.setOffset(agl::Vec<float, 3>{(float)-12.5 * size, (float)-12.5 * size, -1});

			window.drawShape(creatureShape);

			creatureShape.setTexture(&creatureExtraTexture);

			creatureShape.setColor(agl::Color::White);

			creatureShape.setTextureScaling({1, 1});
			creatureShape.setTextureTranslation({1, 1});

			creatureShape.setOffset(agl::Vec<float, 3>{(float)-12.5 * size, (float)-12.5 * size, -.5});

			window.drawShape(creatureShape);
		});

	skipSimRender:;

		// gui rendering

		fps = (1000. / milliDiff);

		window.updateMvp(guiCamera);

		if (focusCreature != nullptr)
		{
			if (contains(simulation.env.getList<Creature>(), focusCreature))
			{
				nodeName = nodeNames[creatureNetworkPointers.network->selectedID];
			}
			else
			{
				focusCreature = nullptr;
			}
		}

		for (std::string msgLog : Debug::log)
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
			SimulationRules simulationRules;
			simulationRules.size.x			  = simMenuPointers.sizeX->value;
			simulationRules.size.y			  = simMenuPointers.sizeY->value;
			simulationRules.gridResolution.x  = simMenuPointers.gridX->value;
			simulationRules.gridResolution.y  = simMenuPointers.gridY->value;
			simulationRules.startingCreatures = simMenuPointers.startingCreatures->value;

			simulation.create(simulationRules, simMenuPointers.seed->value);
		}

		if (event.keybuffer.find('h') != std::string::npos && FocusableElement::focusedField == nullptr)
		{
			menuBar.exists = !menuBar.exists;
		}

		if (!simulation.active)
		{
			goto deadSim;
		}

		// input

		if (event.isKeyPressed(agl::Key::R))
		{
			focusCreature = nullptr;
		}

		if (event.isPointerButtonPressed(agl::Button::Left))
		{
			for (int i = 0; i < menuBar.length; i++)
			{
				SimpleMenu *menu = menuBar.menu[i];

				if (pointInArea(event.getPointerWindowPosition(), menu->position, menu->size))
				{
					goto endif;
				}
			}

			if (leftMenuPointers.food->state) // add food
			{
				simulation.addFood(getCursorScenePosition(event.getPointerWindowPosition(), windowSize, sizeMultiplier,
														  cameraPosition));
			}
			if (leftMenuPointers.meat->state) // add meat
			{
				simulation.addMeat(getCursorScenePosition(event.getPointerWindowPosition(), windowSize, sizeMultiplier,
														  cameraPosition));
			}
			if (leftMenuPointers.select->state) // select creature
			{
				simulation.env.view<Creature>([&](auto &creature, auto it)
				{
					agl::Vec<float, 2> mouse;
					mouse.x = ((event.getPointerWindowPosition().x - (windowSize.x * .5)) * sizeMultiplier) +
							  cameraPosition.x;
					mouse.y = ((event.getPointerWindowPosition().y - (windowSize.y * .5)) * sizeMultiplier) +
							  cameraPosition.y;

					float distance = (mouse - creature.position).length();

					if (distance < creature.radius)
					{
						focusCreature = &creature;

						return;
					}
				});
			}
			if (leftMenuPointers.kill->state) // kill creature
			{
				simulation.env.view<Creature>([&](auto &creature, auto it)
				{
					agl::Vec<float, 2> mouse;
					mouse.x = ((event.getPointerWindowPosition().x - (windowSize.x * .5)) * sizeMultiplier) +
							  cameraPosition.x;
					mouse.y = ((event.getPointerWindowPosition().y - (windowSize.y * .5)) * sizeMultiplier) +
							  cameraPosition.y;

					float distance = (mouse - creature.position).length();

					if (distance < creature.radius)
					{
						it--;
						simulation.addMeat(creature.position, creature.maxHealth / 4);
						simulation.env.removeEntity<Creature>(it);

						return;
					}
				});
			}
			if (leftMenuPointers.forceFood->state)
			{
				agl::Vec<int, 2> cursorRelPos = getCursorScenePosition(event.getPointerWindowPosition(), windowSize,
																	   sizeMultiplier, cameraPosition);

				agl::Vec<int, 2> gridPos =
					simulation.foodGrid->toGridPosition(cursorRelPos, simulation.simulationRules.size);

				simulation.foodGrid->updateElements(gridPos, {-1, -1}, {1, 1}, [&](Food *food) {
					agl::Vec<float, 2> offset	= food->position - cursorRelPos;
					float			   distance = offset.length();

					float forceScalar = leftMenuPointers.forceMultiplier->value / distance;

					agl::Vec<float, 2> force = offset.normalized() * forceScalar;

					food->force += force;
				});
			}
			if (leftMenuPointers.forceMeat->state)
			{
				agl::Vec<int, 2> cursorRelPos = getCursorScenePosition(event.getPointerWindowPosition(), windowSize,
																	   sizeMultiplier, cameraPosition);

				agl::Vec<int, 2> gridPos =
					simulation.meatGrid->toGridPosition(cursorRelPos, simulation.simulationRules.size);

				simulation.meatGrid->updateElements(gridPos, {-1, -1}, {1, 1}, [&](Meat *meat) {
					agl::Vec<float, 2> offset	= meat->position - cursorRelPos;
					float			   distance = offset.length();

					float forceScalar = leftMenuPointers.forceMultiplier->value / distance;

					agl::Vec<float, 2> force = offset.normalized() * forceScalar;

					meat->force += force;
				});
			}
			if (leftMenuPointers.forceCreature->state)
			{
				agl::Vec<int, 2> cursorRelPos = getCursorScenePosition(event.getPointerWindowPosition(), windowSize,
																	   sizeMultiplier, cameraPosition);

				agl::Vec<int, 2> gridPos =
					simulation.creatureGrid->toGridPosition(cursorRelPos, simulation.simulationRules.size);

				simulation.creatureGrid->updateElements(gridPos, {-1, -1}, {1, 1}, [&](Creature *creature) {
					agl::Vec<float, 2> offset	= creature->position - cursorRelPos;
					float			   distance = offset.length();

					float forceScalar = leftMenuPointers.forceMultiplier->value / distance;

					agl::Vec<float, 2> force = offset.normalized() * forceScalar;

					creature->force += force;

					sendDebugLog(std::to_string(force.length()));
				});
			}
			if (leftMenuPointers.sendTo->state)
			{
				if (focusCreature != nullptr)
				{
					focusCreature->position = getCursorScenePosition(event.getPointerWindowPosition(), windowSize,
																	 sizeMultiplier, cameraPosition);
				}
			}

		endif:;
		}

		simulation.foodEnergyDensity	= simRulesPointers.foodDen->value;
		simulation.foodVol				= simRulesPointers.foodVol->value;
		simulation.meatEnergyDensity	= simRulesPointers.meatDen->value;
		simulation.leachVol				= simRulesPointers.leachVol->value;
		simulation.foodCap				= simRulesPointers.maxFood->value;
		simulation.damage				= simRulesPointers.damage->value;
		simulation.energyCostMultiplier = simRulesPointers.energyCostMultiplier->value;

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
