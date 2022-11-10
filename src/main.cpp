#include "../lib/AGL/agl.hpp"

#include "../inc/NeuralNetwork.hpp"
#include "../inc/Creature.hpp"
#include "../inc/Food.hpp"

#include <cstdlib>
#include <ctime>
#include <math.h>

#define TOTAL_FOOD 10

#define BASE_B_VALUE 63

#define WIDTH  1920
#define HEIGHT 1080

int main()
{
	srand(time(NULL));

	agl::RenderWindow window;
	window.setup({WIDTH, HEIGHT}, "EvolutionSimulator");
	window.setClearColor(agl::Color::Black);
	window.setFPS(60);

	agl::Event event;
	event.setWindow(window);

	agl::Shader shader;
	shader.loadFromFile("./vert.glsl", "./frag.glsl");
	window.getShaderUniforms(shader);
	shader.use();

	agl::Camera camera;
	camera.setOrthographicProjection(0, WIDTH, HEIGHT, 0, 0.1, 100);
	camera.setView({0, 0, 50}, {0, 0, 0}, {0, 1, 0});

	agl::Texture blank;
	blank.setBlank();

	agl::Circle nodeShape(10);
	nodeShape.setTexture(&blank);
	nodeShape.setSize(agl::Vec<float, 3>{10, 10, 0});
	nodeShape.setPosition(agl::Vec<float, 3>{500, 500, 3});

	agl::Rectangle connectionShape;
	connectionShape.setTexture(&blank);
	connectionShape.setColor(agl::Color::Red);
	connectionShape.setSize(agl::Vec<float, 3>{1, 50, 2});
	
	agl::Circle background(6);
	background.setTexture(&blank);
	background.setColor({15, 15, 15});
	background.setPosition(agl::Vec<float, 3>{150, 150, -1});
	background.setSize(agl::Vec<float, 3>{150, 150, 1});

	float addAmount = 0.01;

	Food food[TOTAL_FOOD];	
	for(int i = 0; i < TOTAL_FOOD; i++)
	{
		food[i].position = {(float)rand() / (float)RAND_MAX * WIDTH, (float)rand() / (float)RAND_MAX * HEIGHT};
	}

	agl::Circle foodShape(10);
	foodShape.setTexture(&blank);
	foodShape.setColor(agl::Color::Green);
	foodShape.setSize(agl::Vec<float, 3>{10, 10, 0});

	Creature creature;
	creature.setPosition({(float)WIDTH / 2, (float)HEIGHT / 2});
	creature.setWorldSize({WIDTH, HEIGHT});

	agl::Rectangle creatureShape;
	creatureShape.setTexture(&blank);
	creatureShape.setColor(agl::Color::White);
	creatureShape.setSize(agl::Vec<float, 3>{25, 25, 0});
	creatureShape.setOffset(agl::Vec<float, 3>{-12.5, -12.5, 0});

	while (!event.windowClose())
	{
		window.updateMvp(camera);

		event.pollWindow();
		event.pollKeyboard();
		event.pollPointer();

		creature.updateActions(food, TOTAL_FOOD);

		window.clear();

		creatureShape.setPosition(creature.getPosition());
		creatureShape.setRotation(agl::Vec<float, 3>{0, 0, creature.getRotation()});
		window.drawShape(creatureShape);

		for(int i = 0; i < TOTAL_FOOD; i++)
		{
			if(!food[i].exists)
			{
				continue;
			}

			if(creature.getClosestFood() == &food[i])
			{
				foodShape.setColor(agl::Color::Magenta);
			} else {
				foodShape.setColor(agl::Color::Green);
			}
			foodShape.setPosition(agl::Vec<float, 3>{food[i].position.x, food[i].position.y, -1});
			window.drawShape(foodShape);
		}

		window.drawShape(background);

		for (int i = 0; i < creature.getNeuralNetwork().getTotalConnections(); i++)
		{
			agl::Vec<float, 3> start = {0, 0, 2};
			agl::Vec<float, 3> end;
			agl::Vec<float, 3> offset;

			{
				float angle = (360. / creature.getNeuralNetwork().getTotalNodes()) *
							  (creature.getNeuralNetwork().getConnection(i).startNode + 1);

				float x = cos(angle * (3.14159 / 180));
				float y = sin(angle * (3.14159 / 180));

				start.x = (x * 100) + 150;
				start.y = (y * 100) + 150;
			}

			{
				float angle = (360. / creature.getNeuralNetwork().getTotalNodes()) *
							  (creature.getNeuralNetwork().getConnection(i).endNode + 1);

				float x = cos(angle * (3.14159 / 180));
				float y = sin(angle * (3.14159 / 180));

				end.x = (x * 100) + 150;
				end.y = (y * 100) + 150;
			}

			offset = end - start;

			float length = sqrt((offset.x * offset.x) + (offset.y * offset.y));
			connectionShape.setSize(agl::Vec<float, 3>{2, length, 0});

			float angle = acos(offset.x / length) * (180 / 3.14159);
			connectionShape.setRotation(agl::Vec<float, 3>{0, 0, angle + 90});

			float weight = creature.getNeuralNetwork().getConnection(i).weight;

			if (weight > 0)
			{
				connectionShape.setColor({0, (unsigned char)(weight * 255), BASE_B_VALUE});
			}
			else
			{
				connectionShape.setColor({(unsigned char)(-weight * 255), 0, BASE_B_VALUE});
			}

			connectionShape.setPosition(start);
			window.drawShape(connectionShape);
		}
		


		for (int i = 0; i < creature.getNeuralNetwork().getTotalNodes(); i++)
		{
			float angle = (360. / creature.getNeuralNetwork().getTotalNodes()) * (i + 1);

			float x = cos(angle * (3.14159 / 180));
			float y = sin(angle * (3.14159 / 180));

			agl::Vec<float, 3> pos;
			pos.x = x * 100;
			pos.y = y * 100;

			pos.x += 150;
			pos.y += 150;

			pos.z = 3;

			nodeShape.setPosition(pos);

			float nodeValue = creature.getNeuralNetwork().getNode(i).value;

			if (nodeValue > 0)
			{
				nodeShape.setColor({0, (unsigned char)(nodeValue * 255), BASE_B_VALUE});
			}
			else
			{
				nodeShape.setColor({(unsigned char)(-nodeValue * 255), 0, BASE_B_VALUE});
			}

			window.drawShape(nodeShape);
		}

		window.display();
	}

	blank.deleteTexture();
	shader.deleteProgram();
	window.close();

	return 0;
}
