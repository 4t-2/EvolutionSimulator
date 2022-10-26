#include "../lib/AGL/agl.hpp"

#include <math.h>

#define INPUT_NODES	 2
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
		bool   base	   = false;
		int	   id	   = -1;
		float  value   = 0;
		int	   parents = 0;
		Node **parent  = nullptr;
		float *weight  = 0;
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
		Node	  *inputNode[INPUT_NODES];
		Connection connection[TOTAL_CONNECTIONS];

	public:
		NeuralNetwork(Connection connection[]);

		void setConnection(int connectionNumber, Connection connection);
		void setInputNode(int nodeNumber, float value);

		void update();

		Connection getConnection(int connectionNumber);
		Node	   getNode(int nodeNumber);
};

NeuralNetwork::NeuralNetwork(Connection connection[])
{
	// link input node pointers to actual nodes
	for (int i = 0; i < INPUT_NODES; i++)
	{
		inputNode[i] = &node[i];
		node[i].base = true;
	}

	// give every node an ID
	for (int i = 0; i < TOTAL_NODES; i++)
	{
		node[i].id = i;
	}

	// set the amount of parents every node has according to connection
	for (int y = 0; y < TOTAL_CONNECTIONS; y++)
	{
		this->connection[y] = connection[y];
		node[connection[y].endNode].parents++;
	}

	// allocate memory for every node to store a pointer to its parents
	for (int i = 0; i < TOTAL_NODES; i++)
	{
		if (node[i].parents)
		{
			node[i].parent = (Node **)malloc(node[i].parents * sizeof(Node *));

			int setParents = 0;

			for (int x = 0; x < TOTAL_CONNECTIONS; x++)
			{
				if (connection[x].endNode == i)
				{
					node[i].parent[setParents] = &node[connection[x].startNode];

					setParents++;
				}
			}
		}
	}

	return;
}

void NeuralNetwork::setConnection(int connectionNumber, Connection connection)
{
	this->connection[connectionNumber] = connection;

	return;
}

void NeuralNetwork::setInputNode(int nodeNumber, float value)
{
	inputNode[nodeNumber]->value = value;

	return;
}

// this is shit and can definately be improved
void NeuralNetwork::update()
{
	for (int i = INPUT_NODES; i < TOTAL_NODES; i++)
	{
		node[i].base  = false;
		node[i].value = 0;
	}

	bool loop = true;

	while (loop)
	{
		for (int i = 0; i < TOTAL_NODES; i++)
		{
			if (node[i].parents)
			{
				bool allBase = true;

				for (int x = 0; x < node[i].parents; x++)
				{
					if (node[i].parent[x]->base == false)
					{
						allBase = false;
					}
				}

				if (allBase)
				{
					node[i].base = true;
					for (int x = 0; x < node[i].parents; x++)
					{
						node[i].value += node[i].parent[x]->value;
					}

					node[i].value = tanh(node[i].value);
				}
			}
		}

		loop = false;

		for (int i = 0; i < TOTAL_NODES; i++)
		{
			if (node[i].parents && node[i].base == false)
			{
				loop = true;
			}
		}
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

class test
{
	public:
		test(int i)
		{
			num = i;
		}
		int num = 0;
};

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

	Connection connection[TOTAL_CONNECTIONS];

	connection[0].startNode = 0;
	connection[0].endNode	= 2;
	connection[0].weight	= 1;

	connection[1].startNode = 0;
	connection[1].endNode	= 3;
	connection[1].weight	= 0.5;

	connection[2].startNode = 1;
	connection[2].endNode	= 2;
	connection[2].weight	= -0.75;

	NeuralNetwork network(connection);

	for (int i = 0; i < TOTAL_NODES; i++)
	{
		int parents = network.getNode(i).parents;
		printf("%d %d\n", i, parents);

		if (parents)
		{
			for (int x = 0; x < parents; x++)
			{
				printf("\t%d\n", network.getNode(i).parent[x]->id);
			}
		}
	}

	float node1 = 1;
	float node2 = -0.25;

	agl::Circle nodeShape(10);
	nodeShape.setTexture(&blank);
	nodeShape.setSize({25, 25, 0});
	nodeShape.setPosition({500, 500, 0});

	agl::Rectangle connectionShape;
	connectionShape.setTexture(&blank);
	connectionShape.setColor(agl::Color::Red);
	connectionShape.setSize({10, 50, 0});

	for (int i = 0; i < TOTAL_NODES; i++)
	{
		printf("%d %f\n", i, network.getNode(i).value);
	}

	float addAmount = 0.01;

	while (!event.windowClose())
	{
		window.updateMvp(camera);

		event.pollWindow();
		event.pollKeyboard();

		network.setInputNode(0, node1);
		network.setInputNode(1, node2);

		network.update();

		if (event.isKeyPressed(XK_q))
		{
			node1 += addAmount;

			if (node1 >= 1)
			{
				node1 = 1;
			}
		}
		if (event.isKeyPressed(XK_a))
		{
			node1 -= addAmount;

			if (node1 <= -1)
			{
				node1 = -1;
			}
		}
		if (event.isKeyPressed(XK_w))
		{
			node2 += addAmount;

			if (node2 >= 1)
			{
				node2 = 1;
			}
		}
		if (event.isKeyPressed(XK_s))
		{
			node2 -= addAmount;

			if (node2 <= -1)
			{
				node2 = -1;
			}
		}

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
		}

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
		}

		window.display();
	}

	blank.deleteTexture();
	shader.deleteProgram();
	window.close();

	return 0;
}
