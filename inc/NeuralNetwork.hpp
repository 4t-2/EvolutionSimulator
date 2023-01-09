#pragma once

#include <cstdio>
#include <math.h>

class Node
{
	public:
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
		bool  valid	 = true;
		int	  id	 = -1;
		bool  exists = true;
};

class NeuralNetwork
{
	private:
		int totalNodes		 = 0;
		int totalInputNodes	 = 0;
		int totalConnections = 0;
		int connectedNodes	 = 0;

		Node	   *node;
		Node	  **inputNode;
		Connection *connection;
		Node	  **nodeCalculationOrder;

	public:
		NeuralNetwork(int totalNodes, int totalInputNodes, Connection connection[], int totalConnections);

		void setConnection(int connectionNumber, Connection connection);
		void setInputNode(int nodeNumber, float value);

		void update();
		void destroy();

		Connection getConnection(int connectionNumber);
		Node	   getNode(int nodeNumber);
		int		   getTotalNodes();
		int		   getTotalInputNodes();
		int		   getTotalConnections();
};
