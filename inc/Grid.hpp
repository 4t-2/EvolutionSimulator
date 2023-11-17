#pragma once
#include <AGL/agl.hpp>
#include <list>

template <typename T> class Grid
{
	private:
		std::list<T> ***gridData = nullptr;

	public:
		agl::Vec<int, 2> size = {0, 0};

		Grid(agl::Vec<int, 2> size)
		{
			this->size = size;

			gridData = new std::list<T> **[this->size.x];

			for (int x = 0; x < size.x; x++)
			{
				gridData[x] = new std::list<T> *[size.y];
				for (int y = 0; y < size.y; y++)
				{
					gridData[x][y] = new std::list<T>;
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
			gridData[gridPosition.x][gridPosition.y]->emplace_back(data);
		}

		void removeData(agl::Vec<int, 2> gridPosition, T data)
		{
			auto index = std::find(gridData[gridPosition.x][gridPosition.y]->begin(),
								   gridData[gridPosition.x][gridPosition.y]->end(), data);

			if (index != gridData[gridPosition.x][gridPosition.y]->end())
			{
				gridData[gridPosition.x][gridPosition.y]->erase(index);
			}
		}

		std::list<T> *getList(agl::Vec<int, 2> gridPosition)
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

		void updateGrids(agl::Vec<int, 2> gridPosition, agl::Vec<int, 2> startGridOffset,
						 agl::Vec<int, 2>												   endGridOffset,
						 std::function<void(std::list<T> *list, agl::Vec<int, 2> gridPos)> lambda)
		{
			for (int x = startGridOffset.x; x <= endGridOffset.x; x++)
			{
				if (gridPosition.x + x < 0 || gridPosition.x + x > (this->size.x - 1))
				{
					continue;
				}

				for (int y = startGridOffset.y; y <= endGridOffset.y; y++)
				{
					if (gridPosition.y + y < 0 || gridPosition.y + y > (this->size.y - 1))
					{
						continue;
					}

					std::list<T> *list = this->getList({gridPosition.x + x, gridPosition.y + y});

					lambda(list, {gridPosition.x + x, gridPosition.y + y});
				}
			}
		}

		void updateElements(agl::Vec<int, 2> gridPosition, agl::Vec<int, 2> startGridOffset,
							agl::Vec<int, 2> endGridOffset, std::function<void(T element)> lambda)
		{
			auto loopFunc = [&](std::list<T> *list, agl::Vec<int, 2> gridPos) {
				for (auto it = list->begin(); it != list->end();)
				{
					auto next = std::next(it, 1);
					lambda(*it);
					it = next;
				}
			};

			this->updateGrids(gridPosition, startGridOffset, endGridOffset, loopFunc);
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
};
