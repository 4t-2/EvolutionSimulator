#pragma once

#include "ThreadPool.hpp"
#include "macro.hpp"
#include <AGL/agl.hpp>
#include <cstddef>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <random>
#include <thread>
#include <tuple>
#include <typeinfo>

class BaseEntity
{
	public:
		bool			   &exists;
		agl::Vec<float, 2> &position;

		BaseEntity(bool &exists, agl::Vec<float, 2> &position) : exists(exists), position(position)
		{
		}

		virtual ~BaseEntity()
		{
		}
};

class DoNotUse : public BaseEntity
{
	public:
		DoNotUse(bool &exists, agl::Vec<float, 2> &position) : BaseEntity(exists, position)
		{
		}
};

template <typename... T> class Signature
{
};

template <typename... T> class Entity : public DoNotUse, public T...
{
	private:
	public:
		Entity(bool &exists, agl::Vec<float, 2> &position) : DoNotUse(exists, position), T(exists, position)...
		{
		}

		virtual ~Entity()
		{
		}
};

class Environment
{
	private:
		template <typename T, typename U, bool oneWay, bool mirror, typename O, typename E>
		inline void execGridThing(agl::Vec<int, 2> &gridPosition, std::function<void(T &, U &)> &func,
								  std::function<float(T &)> &distFunc)
		{
			auto  hashT = typeid(O).hash_code();
			auto  hashU = typeid(E).hash_code();
			auto &listT = grid[gridPosition.x][gridPosition.y][hashT].list;
			for (auto it = listT.begin(); it != listT.end(); it++)
			{
				T *addressT = (O *)(DoNotUse *)*it;

				float distance = distFunc(*addressT);

				agl::Vec<int, 2> startGrid =
					toGridPosition({(*it)->position.x - distance, (*it)->position.y - distance}) - gridPosition;
				agl::Vec<int, 2> endGrid =
					toGridPosition({(*it)->position.x + distance, (*it)->position.y + distance}) - gridPosition;

				if constexpr (!oneWay)
				{
					for (int y = startGrid.y; y <= -1; y++)
					{
						for (int x = startGrid.x; x <= endGrid.x; x++)
						{
							grid[gridPosition.x + x][gridPosition.y + y][hashU].mtx.lock();
							grid[gridPosition.x][gridPosition.y][hashT].mtx.lock();
							gridUpdate<T, U, oneWay, mirror, O, E, false>(func, gridPosition, {x, y}, addressT, it);
							grid[gridPosition.x][gridPosition.y][hashT].mtx.unlock();
							grid[gridPosition.x + x][gridPosition.y + y][hashU].mtx.unlock();
						}
					}

					for (int x = startGrid.x; x <= -1; x++)
					{
						grid[gridPosition.x + x][gridPosition.y + 0][hashU].mtx.lock();
						grid[gridPosition.x][gridPosition.y][hashT].mtx.lock();
						gridUpdate<T, U, oneWay, mirror, O, E, false>(func, gridPosition, {x, 0}, addressT, it);
						grid[gridPosition.x][gridPosition.y][hashT].mtx.unlock();
						grid[gridPosition.x + x][gridPosition.y + 0][hashU].mtx.unlock();
					}
				}

				if (hashT > hashU)
				{
					grid[gridPosition.x][gridPosition.y][hashT].mtx.lock();
					grid[gridPosition.x][gridPosition.y][hashU].mtx.lock();
				}
				else if (hashT < hashU)
				{
					grid[gridPosition.x][gridPosition.y][hashU].mtx.lock();
					grid[gridPosition.x][gridPosition.y][hashT].mtx.lock();
				}
				else
				{
					grid[gridPosition.x][gridPosition.y][hashT].mtx.lock();
				}
				if (hashT == hashU && oneWay)
				{
					gridUpdate<T, U, oneWay, mirror, O, E, true>(func, gridPosition, {0, 0}, addressT, it);
				}
				else
				{
					gridUpdate<T, U, oneWay, mirror, O, E, false>(func, gridPosition, {0, 0}, addressT, it);
				}

				if (hashT > hashU)
				{
					grid[gridPosition.x][gridPosition.y][hashT].mtx.unlock();
					grid[gridPosition.x][gridPosition.y][hashU].mtx.unlock();
				}
				else if (hashT < hashU)
				{
					grid[gridPosition.x][gridPosition.y][hashU].mtx.unlock();
					grid[gridPosition.x][gridPosition.y][hashT].mtx.unlock();
				}
				else
				{
					grid[gridPosition.x][gridPosition.y][hashT].mtx.unlock();
				}

				for (int x = 1; x <= endGrid.x; x++)
				{
					grid[gridPosition.x][gridPosition.y][hashT].mtx.lock();
					grid[gridPosition.x + x][gridPosition.y + 0][hashU].mtx.lock();
					gridUpdate<T, U, oneWay, mirror, O, E, false>(func, gridPosition, {x, 0}, addressT, it);
					grid[gridPosition.x + x][gridPosition.y + 0][hashU].mtx.unlock();
					grid[gridPosition.x][gridPosition.y][hashT].mtx.unlock();
				}
				for (int y = 1; y <= endGrid.y; y++)
				{
					for (int x = startGrid.x; x <= endGrid.x; x++)
					{
						grid[gridPosition.x][gridPosition.y][hashT].mtx.lock();
						grid[gridPosition.x + x][gridPosition.y + y][hashU].mtx.lock();
						gridUpdate<T, U, oneWay, mirror, O, E, false>(func, gridPosition, {x, y}, addressT, it);
						grid[gridPosition.x + x][gridPosition.y + y][hashU].mtx.unlock();
						grid[gridPosition.x][gridPosition.y][hashT].mtx.unlock();
					}
				}
			}
		}

		// loops through entity 2
		template <bool skip, bool flip, typename T, typename U, bool oneWay, bool mirror, typename O, typename E,
				  typename... Es>
		inline void threadFunc2(agl::Vec<int, 2> &pos, std::function<void(T &, U &)> &func,
								std::function<float(T &)> &distFunc)
		{
			if constexpr (!skip)
			{
				if constexpr (flip)
				{
					if constexpr (std::is_base_of_v<T, E> || std::is_same_v<T, E>)
					{
						execGridThing<T, U, oneWay, mirror, E, O>(pos, func, distFunc);
					}
				}
				else
				{
					if constexpr (std::is_base_of_v<U, E> || std::is_same_v<U, E>)
					{
						execGridThing<T, U, oneWay, mirror, O, E>(pos, func, distFunc);
					}
				}
			}

			if constexpr (sizeof...(Es) > 0)
			{
				threadFunc2<false, flip, T, U, oneWay, mirror, O, Es...>(pos, func, distFunc);
			}
		}

		// loops through entity 1
		template <typename T, typename U, bool oneWay, bool mirror, typename E, typename... Es>
		inline void threadFunc1(agl::Vec<int, 2> &pos, std::function<void(T &, U &)> func,
								std::function<float(T &)> distFunc)
		{
			if constexpr (std::is_base_of_v<T, E> || std::is_same_v<T, E>)
			{
				threadFunc2<false, false, T, U, oneWay, mirror, E, E, Es...>(pos, func, distFunc);
			}
			if constexpr (std::is_base_of_v<U, E> || std::is_same_v<U, E>)
			{
				if constexpr (std::is_base_of_v<T, E> || std::is_same_v<T, E>)
				{
					threadFunc2<true, true, T, U, oneWay, mirror, E, E, Es...>(pos, func, distFunc);
				}
				else
				{
					threadFunc2<false, true, T, U, oneWay, mirror, E, E, Es...>(pos, func, distFunc);
				}
			}

			if constexpr (sizeof...(Es) > 0)
			{
				threadFunc1<T, U, oneWay, mirror, Es...>(pos, func, distFunc);
			}
		}

		template <bool oneWay, bool mirror, typename Param1, typename Param2, typename R, typename... E>
		void digestTupleThreadFunc(std::tuple<E...> *e, std::function<R(Param1 &, Param2 &)> func,
								   std::function<float(Param1 &)> dist, agl::Vec<int, 2> &pos)
		{
			threadFunc1<Param1, Param2, oneWay, mirror, E...>(pos, func, dist);
		}

		template <typename Funcs, int i, bool oneWay, bool mirror, typename Entis>
		inline void threadFunc0(agl::Vec<int, 2> pos, Funcs funcs)
		{
			digestTupleThreadFunc<oneWay, mirror>((Entis *)nullptr, std::get<i>(funcs), std::get<i + 1>(funcs), pos);

			if constexpr (i < (std::tuple_size<Funcs>::value - 2))
			{
				threadFunc0<Funcs, i + 2, oneWay, mirror, Entis>(pos, funcs);
			}
		}

		template <typename T, typename U, bool oneWay = false, bool mirror = false, typename O, typename E,
				  bool sameGrid = false>
		inline void gridUpdate(std::function<void(T &, U &)> func, agl::Vec<int, 2> gridPosition,
							   agl::Vec<int, 2> gridOffset, T *addressT, std::list<BaseEntity *>::iterator &it1)
		{
			auto &list2 =
				getListInGrid({gridOffset.x + gridPosition.x, gridOffset.y + gridPosition.y}, typeid(E).hash_code());

			std::list<BaseEntity *>::iterator it2 = sameGrid ? std::next(it1, 1) : list2.begin();

			for (; *it2 != *list2.end(); it2++)
			{
				U *addressU = (E *)(DoNotUse *)*it2;

				if constexpr (!sameGrid && std::is_same<T, U>())
				{
					if ((void *)addressU == (void *)addressT)
					{
						continue;
					}
				}

				func(*addressT, *addressU);

				if constexpr (mirror)
				{
					func(*addressU, *addressT);
				}
			}
		}

		template <typename R, typename P> constexpr static P testFunc(std::function<R(P &)>)
		{
			return;
		}

		template <typename T, typename F, typename... Fs> static void loopThrougFuncs(T &entitiy, F func, Fs... funcs)
		{
			using functype = decltype(testFunc(std::function(func)));

			if constexpr (std::is_base_of_v<functype, T> || std::is_same_v<functype, T>)
			{
				func(entitiy);
			}

			if constexpr (sizeof...(Fs) > 0)
			{
				loopThrougFuncs<T, Fs...>(entitiy, funcs...);
			}
		}

	public:
		struct GridCell
		{
				std::list<BaseEntity *> list;
				std::mutex				mtx;
		};

		std::map<std::size_t, std::list<BaseEntity *>>			  entityList;
		agl::Vec<float, 2>										  size;
		agl::Vec<int, 2>										  gridResolution;
		std::vector<std::vector<std::map<std::size_t, GridCell>>> grid;
		ThreadPool												  pool;
		agl::Vec<int, 2>										 *randomPosition;

		void *selected = nullptr;

		Environment() : pool(THREADS)
		{
		}

		void setThreads(int threads)
		{
			pool.ThreadPool::~ThreadPool();
			new (&pool) ThreadPool(threads);
		}

		void setupGrid(agl::Vec<float, 2> size, agl::Vec<int, 2> gridResolution)
		{
			this->size			 = size;
			this->gridResolution = gridResolution;

			grid.resize(gridResolution.x);
			for (auto &vec : grid)
			{
				vec.resize(gridResolution.y);
			}

			randomPosition = new agl::Vec<int, 2>[gridResolution.x * gridResolution.y];

			int index = 0;

			if (size.y > size.x)
			{
				for (int y = 0; y < gridResolution.y; y++)
				{
					for (int x = 0; x < gridResolution.x; x++)
					{
						randomPosition[index] = {x, y};
						index++;
					}
				}
			}
			else
			{
				for (int x = 0; x < gridResolution.x; x++)
				{
					for (int y = 0; y < gridResolution.y; y++)
					{
						randomPosition[index] = {x, y};
						index++;
					}
				}
			}
		}

		template <typename T> T &addEntity()
		{
			std::size_t hash = typeid(T).hash_code();

			T *t = new T();

			t->exists = true;
			entityList[hash].emplace_back((BaseEntity *)(DoNotUse *)t);

			return *t;
		}

		template <typename T> std::list<BaseEntity *> &getList()
		{
			return entityList[typeid(T).hash_code()];
		}

		template <typename Search, typename T, typename... Ts>
		inline void view(std::function<void(Search &, std::list<BaseEntity *>::iterator &)> func)
		{
			if constexpr (std::is_base_of_v<Search, T> || std::is_same_v<T, Search>)
			{
				std::size_t hashT = typeid(T).hash_code();

				auto &list = entityList[hashT];

				for (auto it = list.begin(); it != list.end(); it++)
				{
					Search *add = (T *)(DoNotUse *)*it;

					func(*add, it);
				}
			}

			if constexpr (sizeof...(Ts) > 0)
			{
				view<Search, Ts...>(func);
			}
		}

		template <typename Search, typename T, typename... Ts>
		inline void view(std::function<void(Search &, std::list<BaseEntity *>::iterator &)> func,
						 agl::Vec<int, 2> start, agl::Vec<int, 2> end)
		{
			if constexpr (std::is_base_of_v<Search, T> || std::is_same_v<T, Search>)
			{
				for (int x = start.x; x <= end.x; x++)
				{
					for (int y = start.y; y <= end.y; y++)
					{
						std::size_t hashT = typeid(T).hash_code();

						auto &list = getListInGrid({x, y}, hashT);

						for (auto it = list.begin(); it != list.end(); it++)
						{
							Search *add = (T *)(DoNotUse *)*it;

							func(*add, it);
						}
					}
				}
			}

			if constexpr (sizeof...(Ts) > 0)
			{
				view<Search, Ts...>(func, start, end);
			}
		}

		agl::Vec<int, 2> toGridPosition(agl::Vec<float, 2> position)
		{
			agl::Vec<int, 2> gridPosition;
			gridPosition.x = (position.x / size.x) * (gridResolution.x);
			gridPosition.y = (position.y / size.y) * (gridResolution.y);

			if (gridPosition.x < 0)
			{
				gridPosition.x = 0;
			}
			else if (gridPosition.x > (gridResolution.x - 1))
			{
				gridPosition.x = (gridResolution.x - 1);
			}

			if (gridPosition.y < 0)
			{
				gridPosition.y = 0;
			}
			else if (gridPosition.y > (gridResolution.y - 1))
			{
				gridPosition.y = (gridResolution.y - 1);
			}

			return gridPosition;
		}

		template <typename T> void addToGrid(BaseEntity &entity)
		{
			agl::Vec<int, 2> gridPosition = toGridPosition(entity.position);
			auto			&map		  = (grid[gridPosition.x][gridPosition.y])[typeid(T).hash_code()];

			map.mtx.lock();
			map.list.emplace_back(&entity);
			map.mtx.unlock();
		}

		template <typename T>
		void removeEntity(
			typename std::list<BaseEntity *>::iterator it, std::function<void(T &)> func = [](T &) {})
		{
			func(*(T *)(DoNotUse *)(*it));
			delete *it;

			agl::Vec<int, 2> gridPosition = toGridPosition((*it)->position);

			auto &list = grid.at(gridPosition.x).at(gridPosition.y)[typeid(T).hash_code()].list;

			auto i = list.begin();

			for (; i != list.end(); i++)
			{
				if (*i == *it)
				{
					list.erase(i);

					break;
				}
			}

			entityList[typeid(T).hash_code()].erase(it);
		}

		template <typename T> void getArea(std::function<void(T &)> func, agl::Vec<int, 2> gridPosition)
		{
			agl::Vec<int, 2> startGridOffset = {0, 0};
			agl::Vec<int, 2> endGridOffset	 = {1, 1};

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

					for (auto &entity : grid.at(gridPosition.x).at(gridPosition.y)[typeid(T).hash_code()].list)
					{
						if (!entity->exists)
						{
							continue;
						}

						func(*(T *)(DoNotUse *)(entity));
					}
				}
			}
		}

		std::list<BaseEntity *> &getListInGrid(agl::Vec<int, 2> pos, std::size_t hash)
		{
			return grid.at(pos.x).at(pos.y)[hash].list;
		}

		template <typename Entis, bool oneWay = false, bool mirror = false, typename... Funcs>
		void update(Funcs... funcs)
		{
			auto threadedQueue = [&](int start, int end) {
				pool.queue([&, start = start, end = end, funcs = std::tuple(funcs...)]() {
					for (int i = start; i <= end; i++)
					{
						threadFunc0<std::tuple<Funcs...>, 0, oneWay, mirror, Entis>(randomPosition[i], funcs);
					}
				});
			};

			int gridSize  = gridResolution.x * gridResolution.y;
			int chunkSize = gridSize / pool.size;
			int i		  = 0;

			for (; i < pool.size - 1; i++)
			{
				threadedQueue(i * chunkSize, (i * chunkSize) + chunkSize - 1);
			}

			if (i * chunkSize < gridSize)
			{
				threadedQueue(i * chunkSize, gridSize - 1);
			}
		}

		void clearGrid()
		{
			for (auto &x : grid)
			{
				for (auto &y : x)
				{
					for (auto &[key, cell] : y)
					{
						cell.list.clear();
					}
				}
			}
		}

		template <typename T, int i = 0, typename... Funcs> void selfUpdate(Funcs... funcs)
		{
			typedef std::remove_reference_t<decltype(std::tuple_element_t<i, T>())> EnTy;
			auto &list = entityList[typeid(EnTy).hash_code()];

			for (auto it = list.begin(); it != list.end(); it++)
			{
				EnTy *en = (EnTy *)(DoNotUse *)*it;

				loopThrougFuncs<EnTy, Funcs...>(*en, funcs...);

				if (!en->exists || std::isnan(en->position.x))
				{
					it--;
					list.erase(std::next(it, 1));
				}
				else
				{
					addToGrid<EnTy>(*(BaseEntity *)(DoNotUse *)en);
				}
			}

			if constexpr (i < (std::tuple_size<T>::value - 1))
			{
				selfUpdate<T, i + 1, Funcs...>(funcs...);
			}
		}

		void destroy()
		{
			for (auto &[f, s] : entityList)
			{
				for (auto &entity : s)
				{
					delete entity;
				}
			}

			clearGrid();
			entityList.clear();

			while (pool.active())
			{
			}
		}
};
