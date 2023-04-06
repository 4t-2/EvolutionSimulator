#pragma once
#include "../lib/AGL/agl.hpp"
#include "List.hpp"

template <typename T> class Grid
{
	private:
		agl::Vec<int, 2> size	  = {0, 0};
		List<T>		  ***gridData = nullptr;
		int				 max	  = 0;

	public:
		Grid(agl::Vec<int, 2> size, int max)
		{
			this->size = size;
			this->max  = max;

			gridData = new List<T> **[this->size.x];

			for (int x = 0; x < size.x; x++)
			{
				gridData[x] = new List<T> *[size.y];
				for (int y = 0; y < size.y; y++)
				{
					gridData[x][y] = new List<T>(max);
				}
			}
		}

		~Grid()
		{
			for (int x = 0; x < size.x; x++)
			{
				for (int y = 0; y < size.y; y++)
				{
					delete gridData[x][y];
				}

				delete[] gridData[x];
			}

			delete[] gridData;
		}

		void addData(agl::Vec<int, 2> gridPosition, T data)
		{
			gridData[gridPosition.x][gridPosition.y]->add(data);
		}

		void removeData(agl::Vec<int, 2> gridPosition, T data)
		{
			int index = gridData[gridPosition.x][gridPosition.y]->find(data);

			if (index != -1)
			{
				gridData[gridPosition.x][gridPosition.y]->pop(index);
			}
		}

		List<T> *getList(agl::Vec<int, 2> gridPosition)
		{
			return gridData[gridPosition.x][gridPosition.y];
		}

		agl::Vec<int, 2> toGridPosition(agl::Vec<float, 2> position, agl::Vec<float, 2> worldSize)
		{
			agl::Vec<int, 2> gridPosition;
			gridPosition.x = (position.x / worldSize.x) * (size.x);
			gridPosition.y = (position.y / worldSize.y) * (size.y);

			if (gridPosition.x < 0)
			{
				gridPosition.x = 0;
			}
			if (gridPosition.y < 0)
			{
				gridPosition.y = 0;
			}

			if (gridPosition.x > (size.x - 1))
			{
				gridPosition.x = (size.x - 1);
			}
			if (gridPosition.y > (size.y - 1))
			{
				gridPosition.y = (size.y - 1);
			}

			return gridPosition;
		}

		void updateElements(agl::Vec<int, 2> gridPosition, agl::Vec<int, 2> startGridOffset,
						agl::Vec<int, 2> endGridOffset, std::function<void(T element)> lambda)
		{
			for (int x = startGridOffset.x; x <= endGridOffset.x; x++)
			{
				if (gridPosition.x + x < 0 || gridPosition.x + x > (this->getSize().x - 1))
				{
					continue;
				}

				for (int y = startGridOffset.y; y <= endGridOffset.y; y++)
				{
					if (gridPosition.y + y < 0 || gridPosition.y + y > (this->getSize().y - 1))
					{
						continue;
					}

					List<T> *list = this->getList({gridPosition.x + x, gridPosition.y + y});

					for (int i = 0; i < list->getLength(); i++)
					{
						lambda(list->get(i));
					}
				}
			}
		}

		void clear()
		{
			for (int x = 0; x < size.x; x++)
			{
				for (int y = 0; y < size.y; y++)
				{
					gridData[x][y]->clear();
				}
			}
		}

		agl::Vec<int, 2> getSize()
		{
			return size;
		}
};
