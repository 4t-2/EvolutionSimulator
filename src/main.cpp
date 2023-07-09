#include "../lib/AGL/agl.hpp"

#include "../inc/MenuBar.hpp"
#include "../inc/Simulation.hpp"

#include <X11/X.h>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <math.h>
#include <string>
#include <unistd.h>

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
	int			  bufLength = 12;
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
	simulationRules->maxCreatures	   = stoi(buffer[6]);
	simulationRules->maxFood		   = stoi(buffer[7]);
	simulationRules->maxEggs		   = stoi(buffer[8]);
	simulationRules->preferedCreatures = stoi(buffer[9]);
	simulationRules->penaltyBuffer	   = stoi(buffer[10]);
	simulationRules->penaltyPeriod	   = stoi(buffer[11]);

	stream.close();

	return;
}

int main()
{
	printf("Starting AGL\n");

	agl::RenderWindow window;
	window.setup({WIDTH, HEIGHT}, "EvolutionSimulator");
	window.setClearColor(CLEARCOLOR);
	window.setFPS(TARGETFPS);

	agl::Vec<float, 2> windowSize;
	windowSize.x = window.getWindowAttributes().width;
	windowSize.y = window.getWindowAttributes().height;

	glDisable(GL_DEPTH_TEST);

	// window.GLEnable(GL_ALPHA_TEST);
	// glAlphaFunc(GL_GREATER, 0.1f);

	PFNGLXSWAPINTERVALSGIPROC glXSwapIntervalSGI =
		(PFNGLXSWAPINTERVALSGIPROC)glXGetProcAddress((const GLubyte *)"glXSwapIntervalSGI");

	glXSwapIntervalSGI(VSYNC);

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

	MenuShare::init(&blank, &font, &smallFont, &event);

	// menu shapes
	Menu<ValueElement<int>, ValueElement<int>, ValueElement<int>, ValueElement<int>, ValueElement<int>,
		 ValueElement<float>>
		simulationInfo("Statistics");
	simulationInfo.setup({WIDTH - 260, 10, 9}, {250, 175});
	simulationInfo.open({(int)windowSize.x - 260, 10, 9});

	struct
	{
			ValueElement<int> *creatures;
			ValueElement<int> *eggs;
			ValueElement<int> *food;
			ValueElement<int> *meat;
			ValueElement<int> *frame;
			ValueElement<int> *fps;
	} simulationInfoPointers;

	simulationInfo.bindPointers(&simulationInfoPointers);

	struct
	{
			NetworkGraph			  *netGraph;
			SpacerElement			  *s1;
			ValueElement<std::string> *node;
			SpacerElement			  *s2;
			TextElement				  *posText;
			ValueElement<float>		  *posx;
			ValueElement<float>		  *posy;
			TextElement				  *velText;
			ValueElement<float>		  *velx;
			ValueElement<float>		  *vely;
			TextElement				  *forText;
			ValueElement<float>		  *forx;
			ValueElement<float>		  *fory;
			SpacerElement			  *s3;
			ValueElement<bool>		  *eating;
			ValueElement<bool>		  *layingEgg;
			SpacerElement			  *s4;
			ValueElement<float>		  *health;
			ValueElement<float>		  *energy;
			ValueElement<int>		  *lifeLeft;
			SpacerElement			  *s5;
			ValueElement<float>		  *sight;
			ValueElement<float>		  *speed;
			ValueElement<float>		  *size;
			ValueElement<int>		  *hue;
			SpacerElement			  *s6;
			ValueElement<float>		  *biomass;
			ValueElement<float>		  *energyDensity;
			ValueElement<float>		  *metabolism;
			ValueElement<float>		  *preference;
	} creatureInfoPointers;

	Menu<NetworkGraph, SpacerElement, ValueElement<std::string>, SpacerElement, TextElement, ValueElement<float>,
		 ValueElement<float>, TextElement, ValueElement<float>, ValueElement<float>, TextElement, ValueElement<float>,
		 ValueElement<float>, SpacerElement, ValueElement<bool>, ValueElement<bool>, SpacerElement, ValueElement<float>,
		 ValueElement<float>, ValueElement<int>, SpacerElement, ValueElement<float>, ValueElement<float>,
		 ValueElement<float>, ValueElement<int>, SpacerElement, ValueElement<float>, ValueElement<float>,
		 ValueElement<float>, ValueElement<float>>
		creatureInfo("CreatureInfo");
	creatureInfo.setup({10, 10, 9}, {400, HEIGHT - (20)});
	creatureInfo.bindPointers(&creatureInfoPointers);
	creatureInfo.open({10, 10});

	creatureInfo.setupElements({nullptr},					//
							   {25},						//
							   {"Node", nullptr},			//
							   {},							//
							   {"- Position -"},			//
							   {"X", nullptr},				//
							   {"Y", nullptr},				//
							   {"- Velocity -"},			//
							   {"X", nullptr},				//
							   {"Y", nullptr},				//
							   {"- Force -"},				//
							   {"X", nullptr},				//
							   {"Y", nullptr},				//
							   {},							//
							   {"Eating", nullptr},			//
							   {"Laying Egg", nullptr},		//
							   {},							//
							   {"Health", nullptr},			//
							   {"Energy", nullptr},			//
							   {"Life Left", nullptr},		//
							   {},							//
							   {"Sight", nullptr},			//
							   {"Speed", nullptr},			//
							   {"Size", nullptr},			//
							   {"Hue", nullptr},			//
							   {},							//
							   {"Biomass", nullptr},		//
							   {"Energy Density", nullptr}, //
							   {"Metabolism", nullptr},		//
							   {"Preference", nullptr}		//
	);

	struct
	{
			TextElement	  *leftclick;
			ButtonElement *food;
			ButtonElement *meat;
			ButtonElement *select;
			ButtonElement *kill;
			FieldElement  *foodDen;
			FieldElement  *foodVol;
			FieldElement  *meatDen;
			FieldElement  *leachVol;
			FieldElement  *maxFood;
			FieldElement  *damage;
			FieldElement  *energyCostMultiplier;
	} actionMenuPointers;

	Menu<TextElement, ButtonElement, ButtonElement, ButtonElement, ButtonElement, FieldElement, FieldElement,
		 FieldElement, FieldElement, FieldElement, FieldElement, FieldElement>
		actionMenu("ActionMenu");
	actionMenu.setup({WIDTH - 150, 10 + 160, 9}, {250, 360});
	actionMenu.bindPointers(&actionMenuPointers);
	actionMenu.open(
		{(int)windowSize.x - actionMenu.size.x - 10, simulationInfo.position.y + simulationInfo.size.y + 10, 9});

	struct
	{
			TextElement	  *text;
			ButtonElement *confirm;
			ButtonElement *cancel;
	} quitMenuPointers;

	Menu<TextElement, ButtonElement, ButtonElement> quitMenu("QuitMenu");
	quitMenu.setup({0, 0}, {150, 115});
	quitMenu.bindPointers(&quitMenuPointers);
	quitMenu.setupElements({"Quit?"},				  //
						   {"Confirm", 150 - 16 * 2}, //
						   {"Cancel", 150 - 16 * 2}	  //
	);

	MenuBar menuBar;

	// simulation entities
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
	nodeNames[FOWARD_OUTPUT]	 = "Move Foward";
	nodeNames[RIGHT_OUTPUT]		 = "Turn Right";
	nodeNames[LEFT_OUTPUT]		 = "Turn Left";
	nodeNames[EAT_OUTPUT]		 = "Eat";
	nodeNames[LAYEGG_OUTPUT]	 = "Lay Egg";
	nodeNames[MEAT_ROTATION]	 = "Rotation To Meat";
	nodeNames[MEAT_DISTANCE]	 = "Distance To Meat";

	for (int i = (TOTAL_INPUT + TOTAL_OUTPUT); i < TOTAL_NODES; i++)
	{
		nodeNames[i] = "Hidden";
	}

	printf("loading simulation rules from sim.conf\n");

	SimulationRules simulationRules;

	loadRules("./conf/sim.conf", &simulationRules);

	std::cout << "startingCreatures - " << simulationRules.startingCreatures << '\n';
	std::cout << "maxCreatures - " << simulationRules.maxCreatures << '\n';
	std::cout << "foodEnergy - " << simulationRules.foodEnergy << '\n';
	std::cout << "maxFood - " << simulationRules.maxFood << '\n';
	std::cout << "size - " << simulationRules.size << '\n';
	std::cout << "gridResolution - " << simulationRules.gridResolution << '\n';
	std::cout << "maxEggs - " << simulationRules.maxEggs << '\n';
	std::cout << "preferedCreatures - " << simulationRules.preferedCreatures << '\n';
	std::cout << "penaltyBuffer- " << simulationRules.penaltyBuffer << '\n';
	std::cout << "penaltyPeriod- " << simulationRules.penaltyPeriod << '\n';

	background.setSize(simulationRules.size);

	printf("starting sim\n");

	// background.setPosition(simulationRules.size * .5);

	Simulation simulation(simulationRules);

	Creature		 *creature			= simulation.creatureBuffer;
	List<Creature *> *existingCreatures = simulation.existingCreatures;
	Egg				 *egg				= simulation.eggBuffer;
	List<Egg *>		 *existingEggs		= simulation.existingEggs;
	Food			 *food				= simulation.foodBuffer;

	Creature *focusCreature = nullptr;

	float		fps = 0;
	std::string nodeName;

	simulationInfo.setupElements({"Creatures", &simulation.existingCreatures->length}, //
								 {"Eggs", &simulation.existingEggs->length},		   //
								 {"Food", &simulation.existingFood->length},		   //
								 {"Meat", &simulation.existingMeat->length},		   //
								 {"Frame", &simulation.frame},						   //
								 {"FPS", &fps}										   //
	);

	creatureInfoPointers.size->value = (float *)0;

	auto setValues = [&]() {
		creatureInfoPointers.netGraph->network	  = focusCreature->network;
		creatureInfoPointers.node->value		  = &nodeName;
		creatureInfoPointers.posx->value		  = &focusCreature->position.x;
		creatureInfoPointers.posy->value		  = &focusCreature->position.y;
		creatureInfoPointers.velx->value		  = &focusCreature->velocity.x;
		creatureInfoPointers.vely->value		  = &focusCreature->velocity.y;
		creatureInfoPointers.forx->value		  = &focusCreature->force.x;
		creatureInfoPointers.fory->value		  = &focusCreature->force.y;
		creatureInfoPointers.eating->value		  = &focusCreature->eating;
		creatureInfoPointers.layingEgg->value	  = &focusCreature->layingEgg;
		creatureInfoPointers.health->value		  = &focusCreature->health;
		creatureInfoPointers.energy->value		  = &focusCreature->energy;
		creatureInfoPointers.lifeLeft->value	  = &focusCreature->life;
		creatureInfoPointers.sight->value		  = &focusCreature->sight;
		creatureInfoPointers.speed->value		  = &focusCreature->speed;
		creatureInfoPointers.size->value		  = &focusCreature->size;
		creatureInfoPointers.hue->value			  = &focusCreature->hue;
		creatureInfoPointers.biomass->value		  = &focusCreature->biomass;
		creatureInfoPointers.energyDensity->value = &focusCreature->energyDensity;
		creatureInfoPointers.metabolism->value	  = &focusCreature->metabolism;
		creatureInfoPointers.preference->value	  = &focusCreature->preference;
	};

	bool mHeld		= false;
	bool b1Held		= false;
	bool ReturnHeld = false;

	bool skipRender = false;

	float sizeMultiplier = 1;

	actionMenu.setupElements({"Left Click"}, {"Food", 150 - 16 * 2},					 //
							 {"Meat", 150 - 16 * 2},									 //
							 {"Select", 150 - 16 * 2},									 //
							 {"Kill", 150 - 16 * 2},									 //
							 {"FdEnDn", std::to_string(simulation.foodEnergyDensity)},	 //
							 {"FdVol", std::to_string(simulation.foodVol)},				 //
							 {"MtEnDn", std::to_string(simulation.meatEnergyDensity)},	 //
							 {"LeVol", std::to_string(simulation.leachVol)},			 //
							 {"maxFd", std::to_string(simulation.foodCap)},				 //
							 {"dmg", std::to_string(simulation.damage)},				 //
							 {"EnCoMu", std::to_string(simulation.energyCostMultiplier)} //
	);

	printf("entering sim loop\n");

	bool quiting = false;

	quitMenu.open({});

	while (!event.windowClose())
	{
		static int milliDiff = 0;
		int		   start	 = getMillisecond();

		event.poll();

		if (event.isKeyPressed(XK_m))
		{
			mHeld = true;
		}
		else if (mHeld)
		{
			mHeld = false;

			CreatureData creatureData(1, 1, 1, 60, 15);

			creatureData.setConnection(0, CONSTANT_INPUT, FOWARD_OUTPUT, 1);
			creatureData.setConnection(1, CONSTANT_INPUT, EAT_OUTPUT, 1);
			creatureData.setConnection(2, CONSTANT_INPUT, LAYEGG_OUTPUT, 1);
			creatureData.setConnection(3, CREATURE_ROTATION, LEFT_OUTPUT, 1);
			creatureData.setConnection(4, CREATURE_ROTATION, RIGHT_OUTPUT, -1);
			creatureData.setConnection(5, CONSTANT_INPUT, RIGHT_OUTPUT, -1);
			creatureData.setConnection(6, CONSTANT_INPUT, LEFT_OUTPUT, -1);
			creatureData.setConnection(7, CREATURE_PREFERENCE, RIGHT_OUTPUT, 1);
			creatureData.setConnection(8, CREATURE_PREFERENCE, LEFT_OUTPUT, 1);

			creatureData.preference = 0;
			creatureData.metabolism = METABOLISM;

			agl::Vec<float, 2> position;
			position.x = (rand() / (float)RAND_MAX) * simulationRules.size.x;
			position.y = (rand() / (float)RAND_MAX) * simulationRules.size.y;

			simulation.addCreature(creatureData, position);
		}

		if (event.isKeyPressed(XK_Return))
		{
			ReturnHeld = true;
		}
		else if (ReturnHeld)
		{
			ReturnHeld = false;
			skipRender = !skipRender;
		}

		if (!event.isKeyPressed(XK_space))
		{
			simulation.update();
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

		window.updateMvp(camera);

		// Draw food
		for (int i = 0; i < simulation.existingFood->length; i++)
		{
			agl::Vec<float, 2> position = simulation.existingFood->get(i)->position;
			foodShape.setPosition(position);
			window.drawShape(foodShape);
		}

		for (int i = 0; i < simulation.existingMeat->length; i++)
		{
			Meat *meat = simulation.existingMeat->get(i);

			meatShape.setPosition(meat->position);
			meatShape.setSize({meat->radius * 2, meat->radius * 2});
			meatShape.setOffset({-meat->radius, -meat->radius});
			meatShape.setRotation({0, 0, meat->rotation});
			window.drawShape(meatShape);
		}

		// draw eggs
		for (int i = 0; i < existingEggs->length; i++)
		{
			eggShape.setPosition(existingEggs->get(i)->position);
			window.drawShape(eggShape);
		}

		// draw rays
		if (existingCreatures->find(focusCreature) != -1)
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
		for (int i = 0; i < existingCreatures->length; i++)
		{
			creatureShape.setPosition(existingCreatures->get(i)->position);
			creatureShape.setRotation(agl::Vec<float, 3>{0, 0, -float(existingCreatures->get(i)->rotation * 180 / PI)});

			float speed = existingCreatures->get(i)->velocity.length();

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

			if (event.isKeyPressed(XK_z))
			{
				agl::Vec<float, 3> blue =
					agl::Vec<float, 3>{0, 0, 255} * existingCreatures->get(i)->creatureData.preference;
				agl::Vec<float, 3> yellow =
					agl::Vec<float, 3>{255, 255, 0} * (1 - existingCreatures->get(i)->creatureData.preference);

				creatureShape.setColor({(unsigned char)(blue.x + yellow.x), (unsigned char)(blue.y + yellow.y),
										(unsigned char)(blue.z + yellow.z)});
			}
			else
			{
				creatureShape.setColor(hueToRGB(existingCreatures->get(i)->hue));
			}

			float size = existingCreatures->get(i)->size;

			creatureShape.setSize(agl::Vec<float, 3>{25 * size, 60 * size, 0});
			creatureShape.setOffset(agl::Vec<float, 3>{(float)-12.5 * size, (float)-12.5 * size, -1});

			window.drawShape(creatureShape);

			creatureShape.setTexture(&creatureExtraTexture);

			creatureShape.setColor(agl::Color::White);

			creatureShape.setTextureScaling({1, 1});
			creatureShape.setTextureTranslation({1, 1});

			creatureShape.setOffset(agl::Vec<float, 3>{(float)-12.5 * size, (float)-12.5 * size, -.5});

			window.drawShape(creatureShape);
		}

		// gui rendering

		fps = (1000. / milliDiff);

		window.updateMvp(guiCamera);

		window.draw(simulationInfo);

		window.draw(actionMenu);

		if (quiting)
		{
			window.draw(quitMenu);
		}

		window.draw(menuBar);

		if (existingCreatures->find(focusCreature) != -1)
		{
			nodeName			  = nodeNames[creatureInfoPointers.netGraph->selectedID];

			setValues();

			window.draw(creatureInfo);
		}
		else
		{
			focusCreature = nullptr;
		}

		window.display();

	skipRendering:;

		// quit?

		if (quitMenuPointers.confirm->state)
		{
			window.close();
			break;
		}
		if (quitMenuPointers.cancel->state)
		{
			quiting						   = false;
			quitMenuPointers.cancel->state = false;
		}

		// input

		if (event.isKeyPressed(XK_r))
		{
			focusCreature = nullptr;
		}

		if (event.isKeyPressed(XK_Escape))
		{
			quiting = true;
		}

		if (event.isPointerButtonPressed(Button1Mask))
		{
			if (pointInArea(event.getPointerWindowPosition(), simulationInfo.position, simulationInfo.size))
			{
				goto endif;
			}
			if (pointInArea(event.getPointerWindowPosition(), actionMenu.position, actionMenu.size))
			{
				goto endif;
			}
			if (focusCreature != nullptr &&
				pointInArea(event.getPointerWindowPosition(), creatureInfo.position, creatureInfo.size))
			{
				goto endif;
			}
			if (quiting && pointInArea(event.getPointerWindowPosition(), quitMenu.position, quitMenu.size))
			{
				goto endif;
			}

			if (actionMenuPointers.food->state) // add food
			{
				simulation.addFood(getCursorScenePosition(event.getPointerWindowPosition(), windowSize, sizeMultiplier,
														  cameraPosition));
			}
			if (actionMenuPointers.meat->state) // add meat
			{
				simulation.addMeat(getCursorScenePosition(event.getPointerWindowPosition(), windowSize, sizeMultiplier,
														  cameraPosition));
			}
			if (actionMenuPointers.select->state) // select creature
			{
				for (int i = 0; i < existingCreatures->length; i++)
				{
					agl::Vec<float, 2> mouse;
					mouse.x = ((event.getPointerWindowPosition().x - (windowSize.x * .5)) * sizeMultiplier) +
							  cameraPosition.x;
					mouse.y = ((event.getPointerWindowPosition().y - (windowSize.y * .5)) * sizeMultiplier) +
							  cameraPosition.y;

					float distance = (mouse - existingCreatures->get(i)->position).length();

					if (distance < existingCreatures->get(i)->radius)
					{
						focusCreature = existingCreatures->get(i);

						break;
					}
				}
			}
			if (actionMenuPointers.kill->state) // kill creature
			{
				for (int i = 0; i < existingCreatures->length; i++)
				{
					agl::Vec<float, 2> mouse;
					mouse.x = ((event.getPointerWindowPosition().x - (windowSize.x * .5)) * sizeMultiplier) +
							  cameraPosition.x;
					mouse.y = ((event.getPointerWindowPosition().y - (windowSize.y * .5)) * sizeMultiplier) +
							  cameraPosition.y;

					float distance = (mouse - existingCreatures->get(i)->position).length();

					if (distance < existingCreatures->get(i)->radius)
					{
						simulation.addMeat(existingCreatures->get(i)->position,
										   existingCreatures->get(i)->maxHealth / 4);
						existingCreatures->pop(i);

						break;
					}
				}
			}

		endif:;
		}

		// camera movement

		static agl::Vec<float, 2> cameraOffset;
		static agl::Vec<float, 2> startPos;

		if (event.isPointerButtonPressed(Button2Mask))
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
				window.setCursorShape(58);
				startPos = event.getPointerWindowPosition();
				b1Held	 = true;
			}
		}
		else if (b1Held) // let go
		{
			window.setCursorShape(XC_left_ptr);
			cameraOffset = {0, 0};
			b1Held		 = false;
		}

		static float cameraSpeed = 4;

		const float sizeDelta = .2;

		if (event.pointerButton == 4)
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
		if (event.pointerButton == 5)
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

		quitMenu.position =
			agl::Vec<int, 2>{int(windowSize.x - quitMenu.size.x) / 2, int(windowSize.y - quitMenu.size.y) / 2};

		window.setViewport(0, 0, windowSize.x, windowSize.y);

		camera.setOrthographicProjection(-((windowSize.x / 2.) * sizeMultiplier),
										 ((windowSize.x / 2.) * sizeMultiplier), ((windowSize.y / 2.) * sizeMultiplier),
										 -((windowSize.y / 2.) * sizeMultiplier), 0.1, 100);
		camera.setView({cameraPosition.x, cameraPosition.y, 50}, {cameraPosition.x, cameraPosition.y, 0}, {0, 1, 0});

		guiCamera.setOrthographicProjection(0, windowSize.x, windowSize.y, 0, 0.1, 100);

		simulation.foodEnergyDensity	= std::stof(actionMenuPointers.foodDen->value);
		simulation.foodVol				= std::stof(actionMenuPointers.foodVol->value);
		simulation.meatEnergyDensity	= std::stof(actionMenuPointers.meatDen->value);
		simulation.leachVol				= std::stof(actionMenuPointers.leachVol->value);
		simulation.foodCap				= std::stof(actionMenuPointers.maxFood->value);
		simulation.damage				= std::stof(actionMenuPointers.damage->value);
		simulation.energyCostMultiplier = std::stof(actionMenuPointers.energyCostMultiplier->value);

		milliDiff = getMillisecond() - start;
	}

	simulation.destroy();

	MenuShare::destroy();

	font.deleteFont();

	foodTexture.deleteTexture();
	creatureBodyTexture.deleteTexture();
	creatureExtraTexture.deleteTexture();
	eggTexture.deleteTexture();
	blank.deleteTexture();

	simpleShader.deleteProgram();
	gridShader.deleteProgram();

	if (!quiting)
	{
		window.close();
	}

	return 0;
}
