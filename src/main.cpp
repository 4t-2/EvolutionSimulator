#include "../lib/AGL/agl.hpp"

#include "../inc/Menu.hpp"
#include "../inc/Simulation.hpp"

#include <X11/X.h>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <math.h>
#include <unistd.h>

#define TOTAL_FOOD 10

#define BASE_B_VALUE 63

#define WIDTH  1920
#define HEIGHT 1080

#define CLEARCOLOR \
	{              \
		0, 0, 24   \
	}

#define NETWORK_PADDING 20

void printConnections(CreatureData creatureData)
{
	for (int i = 0; i < creatureData.getTotalConnections(); i++)
	{
		printf("connection %d going from %d to %d with weight %f\n", i, creatureData.getConnection()[i].startNode,
			   creatureData.getConnection()[i].endNode, creatureData.getConnection()[i].weight);
	}

	return;
}

int main()
{
	agl::RenderWindow window;
	window.setup({WIDTH, HEIGHT}, "EvolutionSimulator");
	window.setClearColor(CLEARCOLOR);
	window.setFPS(60);

	window.GLEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.1f);

	agl::Event event;
	event.setWindow(window);

	agl::Shader shader;
	shader.loadFromFile("./vert.glsl", "./frag.glsl");
	window.getShaderUniforms(shader);
	shader.use();

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
	font.setup("./VCR_OSD_MONO_1.001.ttf", 24);

	// menu shapes
	Menu simulationInfo;
	simulationInfo.setup({WIDTH - 260, 10}, {250, 100}, &blank, &font);

	Menu creatureInfo;
	creatureInfo.setup({10, 10, 9}, {400, HEIGHT - (20)}, &blank, &font);

	agl::Circle networkBackground(60);
	networkBackground.setTexture(&blank);
	networkBackground.setColor({15, 15, 15});
	networkBackground.setSize(agl::Vec<float, 3>{150, 150, 0});
	networkBackground.setPosition(agl::Vec<float, 3>{creatureInfo.getPosition().x + (creatureInfo.getSize().x / 2),
													 creatureInfo.getPosition().y + MENU_BORDER + MENU_PADDING +
														 MENU_SHADOWSIZE + networkBackground.getSize().y,
													 10});

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
	foodShape.setOffset({5, 5, -3});

	agl::Rectangle creatureShape;
	creatureShape.setTexture(&creatureBodyTexture);
	creatureShape.setColor(agl::Color::White);
	creatureShape.setSize(agl::Vec<float, 3>{25, 60, 0});
	creatureShape.setOffset(agl::Vec<float, 3>{-12.5, -12.5, -1});

	agl::Rectangle eggShape;
	eggShape.setTexture(&eggTexture);
	eggShape.setColor(agl::Color::White);
	eggShape.setSize(agl::Vec<float, 3>{15, 15, 0});
	eggShape.setOffset({7.5, 7.5, -2});

	agl::Rectangle rayShape;
	rayShape.setTexture(&blank);
	rayShape.setColor(agl::Color::White);
	rayShape.setSize(agl::Vec<float, 3>{1, RAY_LENGTH});
	rayShape.setOffset(agl::Vec<float, 3>{-0.5, 0, -1.5});

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

	for(int i  = (TOTAL_INPUT + TOTAL_OUTPUT); i < TOTAL_NODES; i++)
	{
		nodeNames[i] = "Hidden";
	}

	SimulationRules simulationRules;
	simulationRules.startingCreatures = 100;
	simulationRules.maxCreatures = 2000;
	simulationRules.foodEnergy = 60;
	simulationRules.maxFood = 1500;
	simulationRules.size = {19200, 10800};
	simulationRules.gridResolution = {15, 15};
	simulationRules.maxEggs = 1000;

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

	while (!event.windowClose())
	{
		event.pollWindow();
		event.pollKeyboard();
		event.pollPointer();

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
		}

		if (skipRender)
		{
			goto skipRendering;
		}

		window.clear();

		// Simulation rendering

		window.updateMvp(camera);

		// draw creature
		for (int i = 0; i < existingCreatures->getLength(); i++)
		{
			creatureShape.setPosition(existingCreatures->get(i)->getPosition());
			creatureShape.setRotation(
				agl::Vec<float, 3>{0, 0, -float(existingCreatures->get(i)->getRotation() * 180 / PI)});

			float speed = existingCreatures->get(i)->getVelocity().length() / 10;

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
		// Draw food
		for (int i = 0; i < simulation.getExistingFood()->getLength(); i++)
		{
			foodShape.setColor(agl::Color::Green);

			agl::Vec<float, 2> position = simulation.getExistingFood()->get(i)->position;
			foodShape.setPosition(agl::Vec<float, 2>{position.x, position.y});
			window.drawShape(foodShape);
		}

		// draw eggs
		for (int i = 0; i < existingEggs->getLength(); i++)
		{
			eggShape.setPosition(existingEggs->get(i)->getPosition());
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
				rayShape.setPosition(focusCreature->getPosition());
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

				rayShape.setPosition(focusCreature->getPosition());
				rayShape.setRotation(agl::Vec<float, 3>{0, 0, rayAngle});
				window.drawShape(rayShape);
			}
		}

		// gui rendering

		window.updateMvp(guiCamera);

		{
			std::stringstream ss;

			ss << "Creatures - " << simulation.getExistingCreatures()->getLength() << '\n';
			ss << "Eggs - " << simulation.getExistingEggs()->getLength() << '\n';
			ss << "Food - " << simulation.getExistingFood()->getLength();

			simulationInfo.setText(ss.str());
		}

		window.drawShape(*simulationInfo.getOuterShadowShape());
		window.drawShape(*simulationInfo.getBorderShape());
		window.drawShape(*simulationInfo.getInnerShadowShape());
		window.drawShape(*simulationInfo.getBodyShape());
		window.drawText(*simulationInfo.getText());

		if (existingCreatures->find(focusCreature) != -1)
		{
			std::stringstream ss;

			static int selectedID = 0;

			ss << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
			ss << "Node - " << nodeNames[selectedID] << '\n';
			ss << '\n';
			ss << "Position - " << agl::Vec<int, 2>(focusCreature->getPosition()) << '\n';
			ss << "Velocity - " << agl::Vec<int, 2>(focusCreature->getVelocity()) << '\n';
			ss << '\n';
			ss << "Eating - " << focusCreature->getEating() << '\n';
			ss << "Laying Egg - " << focusCreature->getLayingEgg() << '\n';
			ss << "Health - " << focusCreature->getHealth() << '\n';
			ss << "Energy - " << focusCreature->getEnergy() << '\n';
			ss << "Life left - " << focusCreature->getLifeLeft() << '\n';
			ss << '\n';
			ss << "Sight - " << focusCreature->getSight() << '\n';
			ss << "Speed - " << focusCreature->getSpeed() << '\n';
			ss << "Size - " << focusCreature->getSize() << '\n';
			ss << "Hue - " << focusCreature->getHue() << '\n';

			creatureInfo.setText(ss.str());

			window.drawShape(*creatureInfo.getOuterShadowShape());
			window.drawShape(*creatureInfo.getBorderShape());
			window.drawShape(*creatureInfo.getInnerShadowShape());
			window.drawShape(*creatureInfo.getBodyShape());
			window.drawText(*creatureInfo.getText());

			window.drawShape(networkBackground);

			// draw node connections
			for (int i = 0; i < focusCreature->getNeuralNetwork().getTotalConnections(); i++)
			{
				Connection connection = focusCreature->getNeuralNetwork().getConnection(i);

				if(!connection.valid)
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

		static float sizeMultiplier = 1;

		if (event.isPointerButtonPressed(Button1Mask))
		{
			for (int i = 0; i < existingCreatures->getLength(); i++)
			{
				agl::Vec<float, 2> mouse;
				mouse.x = ((event.getPointerWindowPosition().x - (WIDTH * .5)) * sizeMultiplier) + cameraPosition.x;
				mouse.y = ((event.getPointerWindowPosition().y - (HEIGHT * .5)) * sizeMultiplier) + cameraPosition.y;

				float distance = (mouse - existingCreatures->get(i)->getPosition()).length();

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
				mouse.x = event.getPointerWindowPosition().x;
				mouse.y = event.getPointerWindowPosition().y;

				float distance = (mouse - existingCreatures->get(i)->getPosition()).length();

				if (distance < existingCreatures->get(i)->getRadius())
				{
					existingCreatures->pop(i);

					break;
				}
			}
		}

		static agl::Vec<float, 2> cameraOffset;
		static agl::Vec<float, 2> startPos;

		if (event.isPointerButtonPressed(Button1Mask))
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
				startPos = event.getPointerWindowPosition();
				b1Held	 = true;
			}
		}
		else if (b1Held) // let go
		{
			cameraOffset = {0, 0};
			b1Held		 = false;
		}

		static float cameraSpeed = 4;

		if (event.isKeyPressed(XK_Down))
		{
			sizeMultiplier += .1;
		}
		if (event.isKeyPressed(XK_Up))
		{
			sizeMultiplier -= .1;
			if (sizeMultiplier < 0)
			{
				sizeMultiplier = 0;
			}
		}

		camera.setOrthographicProjection(-((WIDTH / 2.) * sizeMultiplier), ((WIDTH / 2.) * sizeMultiplier),
										 ((HEIGHT / 2.) * sizeMultiplier), -((HEIGHT / 2.) * sizeMultiplier), 0.1, 100);
		camera.setView({cameraPosition.x, cameraPosition.y, 50}, {cameraPosition.x, cameraPosition.y, 0}, {0, 1, 0});

		frame++;
	}

	simulation.destroy();

	blank.deleteTexture();
	shader.deleteProgram();
	window.close();

	return 0;
}
