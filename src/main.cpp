#include "../lib/AGL/agl.hpp"

#include <math.h>

#define INPUT_NODES	 1
#define OUTPUT_NODES 3
#define HIDDEN_NODES 0
#define TOTAL_NODES	 (INPUT_NODES + HIDDEN_NODES + OUTPUT_NODES)

#define TOTAL_CONNECTIONS 3

#define BASE_B_VALUE 63

enum NodeType
{
	input  = 0,
	hidden = 1,
	output = 2
};

class Node
{
	public:
		float	 value = 0;
		NodeType type  = hidden;
};

class Connection
{
	public:
		int	  startNode;
		int	  endNode;
		float weight;
};

class NeuralNetwork
{
	private:
		Node	   node[TOTAL_NODES];
		Connection connection[TOTAL_CONNECTIONS];

	public:
		NeuralNetwork();

		void setConnection(int connectionNumber, Connection connection);
		void setInputNode(int nodeNumber, float value);

		void update();

		Connection getConnection(int connectionNumber);
		Node	   getNode(int nodeNumber);
};

NeuralNetwork::NeuralNetwork()
{
	return;
}

void NeuralNetwork::setConnection(int connectionNumber, Connection connection)
{
	this->connection[connectionNumber] = connection;

	return;
}

void NeuralNetwork::setInputNode(int nodeNumber, float value)
{
	node[nodeNumber].value = value;

	return;
}

void NeuralNetwork::update()
{
	for (int i = 0; i < TOTAL_CONNECTIONS; i++)
	{
		node[connection[i].endNode].value = node[connection[i].startNode].value * connection[i].weight;
	}

	return;
}

Connection NeuralNetwork::getConnection(int connectionNumber)
{
	return connection[connectionNumber];
}

Node NeuralNetwork::getNode(int nodeNumber)
{
	return node[nodeNumber];
}

int main()
{
	agl::RenderWindow window;
	window.setup({1000, 1000}, "EvolutionSimulator");
	window.setClearColor(agl::Color::Black);
	window.setFPS(60);

	agl::Event event;
	event.setWindow(window);

	agl::Shader shader;
	shader.loadFromFile("./vert.glsl", "./frag.glsl");
	window.getShaderUniforms(shader);
	shader.use();

	agl::Camera camera;
	camera.setOrthographicProjection(0, 1000, 1000, 0, 0.1, 100);
	camera.setView({0, 0, 1}, {0, 0, 0}, {0, 1, 0});

	agl::Texture blank;
	blank.setBlank();

	NeuralNetwork network;

	Connection connection;

	connection.startNode = 0;
	connection.endNode	 = 1;
	connection.weight	 = 1;

	network.setConnection(0, connection);

	connection.startNode = 0;
	connection.endNode	 = 2;
	connection.weight	 = 0;

	network.setConnection(1, connection);

	connection.startNode = 0;
	connection.endNode	 = 3;
	connection.weight	 = -1;

	network.setConnection(2, connection);

	network.setInputNode(0, 1);

	network.update();

	agl::Circle nodeShape(10);
	nodeShape.setTexture(&blank);
	nodeShape.setSize({25, 25, 0});
	nodeShape.setPosition({500, 500, 0});

	agl::Rectangle connectionShape;
	connectionShape.setTexture(&blank);
	connectionShape.setColor(agl::Color::Red);
	connectionShape.setSize({10, 50, 0});

	while (!event.windowClose())
	{
		window.updateMvp(camera);

		event.pollWindow();
		event.pollKeyboard();

		window.clear();

		for (int i = 0; i < TOTAL_CONNECTIONS; i++)
		{
			agl::Vec3f start;
			agl::Vec3f end;
			agl::Vec3f offset;

			{
				float angle = (360. / TOTAL_NODES) * (network.getConnection(i).startNode + 1);

				float x = cos(angle * (3.14159 / 180));
				float y = sin(angle * (3.14159 / 180));

				start.x = (x * 300) + 500;
				start.y = (y * 300) + 500;
			}

			{
				float angle = (360. / TOTAL_NODES) * (network.getConnection(i).endNode + 1);

				float x = cos(angle * (3.14159 / 180));
				float y = sin(angle * (3.14159 / 180));

				end.x = (x * 300) + 500;
				end.y = (y * 300) + 500;
			}

			offset = end - start;

			float length = sqrt((offset.x * offset.x) + (offset.y * offset.y));
			connectionShape.setSize({10, length, 0});

			float angle = acos(offset.x / length) * (180 / 3.14159);
			connectionShape.setRotation({0, 0, angle + 90});

			float weight = network.getConnection(i).weight;

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

			std::cout << angle << std::endl;
		}
		printf("\n");

		for (int i = 0; i < TOTAL_NODES; i++)
		{
			float angle = (360. / TOTAL_NODES) * (i + 1);

			float x = cos(angle * (3.14159 / 180));
			float y = sin(angle * (3.14159 / 180));

			agl::Vec3f pos;
			pos.x = x * 300;
			pos.y = y * 300;

			pos.x += 500;
			pos.y += 500;

			nodeShape.setPosition(pos);

			float nodeValue = network.getNode(i).value;

			if (nodeValue > 0)
			{
				nodeShape.setColor({0, (unsigned char)(nodeValue * 255), BASE_B_VALUE});
			}
			else
			{
				nodeShape.setColor({(unsigned char)(-nodeValue * 255), 0, BASE_B_VALUE});
			}

			window.drawShape(nodeShape);

			printf("%d %f\n", i, nodeValue);
		}
		printf("\n");

		window.display();
	}

	blank.deleteTexture();
	shader.deleteProgram();
	window.close();

	return 0;
}
