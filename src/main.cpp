#include "../lib/AGL/agl.hpp"

#include "../inc/NeuralNetwork.hpp"

#include <math.h>

#define BASE_B_VALUE 63

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

	Connection connection[4];

	connection[0].startNode = 0;
	connection[0].endNode	= 2;
	connection[0].weight	= 1;

	connection[1].startNode = 0;
	connection[1].endNode	= 4;
	connection[1].weight	= 0.5;

	connection[2].startNode = 1;
	connection[2].endNode	= 2;
	connection[2].weight	= -1;

	connection[3].startNode = 4;
	connection[3].endNode	= 3;
	connection[3].weight	= -1;

	NeuralNetwork network(5, 2, connection, 4);

	for (int i = 0; i < network.getTotalNodes(); i++)
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

	float addAmount = 0.01;

	network.update();
	while (!event.windowClose())
	{
		window.updateMvp(camera);

		event.pollWindow();
		event.pollKeyboard();

		network.setInputNode(0, node1);
		network.setInputNode(1, node2);

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

		network.update();

		window.clear();

		for (int i = 0; i < network.getTotalConnections(); i++)
		{
			agl::Vec3f start;
			agl::Vec3f end;
			agl::Vec3f offset;

			{
				float angle = (360. / network.getTotalNodes()) * (network.getConnection(i).startNode + 1);

				float x = cos(angle * (3.14159 / 180));
				float y = sin(angle * (3.14159 / 180));

				start.x = (x * 300) + 500;
				start.y = (y * 300) + 500;
			}

			{
				float angle = (360. / network.getTotalNodes()) * (network.getConnection(i).endNode + 1);

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

		for (int i = 0; i < network.getTotalNodes(); i++)
		{
			float angle = (360. / network.getTotalNodes()) * (i + 1);

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
