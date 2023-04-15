#include "../lib/AGL/agl.hpp"

#include "../inc/Menu.hpp"
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

#define TOTAL_FOOD 10

#define BASE_B_VALUE 63

#define WIDTH  1920
#define HEIGHT 1080

#define CLEARCOLOR \
	{              \
		12, 12, 24 \
	}

#define NETWORK_PADDING 20

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
	// window.setFPS(60);

	glDisable(GL_DEPTH_TEST);

	// window.GLEnable(GL_ALPHA_TEST);
	// glAlphaFunc(GL_GREATER, 0.1f);

	XSelectInput(window.getDisplay(), window.getWindow(), ButtonPressMask | ButtonReleaseMask);

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
	font.setup("./font/VCR_OSD_MONO_1.001.ttf", 24);

	agl::Rectangle background;
	background.setTexture(&blank);
	background.setColor(CLEARCOLOR);

	background.setPosition({0, 0, 0});

	// menu shapes
	Menu<ValueElement<int>, ValueElement<int>, ValueElement<int>, ValueElement<int>, ValueElement<int>,
		 ValueElement<float>>
		simulationInfo(&blank, &font, &event);
	simulationInfo.setup({WIDTH - 260, 10, 9}, {250, 175});

	Menu<SpacerElement, ValueElement<std::string>, SpacerElement, TextElement, ValueElement<float>, ValueElement<float>,
		 TextElement, ValueElement<float>, ValueElement<float>, TextElement, ValueElement<float>, ValueElement<float>,
		 SpacerElement, ValueElement<bool>, ValueElement<bool>, SpacerElement, ValueElement<float>, ValueElement<float>,
		 ValueElement<int>, SpacerElement, ValueElement<float>, ValueElement<float>, ValueElement<float>,
		 ValueElement<int>, SpacerElement, ValueElement<float>, ValueElement<float>, ValueElement<float>,
		 ValueElement<float>>
		creatureInfo(&blank, &font, &event);
	creatureInfo.setup({10, 10, 9}, {400, HEIGHT - (20)});

	creatureInfo.get<0>().height = 350;
	creatureInfo.get<3>().str	 = "- Position -";
	creatureInfo.get<6>().str	 = "- Velocity -";
	creatureInfo.get<9>().str	 = "- Force -";

	float vel = 0;

	creatureInfo.get<27>() = {"vel", &vel};

	Menu<ButtonElement, ButtonElement, ButtonElement, ButtonElement> actionMenu(&blank, &font, &event);
	actionMenu.setup({WIDTH - 150, 10 + 160, 9}, {150, 140});
	actionMenu.get<0>().label = "Food";
	actionMenu.get<0>().width = 150 - 16 * 2;
	actionMenu.get<1>().label = "Meat";
	actionMenu.get<1>().width = 150 - 16 * 2;
	actionMenu.get<2>().label = "Select";
	actionMenu.get<2>().width = 150 - 16 * 2;
	actionMenu.get<3>().label = "Kill";
	actionMenu.get<3>().width = 150 - 16 * 2;

	Menu<TextElement, ButtonElement, ButtonElement> quitMenu(&blank, &font, &event);
	quitMenu.setup({0, 0}, {150, 115});
	quitMenu.get<0>().str	= "Quit?";
	quitMenu.get<1>().label = "Confirm";
	quitMenu.get<1>().width = 150 - 16 * 2;
	quitMenu.get<2>().label = "Cancel";
	quitMenu.get<2>().width = 150 - 16 * 2;

	agl::Circle networkBackground(60);
	networkBackground.setTexture(&blank);
	networkBackground.setColor({15, 15, 15});
	networkBackground.setSize(agl::Vec<float, 3>{150, 150, 0});
	networkBackground.setPosition(agl::Vec<float, 3>{
		creatureInfo.position.x + (creatureInfo.size.x / 2),
		creatureInfo.position.y + MENU_BORDER + MENU_PADDING + MENU_SHADOWSIZE + networkBackground.getSize().y, 10});

	agl::Circle nodeShape(10);
	nodeShape.setTexture(&blank);
	nodeShape.setSize(agl::Vec<float, 3>{10, 10, 0});
	nodeShape.setPosition(agl::Vec<float, 3>{500, 500, 3});
	nodeShape.setOffset({0, 0, float(networkBackground.getPosition().z + 0.2)});

	agl::Rectangle connectionShape;
	connectionShape.setTexture(&blank);
	connectionShape.setColor(agl::Color::Red);
	connectionShape.setSize(agl::Vec<float, 3>{1, 50, 2});
	connectionShape.setOffset({0, 0, float(networkBackground.getPosition().z + 0.1)});

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

	Creature *focusCreature	   = nullptr;

	float		fps	  = 0;
	std::string nodeName;

	simulationInfo.get<0>() = {"Creatures", &simulation.existingCreatures->length};
	simulationInfo.get<1>() = {"Eggs", &simulation.existingEggs->length};
	simulationInfo.get<2>() = {"Food", &simulation.existingFood->length};
	simulationInfo.get<3>() = {"Meat", &simulation.existingMeat->length};
	simulationInfo.get<4>() = {"Frame", &simulation.frame};
	simulationInfo.get<5>() = {"FPS", &fps};

	auto setValues = [&]() {
		creatureInfo.get<1>()  = {"Node", &nodeName};
		creatureInfo.get<4>()  = {"X", &focusCreature->position.x};
		creatureInfo.get<5>()  = {"Y", &focusCreature->position.y};
		creatureInfo.get<7>()  = {"X", &focusCreature->velocity.x};
		creatureInfo.get<8>()  = {"Y", &focusCreature->velocity.y};
		creatureInfo.get<10>() = {"X", &focusCreature->force.x};
		creatureInfo.get<11>() = {"Y", &focusCreature->force.y};
		creatureInfo.get<13>() = {"Eating", &focusCreature->eating};
		creatureInfo.get<14>() = {"Laying Egg", &focusCreature->layingEgg};
		creatureInfo.get<16>() = {"Health", &focusCreature->health};
		creatureInfo.get<17>() = {"Energy", &focusCreature->energy};
		creatureInfo.get<18>() = {"Life Left", &focusCreature->life};
		creatureInfo.get<20>() = {"Sight", &focusCreature->sight};
		creatureInfo.get<21>() = {"Speed", &focusCreature->speed};
		creatureInfo.get<22>() = {"Size", &focusCreature->size};
		creatureInfo.get<23>() = {"Hue", &focusCreature->hue};
		creatureInfo.get<25>() = {"Biomass", &focusCreature->biomass};
		creatureInfo.get<26>() = {"Energy Density", &focusCreature->energyDensity};
		creatureInfo.get<27>() = {"Egg Cost", &focusCreature->eggCost};
		creatureInfo.get<28>() = {"Egg Deposit", &focusCreature->eggDesposit};
	};

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

		int mouseWheelPos = 0;

		event.poll([&](XEvent xev) {
			switch (xev.type)
			{
				case ButtonPress:
					if (xev.xbutton.button == 4)
					{
						mouseWheelPos = 1;
					}
					if (xev.xbutton.button == 5)
					{
						mouseWheelPos = -1;
					}
					break;
			}
		});

		if (event.isKeyPressed(XK_m))
		{
			mHeld = true;
		}
		else if (mHeld)
		{
			mHeld = false;

			CreatureData creatureData(1, 2, 1, 60, 15);

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
			meatShape.setPosition(simulation.existingMeat->get(i)->position);
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
			{
				float angleOffset = focusCreature->network->getNode(CREATURE_ROTATION).value * 180;
				angleOffset += 180;

				float rayAngle = angleOffset - agl::radianToDegree(focusCreature->rotation);

				float weight = focusCreature->network->getNode(CREATURE_DISTANCE).value;

				rayShape.setColor({0, (unsigned char)(weight * 255), BASE_B_VALUE});

				rayShape.setPosition(focusCreature->position);
				rayShape.setRotation(agl::Vec<float, 3>{0, 0, rayAngle});
				window.drawShape(rayShape);
			}
			{
				float angleOffset = focusCreature->network->getNode(FOOD_ROTATION).value * 180;
				angleOffset += 180;

				float rayAngle = angleOffset - agl::radianToDegree(focusCreature->rotation);

				float weight = focusCreature->network->getNode(FOOD_DISTANCE).value;

				rayShape.setColor({0, (unsigned char)(weight * 255), BASE_B_VALUE});

				rayShape.setPosition(focusCreature->position);
				rayShape.setRotation(agl::Vec<float, 3>{0, 0, rayAngle});
				window.drawShape(rayShape);
			}
			{
				float angleOffset = focusCreature->network->getNode(MEAT_ROTATION).value * 180;
				angleOffset += 180;

				float rayAngle = angleOffset - agl::radianToDegree(focusCreature->rotation);

				float weight = focusCreature->network->getNode(MEAT_DISTANCE).value;

				rayShape.setColor({0, (unsigned char)(weight * 255), BASE_B_VALUE});

				rayShape.setPosition(focusCreature->position);
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
				if (existingCreatures->get(i)->creatureData.preference > .5)
				{
					creatureShape.setColor(agl::Color::Blue);
				}
				else
				{
					creatureShape.setColor(agl::Color::Yellow);
				}
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

		if (existingCreatures->find(focusCreature) != -1)
		{
			static int selectedID = 0;
			nodeName			  = nodeNames[selectedID];

			setValues();

			window.draw(creatureInfo);
			window.drawShape(networkBackground);

			// draw node connections
			for (int i = 0; i < focusCreature->network->getTotalConnections(); i++)
			{
				Connection connection = focusCreature->network->getConnection(i);

				if (!connection.valid)
				{
					continue;
				}

				float startAngle = connection.startNode + 1;
				startAngle /= focusCreature->network->getTotalNodes();
				startAngle *= PI * 2;

				float endAngle = connection.endNode + 1;
				endAngle /= focusCreature->network->getTotalNodes();
				endAngle *= PI * 2;

				agl::Vec<float, 2> startPosition = agl::pointOnCircle(startAngle);
				startPosition.x = (startPosition.x * (networkBackground.getSize().x - NETWORK_PADDING)) +
								  networkBackground.getPosition().x;
				startPosition.y = (startPosition.y * (networkBackground.getSize().x - NETWORK_PADDING)) +
								  networkBackground.getPosition().y;

				agl::Vec<float, 2> endPosition = agl::pointOnCircle(endAngle);
				endPosition.x				   = (endPosition.x * (networkBackground.getSize().x - NETWORK_PADDING)) +
								networkBackground.getPosition().x;
				endPosition.y = (endPosition.y * (networkBackground.getSize().x - NETWORK_PADDING)) +
								networkBackground.getPosition().y;

				agl::Vec<float, 2> offset = startPosition - endPosition;

				float angle = agl::radianToDegree(vectorAngle(offset)) + 180;

				float weight = connection.weight;

				if (weight > 0)
				{
					connectionShape.setColor({0, (unsigned char)(weight * 255), BASE_B_VALUE});
				}
				else
				{
					connectionShape.setColor({(unsigned char)(-weight * 255), 0, BASE_B_VALUE});
				}

				connectionShape.setSize(agl::Vec<float, 2>{2, offset.length()});
				connectionShape.setPosition(startPosition);
				connectionShape.setRotation(agl::Vec<float, 3>{0, 0, angle});
				window.drawShape(connectionShape);
			}

			// draw nodes
			for (int i = 0; i < focusCreature->network->getTotalNodes(); i++)
			{
				float angle = (360. / focusCreature->network->getTotalNodes()) * (i + 1);

				float x = cos(angle * (3.14159 / 180));
				float y = sin(angle * (3.14159 / 180));

				agl::Vec<float, 2> pos;
				pos.x = x * (networkBackground.getSize().x - NETWORK_PADDING);
				pos.y = y * (networkBackground.getSize().x - NETWORK_PADDING);

				pos.x += networkBackground.getPosition().x;
				pos.y += networkBackground.getPosition().y;

				nodeShape.setPosition(pos);

				float nodeValue = focusCreature->network->getNode(i).value;

				if (nodeValue > 0)
				{
					nodeShape.setColor({0, (unsigned char)(nodeValue * 255), BASE_B_VALUE});
				}
				else
				{
					nodeShape.setColor({(unsigned char)(-nodeValue * 255), 0, BASE_B_VALUE});
				}

				if ((pos - event.getPointerWindowPosition()).length() < 10)
				{
					selectedID = i;
				}

				window.drawShape(nodeShape);
			}
		}
		else
		{
			focusCreature = nullptr;
		}

		window.display();

	skipRendering:;

		// quit?

		if (quitMenu.get<1>().state)
		{
			window.close();
			break;
		}
		if (quitMenu.get<2>().state)
		{
			quiting					= false;
			quitMenu.get<2>().state = false;
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

		static agl::Vec<float, 2> windowSize;
		windowSize.x = window.getWindowAttributes().width;
		windowSize.y = window.getWindowAttributes().height;

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

			if (actionMenu.get<0>().state) // add food
			{
				simulation.addFood(getCursorScenePosition(event.getPointerWindowPosition(), windowSize, sizeMultiplier,
														  cameraPosition));
			}
			if (actionMenu.get<1>().state) // add meat
			{
				simulation.addMeat(getCursorScenePosition(event.getPointerWindowPosition(), windowSize, sizeMultiplier,
														  cameraPosition));
			}
			if (actionMenu.get<2>().state) // select creature
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
			if (actionMenu.get<3>().state) // kill creature
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
						simulation.addMeat(existingCreatures->get(i)->position);
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

		if (mouseWheelPos == 1)
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
		if (mouseWheelPos == -1)
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

		simulationInfo.setPosition({windowSize.x - 260, 10, 9});
		actionMenu.setPosition({windowSize.x - 160, simulationInfo.position.y + simulationInfo.size.y + 10, 9});
		quitMenu.setPosition({(windowSize.x - quitMenu.size.x) / 2, (windowSize.y - quitMenu.size.y) / 2});

		window.setViewport(0, 0, windowSize.x, windowSize.y);

		camera.setOrthographicProjection(-((windowSize.x / 2.) * sizeMultiplier),
										 ((windowSize.x / 2.) * sizeMultiplier), ((windowSize.y / 2.) * sizeMultiplier),
										 -((windowSize.y / 2.) * sizeMultiplier), 0.1, 100);
		camera.setView({cameraPosition.x, cameraPosition.y, 50}, {cameraPosition.x, cameraPosition.y, 0}, {0, 1, 0});

		guiCamera.setOrthographicProjection(0, windowSize.x, windowSize.y, 0, 0.1, 100);

		milliDiff = getMillisecond() - start;
	}

	simulation.destroy();

	simulationInfo.destroy();
	creatureInfo.destroy();

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
