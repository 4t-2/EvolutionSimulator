#include "../inc/NeuralNetwork.hpp"

NeuralNetwork::NeuralNetwork(int totalNodes, int totalInputNodes, Connection connection[], int totalConnections)
{
	this->totalNodes	   = totalNodes;
	this->totalInputNodes  = totalInputNodes;
	this->totalConnections = totalConnections;

	node			 = new Node[totalNodes];
	inputNode		 = new Node *[totalInputNodes];
	this->connection = new Connection[totalConnections];

	// link input node pointers to actual nodes
	for (int i = 0; i < totalInputNodes; i++)
	{
		inputNode[i] = &node[i];
	}

	// give every node an ID
	for (int i = 0; i < totalNodes; i++)
	{
		node[i].id = i;
	}

	// set connections stored to be equal to inputted ones
	for (int i = 0; i < totalConnections; i++)
	{
		this->connection[i] = connection[i];
	}

	// set impossible connections as invalid (if it loops)
	bool *validCheck = new bool[totalConnections];

	for (int i = 0; i < totalConnections; i++)
	{
		validCheck[i] = false;
	}

	printf("exit\n");

	for (bool allChecked = false; !allChecked;)
	{
		for (int i = 0; i < totalConnections; i++)
		{
			if (validCheck[i])
			{
				continue;
			}

			printf("testing %d\n", i);

			int startNode = this->connection[i].startNode;

			bool isNodeBase = true;

			for (int x = 0; x < totalConnections; x++)
			{
				if (x == i)
				{
					continue;
				}

				if (this->connection[x].endNode == startNode)
				{
					if (validCheck[x] || !this->connection[x].valid)
					{
						continue;
					}

					isNodeBase = false;
					break;
				}
			}

			if (isNodeBase)
			{
				printf("%d is base\n", i);
				for (int x = 0; x < totalConnections; x++)
				{
					if (x == i)
					{
						continue;
					}

					if (this->connection[i].endNode == this->connection[x].startNode && validCheck[x])
					{
						printf("%d is invalid\n", i);
						this->connection[i].valid = false;
					}
				}
				validCheck[i] = true;
			}

			printf("\n");
		}

		allChecked = true;

		for (int i = 0; i < totalConnections; i++)
		{
			if (!validCheck[i])
			{
				allChecked = false;
			}
		}
	}

	printf("exit\n");

	delete[] validCheck;

	// set the amount of parents every node has according to connection
	for (int y = 0; y < totalConnections; y++)
	{
		if (this->connection[y].valid)
		{
			node[this->connection[y].endNode].parents++;
		}
	}

	// allocate memory for every node to store a pointer to its parents
	for (int i = 0; i < totalNodes; i++)
	{
		if (node[i].parents)
		{
			node[i].parent = new Node *[node[i].parents];
			node[i].weight = new float[node[i].parents];

			int setParents = 0;

			for (int x = 0; x < totalConnections; x++)
			{
				if (connection[x].valid)
				{
					if (connection[x].endNode == i)
					{
						node[i].parent[setParents] = &node[connection[x].startNode];
						node[i].weight[setParents] = connection[x].weight;

						setParents++;
					}
				}
			}
		}
	}

	for (int i = 0; i < totalNodes; i++)
	{
		if (node[i].parents)
		{
			connectedNodes++;
		}
	}

	nodeCalculationOrder = new Node *[connectedNodes];

	for (int i = totalInputNodes; i < totalNodes; i++)
	{
		node[i].value = 2;
	}

	bool loop = true;

	int nodeOrder = 0;

	while (loop)
	{
		for (int i = 0; i < totalNodes; i++)
		{
			if (node[i].parents)
			{
				bool allBase = true;

				for (int x = 0; x < node[i].parents; x++)
				{
					if (node[i].parent[x]->value == 2)
					{
						allBase = false;
					}
				}

				if (allBase)
				{
					bool inList = false;

					for (int x = 0; x < nodeOrder; x++)
					{
						if (node[i].id == nodeCalculationOrder[x]->id)
						{
							inList = true;
						}
					}

					if (!inList)
					{
						node[i].value = 0;

						nodeCalculationOrder[nodeOrder] = &node[i];

						nodeOrder++;
					}
				}
			}
		}

		loop = false;

		for (int i = 0; i < totalNodes; i++)
		{
			if (node[i].parents && node[i].value == 2)
			{
				loop = true;
			}
		}
	}

	for (int i = 0; i < totalNodes; i++)
	{
		node[i].value = 0;
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
	for (int i = 0; i < connectedNodes; i++)
	{
		for (int x = 0; x < nodeCalculationOrder[i]->parents; x++)
		{
			nodeCalculationOrder[i]->value +=
				nodeCalculationOrder[i]->parent[x]->value * nodeCalculationOrder[i]->weight[x];
		}

		nodeCalculationOrder[i]->value = tanh(nodeCalculationOrder[i]->value);
	}

	return;
}

void NeuralNetwork::destroy()
{
	// free memory in nodes
	for (int i = 0; i < totalNodes; i++)
	{
		delete[] node[i].weight;
		delete[] node[i].parent;
	}

	delete[] node;
	delete[] inputNode;
	delete[] connection;
	delete[] nodeCalculationOrder;

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

int NeuralNetwork::getTotalNodes()
{
	return totalNodes;
}

int NeuralNetwork::getTotalInputNodes()
{
	return totalInputNodes;
}

int NeuralNetwork::getTotalConnections()
{
	return totalConnections;
}
