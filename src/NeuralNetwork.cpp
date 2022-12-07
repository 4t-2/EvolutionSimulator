#include "../inc/NeuralNetwork.hpp"

void travelBranchForLoop(Connection *connection, int id, bool *isNodeVisited, int totalConnections,
						 int totalNodes) // recursion hell
{
	isNodeVisited[connection[id].startNode] = true;

	if (isNodeVisited[connection[id].endNode])
	{
		connection[id].valid = false;

		return;
	}
	else
	{
		for (int i = 0; i < totalConnections; i++)
		{
			if (connection[id].endNode == connection[i].startNode)
			{
				bool *isNodeVisitedBranch = new bool[totalNodes];

				for (int x = 0; x < totalNodes; x++)
				{
					isNodeVisitedBranch[x] = isNodeVisited[x];
				}

				travelBranchForLoop(connection, i, isNodeVisitedBranch, totalConnections, totalNodes);

				delete[] isNodeVisitedBranch;
			}
		}
	}

	return;
}

void travelNodeTree(Node *node, int totalNodes, Node **nodeCalculationOrder, int connectedNodes, int *nodeOrder, int id)
{
	for (int i = 0; i < totalNodes; i++)
	{
		if (node[i].parents)
		{
			for (int x = 0; x < node[i].parents; x++)
			{
				if (x == i)
				{
					continue;
				}

				if (node[i].parent[x]->id == id)
				{
					for (int y = 0; y < *nodeOrder; y++)
					{
						if (nodeCalculationOrder[y]->id == i)
						{
							goto exitLoop;
						}
					}

					nodeCalculationOrder[*nodeOrder] = &node[i];
					*nodeOrder += 1;

					travelNodeTree(node, totalNodes, nodeCalculationOrder, connectedNodes, nodeOrder, i);
				}

			exitLoop:;
			}
		}
	}

	return;
}

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
		this->connection[i]	   = connection[i];
		this->connection[i].id = i;
	}

	// Invalidate duplicated connections

	for (int i = 0; i < this->totalConnections; i++)
	{
		if (this->connection[i].valid)
		{
			for (int x = 0; x < this->totalConnections; x++)
			{
				if (this->connection[x].valid && x != i)
				{
					if (this->connection[x].startNode == this->connection[i].startNode &&
						this->connection[x].endNode == this->connection[i].endNode)
					{
						this->connection[x].valid = false;
					}
				}
			}
		}
	}

	// invalid connections going into input

	for (int i = 0; i < this->totalConnections; i++)
	{
		if (this->connection[i].valid)
		{
			if (this->connection[i].endNode < totalInputNodes)
			{
				this->connection[i].valid = false;
			}
		}
	}

	// Invalidate looping connections

	bool *isConnectionBase = new bool[this->totalConnections];

	for (int i = 0; i < totalConnections; i++)
	{
		if (!this->connection[i].valid)
		{
			continue;
		}

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
				isNodeBase = false;
				break;
			}
		}

		isConnectionBase[i] = isNodeBase;
	}

	for (int i = 0; i < totalConnections; i++)
	{
		if (isConnectionBase[i])
		{
			bool *isNodeVisited = new bool[this->totalNodes];
			for (int i = 0; i < this->totalNodes; i++)
			{
				isNodeVisited[i] = false;
			}

			travelBranchForLoop(this->connection, i, isNodeVisited, this->totalConnections, this->totalNodes);

			delete[] isNodeVisited;
		}
	}

	delete[] isConnectionBase;

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
				if (this->connection[x].valid)
				{
					if (this->connection[x].endNode == i)
					{
						node[i].parent[setParents] = &node[this->connection[x].startNode];
						node[i].weight[setParents] = this->connection[x].weight;

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

	for (int i = 0; i < totalInputNodes; i++)
	{
		node[i].value = 2;
	}

	nodeCalculationOrder = new Node *[connectedNodes];

	int nodeOrder = 0;

	for (int i = 0; i < totalInputNodes; i++)
	{
		travelNodeTree(node, totalNodes, nodeCalculationOrder, connectedNodes, &nodeOrder, i);
	}

	connectedNodes = nodeOrder;

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
