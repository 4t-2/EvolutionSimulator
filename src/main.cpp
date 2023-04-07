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
	for (int i = 0; i < creatureData.getTotalConnections(); i++)
	{
		printf("connection %d going from %d to %d with weight %f\n", i, creatureData.getConnection()[i].startNode,
			   creatureData.getConnection()[i].endNode, creatureData.getConnection()[i].weight);
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
	std::cout << gridShader.loadFromFile("./shader/gridvert.glsl", "./shader/grid.glsl") << '\n';

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

	agl::Texture blank;
	blank.setBlank();

	agl::Font font;
	font.setup("./font/VCR_OSD_MONO_1.001.ttf", 24);

	agl::Rectangle background;
	background.setTexture(&blank);
	background.setColor(CLEARCOLOR);

	float backgroundSize = 100000;

	background.setSize({backgroundSize, backgroundSize});
	background.setPosition({0, 0, 0});
	background.setOffset({-backgroundSize / 2, -backgroundSize / 2, -40});

	// menu shapes
	Menu<ValueElement, ValueElement, ValueElement, ValueElement, ValueElement> simulationInfo(&blank, &font);
	simulationInfo.setup({WIDTH - 260, 10, 9}, {250, 150});
	simulationInfo.get<0>().label = "Creatures";
	simulationInfo.get<1>().label = "Eggs";
	simulationInfo.get<2>().label = "Food";
	simulationInfo.get<3>().label = "Frame";
	simulationInfo.get<4>().label = "FPS";

	Menu<SpacerElement, ValueElement, SpacerElement, TextElement, ValueElement, ValueElement, TextElement, ValueElement,
		 ValueElement, TextElement, ValueElement, ValueElement, SpacerElement, ValueElement, ValueElement,
		 SpacerElement, ValueElement, ValueElement, ValueElement, SpacerElement, ValueElement, ValueElement,
		 ValueElement, ValueElement, TextElement>
		creatureInfo(&blank, &font);
	creatureInfo.setup({10, 10, 9}, {400, HEIGHT - (20)});

	creatureInfo.get<0>().height = 350;
	creatureInfo.get<1>().label	 = "Node";

	creatureInfo.get<3>().str	 = "- Position -";
	creatureInfo.get<4>().label	 = "X";
	creatureInfo.get<5>().label	 = "Y";
	creatureInfo.get<6>().str	 = "- Velocity -";
	creatureInfo.get<7>().label	 = "X";
	creatureInfo.get<8>().label	 = "Y";
	creatureInfo.get<9>().str	 = "- Acceleration -";
	creatureInfo.get<10>().label = "X";
	creatureInfo.get<11>().label = "Y";

	creatureInfo.get<13>().label = "Eating";
	creatureInfo.get<14>().label = "Laying Egg";

	creatureInfo.get<16>().label = "Health";
	creatureInfo.get<17>().label = "Energy";
	creatureInfo.get<18>().label = "Life Left";

	creatureInfo.get<20>().label = "Sight";
	creatureInfo.get<21>().label = "Speed";
	creatureInfo.get<22>().label = "Size";
	creatureInfo.get<23>().label = "Hue";

	creatureInfo.get<24>().str = "";

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

	printf("starting sim\n");

	// background.setPosition(simulationRules.size * .5);

	Simulation simulation(simulationRules);

	Creature		 *creature			= simulation.getCreatureBuffer();
	List<Creature *> *existingCreatures = simulation.getExistingCreatures();
	Egg				 *egg				= simulation.getEggBuffer();
	List<Egg *>		 *existingEggs		= simulation.getExistingEggs();
	Food			 *food				= simulation.getFoodBuffer();

	Creature *focusCreature;

	bool mHeld		= false;
	bool b1Held		= false;
	bool ReturnHeld = false;

	bool skipRender = false;

	int frame = 0;

	float sizeMultiplier = 1;

	printf("entering sim loop\n");

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

			existingCreatures->pop(0);
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

			frame++;

			// std::cout << frame << '\n';
		}

		agl::Vec<float, 3> zOffset = {0, 0, 0};

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
		for (int i = 0; i < simulation.getExistingFood()->getLength(); i++)
		{
			foodShape.setColor(agl::Color::Green);

			agl::Vec<float, 2> position = simulation.getExistingFood()->get(i)->position;
			foodShape.setPosition(zOffset + position);
			window.drawShape(foodShape);
		}

		// draw eggs
		for (int i = 0; i < existingEggs->getLength(); i++)
		{
			eggShape.setPosition(zOffset + existingEggs->get(i)->getPosition());
			window.drawShape(eggShape);
		}

		// draw rays
		if (existingCreatures->find(focusCreature) != -1)
		{
			{
				float angleOffset = focusCreature->getNeuralNetwork().getNode(CREATURE_ROTATION).value * 180;
				angleOffset += 180;

				float weight = focusCreature->getNeuralNetwork().getNode(CREATURE_DISTANCE).value;

				rayShape.setColor({0, (unsigned char)(weight * 255), BASE_B_VALUE});
				rayShape.setSize(agl::Vec<float, 3>{1, RAY_LENGTH * focusCreature->getSight()});
				rayShape.setPosition(focusCreature->position);
				rayShape.setRotation(
					agl::Vec<float, 3>{0, 0, angleOffset - agl::radianToDegree(focusCreature->getRotation())});
				window.drawShape(rayShape);
			}

			{
				float angleOffset = focusCreature->getNeuralNetwork().getNode(FOOD_ROTATION).value * 180;
				angleOffset += 180;

				float rayAngle = angleOffset - agl::radianToDegree(focusCreature->getRotation());

				float weight = focusCreature->getNeuralNetwork().getNode(FOOD_DISTANCE).value;

				rayShape.setColor({0, (unsigned char)(weight * 255), BASE_B_VALUE});

				rayShape.setPosition(focusCreature->position);
				rayShape.setRotation(agl::Vec<float, 3>{0, 0, rayAngle});
				window.drawShape(rayShape);
			}
		}

		// draw creature
		for (int i = 0; i < existingCreatures->getLength(); i++)
		{
			creatureShape.setPosition(zOffset + existingCreatures->get(i)->position);
			creatureShape.setRotation(
				agl::Vec<float, 3>{0, 0, -float(existingCreatures->get(i)->getRotation() * 180 / PI)});

			float speed = existingCreatures->get(i)->position.length() / 10;

			creatureShape.setTexture(&creatureBodyTexture);

			int textureFrame = int(frame * speed) % 6;

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

			creatureShape.setColor(hueToRGB(existingCreatures->get(i)->getHue()));

			float size = existingCreatures->get(i)->getSize();

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

		window.updateMvp(guiCamera);

		{
			simulationInfo.get<0>().value = std::to_string(simulation.getExistingCreatures()->getLength());
			simulationInfo.get<1>().value = std::to_string(simulation.getExistingEggs()->getLength());
			simulationInfo.get<2>().value = std::to_string(simulation.getExistingFood()->getLength());
			simulationInfo.get<3>().value = std::to_string(frame);
			simulationInfo.get<4>().value = std::to_string(1000. / milliDiff);
		}

		window.draw(simulationInfo);

		if (existingCreatures->find(focusCreature) != -1)
		{
			static int selectedID = 0;

			creatureInfo.get<1>().value	 = nodeNames[selectedID];
			creatureInfo.get<4>().value	 = std::to_string(focusCreature->position.x);
			creatureInfo.get<5>().value	 = std::to_string(focusCreature->position.y);
			creatureInfo.get<7>().value	 = std::to_string(focusCreature->velocity.x);
			creatureInfo.get<8>().value	 = std::to_string(focusCreature->velocity.y);
			creatureInfo.get<10>().value = std::to_string(focusCreature->force.x / focusCreature->mass);
			creatureInfo.get<11>().value = std::to_string(focusCreature->force.y / focusCreature->mass);
			creatureInfo.get<13>().value = std::to_string(focusCreature->getEating());
			creatureInfo.get<14>().value = std::to_string(focusCreature->getLayingEgg());
			creatureInfo.get<16>().value = std::to_string(focusCreature->getHealth());
			creatureInfo.get<17>().value = std::to_string(focusCreature->getEnergy());
			creatureInfo.get<18>().value = std::to_string(focusCreature->getLifeLeft());
			creatureInfo.get<20>().value = std::to_string(focusCreature->getSight());
			creatureInfo.get<21>().value = std::to_string(focusCreature->getSpeed());
			creatureInfo.get<22>().value = std::to_string(focusCreature->getSize());
			creatureInfo.get<23>().value = std::to_string(focusCreature->getHue());
			creatureInfo.get<24>().str = std::to_string(focusCreature->getCreatureData().preference);

			window.draw(creatureInfo);

			window.drawShape(networkBackground);

			// draw node connections
			for (int i = 0; i < focusCreature->getNeuralNetwork().getTotalConnections(); i++)
			{
				Connection connection = focusCreature->getNeuralNetwork().getConnection(i);

				if (!connection.valid)
				{
					continue;
				}

				float startAngle = connection.startNode + 1;
				startAngle /= focusCreature->getNeuralNetwork().getTotalNodes();
				startAngle *= PI * 2;

				float endAngle = connection.endNode + 1;
				endAngle /= focusCreature->getNeuralNetwork().getTotalNodes();
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
			for (int i = 0;
				 i < existingCreatures->get(existingCreatures->find(focusCreature))->getNeuralNetwork().getTotalNodes();
				 i++)
			{
				float angle = (360. / focusCreature->getNeuralNetwork().getTotalNodes()) * (i + 1);

				float x = cos(angle * (3.14159 / 180));
				float y = sin(angle * (3.14159 / 180));

				agl::Vec<float, 2> pos;
				pos.x = x * (networkBackground.getSize().x - NETWORK_PADDING);
				pos.y = y * (networkBackground.getSize().x - NETWORK_PADDING);

				pos.x += networkBackground.getPosition().x;
				pos.y += networkBackground.getPosition().y;

				nodeShape.setPosition(pos);

				float nodeValue = focusCreature->getNeuralNetwork().getNode(i).value;

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

		if (event.isKeyPressed(XK_r))
		{
			focusCreature = nullptr;
		}

		static agl::Vec<float, 2> size;
		size.x = window.getWindowAttributes().width;
		size.y = window.getWindowAttributes().height;

		if (event.isPointerButtonPressed(Button1Mask))
		{
			for (int i = 0; i < existingCreatures->getLength(); i++)
			{
				agl::Vec<float, 2> mouse;
				mouse.x = ((event.getPointerWindowPosition().x - (size.x * .5)) * sizeMultiplier) + cameraPosition.x;
				mouse.y = ((event.getPointerWindowPosition().y - (size.y * .5)) * sizeMultiplier) + cameraPosition.y;

				float distance = (mouse - existingCreatures->get(i)->position).length();

				if (distance < existingCreatures->get(i)->getRadius())
				{
					focusCreature = existingCreatures->get(i);

					break;
				}
			}
		}

		if (event.isPointerButtonPressed(Button3Mask))
		{
			for (int i = 0; i < existingCreatures->getLength(); i++)
			{
				agl::Vec<float, 2> mouse;
				mouse.x = ((event.getPointerWindowPosition().x - (size.x * .5)) * sizeMultiplier) + cameraPosition.x;
				mouse.y = ((event.getPointerWindowPosition().y - (size.y * .5)) * sizeMultiplier) + cameraPosition.y;

				float distance = (mouse - existingCreatures->get(i)->position).length();

				if (distance < existingCreatures->get(i)->getRadius())
				{
					existingCreatures->pop(i);

					break;
				}
			}
		}

		if (event.isKeyPressed(XK_f))
		{
			simulation.addFood(
				getCursorScenePosition(event.getPointerWindowPosition(), size, sizeMultiplier, cameraPosition));
		}

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
				getCursorScenePosition(event.getPointerWindowPosition(), size, sizeMultiplier, cameraPosition);

			sizeMultiplier -= scale;

			agl::Vec<float, 2> newPos =
				getCursorScenePosition(event.getPointerWindowPosition(), size, sizeMultiplier, cameraPosition);

			agl::Vec<float, 2> offset = oldPos - newPos;

			cameraPosition = cameraPosition + offset;
		}
		if (mouseWheelPos == -1)
		{
			float scale = sizeDelta * sizeMultiplier;

			agl::Vec<float, 2> oldPos =
				getCursorScenePosition(event.getPointerWindowPosition(), size, sizeMultiplier, cameraPosition);

			sizeMultiplier += scale;

			agl::Vec<float, 2> newPos =
				getCursorScenePosition(event.getPointerWindowPosition(), size, sizeMultiplier, cameraPosition);

			agl::Vec<float, 2> offset = oldPos - newPos;

			cameraPosition = cameraPosition + offset;
		}

		simulationInfo.setPosition({size.x - 260, 10, 9});

		window.setViewport(0, 0, size.x, size.y);

		camera.setOrthographicProjection(-((size.x / 2.) * sizeMultiplier), ((size.x / 2.) * sizeMultiplier),
										 ((size.y / 2.) * sizeMultiplier), -((size.y / 2.) * sizeMultiplier), 0.1, 100);
		camera.setView({cameraPosition.x, cameraPosition.y, 50}, {cameraPosition.x, cameraPosition.y, 0}, {0, 1, 0});

		guiCamera.setOrthographicProjection(0, size.x, size.y, 0, 0.1, 100);

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

	window.close();

	return 0;
}
