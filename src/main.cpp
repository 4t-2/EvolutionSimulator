#include "../lib/AGL/agl.hpp"

#include "../inc/NeuralNetwork.hpp"

#include <math.h>

#define BASE_B_VALUE 63

#define WIDTH  1920
#define HEIGHT 1080

class Creature
{
	private:
		agl::Vec2f position = {(float)WIDTH / 2, (float)HEIGHT / 2};
		agl::Vec2f velocity = {0, 0};
		float	   rotation = 0;

		// first 2 neurons are input
		// next 3 are output
		// rest are hidden
		NeuralNetwork *network;

	public:
		Creature();

		void update();



		NeuralNetwork getNeuralNetwork();
		agl::Vec2f	  getPosition();
		float		  getRotation();
};

Creature::Creature()
{
	Connection connection[2];

	connection[0].startNode = 0;
	connection[0].endNode	= 3;
	connection[0].weight	= 0.1;

	connection[1].startNode = 0;
	connection[1].endNode	= 4;
	connection[1].weight	= -0.1;


	network = new NeuralNetwork(5, 2, connection, 2);

	return;
}

void Creature::update()
{
	network->setInputNode(0, (position.x / WIDTH * 2)-1);

	network->update();

	velocity = {0, 0};

	float speed = 2.5;

	// if (network->getNode(2).value > 0.5)
	// {
	// 	speed = 2.5;
	// }

	if (network->getNode(3).value > 0.5)
	{
		rotation += 0.05;
	}

	if (network->getNode(4).value > 0.5)
	{
		rotation -= 0.05;
	}

	velocity.x = cos(rotation) * speed;
	velocity.y = sin(rotation) * speed;

	position.x += velocity.x;
	position.y += velocity.y;

return;
}

NeuralNetwork Creature::getNeuralNetwork()
{
	return *network;
}

agl::Vec2f Creature::getPosition()
{
	return position;
}

float Creature::getRotation()
{
	return -rotation * 180 / 3.14159;
}

int main()
{
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
	nodeShape.setSize({10, 10, 0});
	nodeShape.setPosition({500, 500, 3});

	agl::Rectangle connectionShape;
	connectionShape.setTexture(&blank);
	connectionShape.setColor(agl::Color::Red);
	connectionShape.setSize({1, 50, 2});
	
	agl::Circle background(6);
	background.setTexture(&blank);
	background.setColor({15, 15, 15});
	background.setPosition({150, 150, -1});
	background.setSize({150, 150, 1});

	float addAmount = 0.01;

	Creature creature;

	agl::Rectangle creatureShape;
	creatureShape.setTexture(&blank);
	creatureShape.setColor(agl::Color::White);
	creatureShape.setSize({25, 25, 0});
	creatureShape.setOffset({-12.5, -12.5, 0});

	while (!event.windowClose())
	{
		window.updateMvp(camera);

		event.pollWindow();
		event.pollKeyboard();

		creature.update();

		window.clear();

		printf("%f\n", creature.getRotation());

		creatureShape.setPosition(creature.getPosition());
		creatureShape.setRotation({0, 0, creature.getRotation()});
		window.drawShape(creatureShape);

		window.drawShape(background);

		for (int i = 0; i < creature.getNeuralNetwork().getTotalConnections(); i++)
		{
			agl::Vec3f start = {0, 0, 2};
			agl::Vec3f end;
			agl::Vec3f offset;

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
			connectionShape.setSize({2, length, 0});

			float angle = acos(offset.x / length) * (180 / 3.14159);
			connectionShape.setRotation({0, 0, angle + 90});

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

			agl::Vec3f pos;
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
