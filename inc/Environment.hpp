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
#include <tuple>
#include <typeinfo>

class Environment;

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
		const static Signature<T...> signature;

		Entity(bool &exists, agl::Vec<float, 2> &position) : DoNotUse(exists, position), T(exists, position)...
		{
		}

		virtual ~Entity()
		{
		}
};

template <typename... T> const Signature<T...> Entity<T...>::signature;

class Environment
{
	private:
		template <typename S> void registerTraits()
		{
		}

		template <typename S, typename T> void registerTraits()
		{
			std::size_t eHash = typeid(S).hash_code();
			std::size_t tHash = typeid(T).hash_code();

			traits[tHash].emplace_back(eHash);
		}

		template <typename S, typename T, typename... Ts, typename std::enable_if<sizeof...(Ts) != 0>::type * = nullptr>
		void registerTraits()
		{
			std::size_t eHash = typeid(S).hash_code();
			std::size_t tHash = typeid(T).hash_code();

			traits[tHash].emplace_back(eHash);

			registerTraits<Ts...>();
		}

		template <typename S, typename... Ts> void registerTraits(Signature<Ts...> signature)
		{
			registerTraits<S, Ts...>();

			std::size_t eHash = typeid(S).hash_code();
			traits[eHash].emplace_back(eHash);
		}

		template <int i = 0, typename... Ts, typename std::enable_if<(i == sizeof...(Ts))>::type * = nullptr>
		void getArgsFromIterators(std::tuple<Ts *...> &args, std::list<BaseEntity *>::iterator it[])
		{
		}

		template <int i = 0, typename... Ts, typename std::enable_if<(i < sizeof...(Ts))>::type * = nullptr>
		void getArgsFromIterators(std::tuple<Ts *...> &args, std::list<BaseEntity *>::iterator it[])
		{
			std::get<i>(args) = std::remove_pointer_t<decltype(std::get<i>(args))>(it[i]);

			getArgsFromIterators<i + 1, Ts...>(args, it);
		}

		template <typename T> void addToTraitMap()
		{
		}
		template <typename T, typename U> void addToTraitMap()
		{
			long long offset = (long long)(U *)(T *)(1) - (long long)(BaseEntity *)(DoNotUse *)(T *)1;
			traitMap[std::pair(typeid(T).hash_code(), typeid(U).hash_code())] = offset;
		}
		template <typename T, typename U, typename... Us,
				  typename std::enable_if<(sizeof...(Us) > 0)>::type * = nullptr>
		void addToTraitMap()
		{
			long long offset = (long long)(U *)(T *)(1) - (long long)(BaseEntity *)(DoNotUse *)(T *)1;
			traitMap[std::pair(typeid(T).hash_code(), typeid(U).hash_code())] = offset;
		}

		template <typename T, typename... Us> void addToTraitMap(Signature<Us...> sig)
		{
			addToTraitMap<T, Us...>();
		}

	public:
		struct GridCell
		{
				std::list<BaseEntity *> list;
				std::mutex				mtx;
		};

		std::map<std::size_t, std::list<BaseEntity *>>			  entityList;
		std::map<std::size_t, std::vector<std::size_t>>			  traits;
		std::map<std::pair<std::size_t, std::size_t>, long long>  traitMap;
		agl::Vec<float, 2>										  size;
		agl::Vec<int, 2>										  gridResolution;
		std::vector<std::vector<std::map<std::size_t, GridCell>>> grid;
		ThreadPool												  pool;
		agl::Vec<int, 2>										 *randomPosition;

		Environment() : pool(10)
		{
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

			for (int y = 0; y < gridResolution.y; y++)
			{
				for (int x = 0; x < gridResolution.x; x++)
				{
					randomPosition[index] = {x, y};
					index++;
				}
			}

			std::shuffle(randomPosition, randomPosition + index, std::default_random_engine(0));
		}

		template <typename T> void setupTraits()
		{
			registerTraits<T>(T::signature);
			addToTraitMap<T>(T::signature);
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

		template <typename T> void view(std::function<void(T &, std::list<BaseEntity *>::iterator &)> func)
		{
			for (auto it = getList<T>().begin(); it != getList<T>().end(); it++)
			{
				func(*(T *)(DoNotUse *)(*it), it);
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
			auto			&map		  = grid[gridPosition.x][gridPosition.y];

			map[typeid(T).hash_code()].list.emplace_back(&entity);
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

		template <typename T, typename U, bool oneWay = false, bool mirror = false>
		void gridUpdate(std::function<void(T &, U &, std::size_t, std::size_t)> func, agl::Vec<int, 2> gridPosition,
						agl::Vec<int, 2> gridOffset, std::size_t hashT, std::size_t hashU)
		{
			long long offsetT;

			if constexpr (!std::is_base_of_v<DoNotUse, T>)
			{
				offsetT = traitMap[std::pair(hashT, typeid(T).hash_code())];
			}

			long long offsetU;

			if constexpr (!std::is_base_of_v<DoNotUse, U>)
			{
				offsetT = traitMap[std::pair(hashU, typeid(T).hash_code())];
			}

			auto &list1 = getListInGrid(gridPosition, hashT);
			auto &list2 = getListInGrid({gridOffset.x + gridPosition.x, gridOffset.y + gridPosition.y}, hashU);

			auto it1		= list1.begin();
			auto list2Begin = list2.begin();

			std::list<BaseEntity *>::iterator &it2Start = &list1 == &list2 ? it1 : list2Begin;

			for (; it1 != list1.end(); it1++)
			{
				std::list<BaseEntity *>::iterator it2 = it2Start;

				if (&list1 == &list2)
				{
					it2++;
				}

				for (; it2 != list2.end(); it2++)
				{
					T *addressT;
					U *addressU;

					if constexpr (std::is_base_of_v<DoNotUse, T>)
					{
						addressT = (T *)(DoNotUse *)(*it1);
					}
					else
					{
						addressT = (T *)((long long)*it1 + (long long)offsetT);
					}
					if constexpr (std::is_base_of_v<DoNotUse, U>)
					{
						addressU = (U *)(DoNotUse *)(*it2);
					}
					else
					{
						addressU = (U *)((long long)*it2 + (long long)offsetT);
					}

					func(*addressT, *addressU, hashT, hashU);

					if constexpr (mirror)
					{
						func(*addressU, *addressT, hashU, hashT);
					}
				}
			}
		}

		template <typename T, typename U, bool oneWay = false, bool mirror = false>
		void update(std::function<void(T &, U &, std::size_t, std::size_t)> func)
		{
			agl::Vec<int, 2> startGridOffset = {-1, -1};

			agl::Vec<int, 2> endGridOffset = {1, 1};

			for (int i = 0; i < (gridResolution.x * gridResolution.y); i++)
			{
				agl::Vec<int, 2> &gridPosition = randomPosition[i];
				pool.queue([&, gridPosition = gridPosition, func = func, startGridOffset = startGridOffset,
							endGridOffset = endGridOffset]() {
					for (auto hashT : traits[typeid(T).hash_code()])
					{
						for (auto hashU : traits[typeid(U).hash_code()])
						{
							if constexpr (!oneWay)
							{
								for (int y = startGridOffset.y; y < 0; y++)
								{
									if (gridPosition.y + y < 0 || gridPosition.y + y > (gridResolution.y - 1))
									{
										continue;
									}
									for (int x = startGridOffset.x; x < 0; x++)
									{
										if (gridPosition.x + x < 0 || gridPosition.x + x > (gridResolution.x - 1))
										{
											continue;
										}

										grid[gridPosition.x + x][gridPosition.y +
										y][hashU].mtx.lock();
										grid[gridPosition.x][gridPosition.y][hashT].mtx.lock();
										gridUpdate<T, U, oneWay, mirror>(func, gridPosition, {x, y}, hashT, hashU);
										grid[gridPosition.x][gridPosition.y][hashT].mtx.unlock();
										grid[gridPosition.x + x][gridPosition.y +
										y][hashU].mtx.unlock();
									}
								}
							}

							grid[gridPosition.x][gridPosition.y][hashT].mtx.lock();
							if (hashT != hashU)
							{
								grid[gridPosition.x][gridPosition.y][hashU].mtx.lock();
							}
							gridUpdate<T, U, oneWay, mirror>(func, gridPosition, {0, 0}, hashT, hashU);
							if (hashT != hashU)
							{
								grid[gridPosition.x][gridPosition.y][hashU].mtx.unlock();
							}
							grid[gridPosition.x][gridPosition.y][hashT].mtx.unlock();

							for (int y = 1; y <= endGridOffset.y; y++)
							{
								if (gridPosition.y + y < 0 || gridPosition.y + y > (gridResolution.y - 1))
								{
									continue;
								}
								for (int x = 1; x <= startGridOffset.x; x++)
								{
									if (gridPosition.x + x < 0 || gridPosition.x + x > (gridResolution.x - 1))
									{
										continue;
									}

									grid[gridPosition.x][gridPosition.y][hashT].mtx.lock();
									grid[gridPosition.x + x][gridPosition.y +
									y][hashU].mtx.lock();
									gridUpdate<T, U, oneWay, mirror>(func, gridPosition, {x, y}, hashT, hashU);
									grid[gridPosition.x + x][gridPosition.y +
									y][hashU].mtx.lock();
									grid[gridPosition.x][gridPosition.y][hashT].mtx.lock();
								}
							}
						}
					}
				});
			}
		}

		template <typename T> void selfUpdate(std::function<void(T &)> func)
		{
			auto &list = entityList[typeid(T).hash_code()];

			for (auto it = list.begin(); it != list.end(); it++)
			{
				BaseEntity &entity = **it;

				func(*(T *)(DoNotUse *)(*it));

				if (!entity.exists)
				{
					it--;
					list.erase(std::next(it, 1));
				}
				else
				{
					addToGrid<T>(entity);
				}
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

		// template <typename... Ts> void update(std::function<void(Ts *...)>
		// updateFunc)
		// {
		// 	std::list<EntityPointer> *toUpdate[sizeof...(Ts)];
		//
		// 	getListFromTypes<Ts...>(toUpdate);
		//
		// 	std::list<EntityPointer>::iterator index[sizeof...(Ts)];
		//
		// 	for (int i = 0; i < sizeof...(Ts); i++)
		// 	{
		// 		index[i] = toUpdate[i]->begin();
		// 	}
		//
		// 	int i = 0;
		// 	while (i < sizeof...(Ts))
		// 	{
		// 		std::tuple<Ts *...> entities{};
		//
		// 		getArgsFromIterators(entities, index);
		//
		// 		std::apply(updateFunc, entities);
		//
		// 		for (i = 0; i < sizeof...(Ts); i++)
		// 		{
		// 			index[i]++;
		//
		// 			if (index[i] != toUpdate[i]->end())
		// 			{
		// 				break;
		// 			}
		// 			else
		// 			{
		// 				index[i] = toUpdate[i]->begin();
		// 			}
		// 		}
		// 	}
		// }

		// template <typename T, typename U> void twinUpdate(std::function<void(T
		// *interactor, U *interactee)> func)
		// {
		// 	std::list<EntityPointer> &interactor =
		// entityTraits[typeid(T).hash_code()]; 	std::list<EntityPointer>
		// &interactee = entityTraits[typeid(U).hash_code()];
		//
		// 	for (EntityPointer &interactor : interactor)
		// 	{
		// 		for (EntityPointer &interactee : interactee)
		// 		{
		// 			func((T *)interactor.data, (U *)interactee.data);
		// 		}
		// 	}
		// }

		// template <typename T> void selfUpdate(std::function<void(T *interactor, T
		// *interactee)> func)
		// {
		// 	std::list<EntityPointer> &list = entityTraits[typeid(T).hash_code()];
		//
		// 	for (auto it1 = list.begin(); it1 != list.end(); it1++)
		// 	{
		// 		for (auto it2 = it1; it2 != list.end(); it2++)
		// 		{
		// 			func((T *)it1->data, (T *)it2->data);
		// 		}
		// 	}
		// }

		// void update()
		// {
		// 	// interupdate
		//
		// 	// update selves
		//
		// 	for (auto &pair : entityTraits)
		// 	{
		// 		for (EntityPointer &pointer : pair.second)
		// 		{
		// 			BaseEntity &entity = *pointer.data;
		// 			entity.update();
		// 		}
		// 	}
		//
		// 	for (auto &pair : entityList)
		// 	{
		// 		for (auto it = pair.second.begin(); it != pair.second.end();
		// it++)
		// 		{
		// 			BaseEntity &entity = *it->data;
		// 			entity.update();
		//
		// 			if (!entity.exists)
		// 			{
		// 				auto next = std::next(it, -1);
		//
		// 				it = next;
		// 			}
		// 		}
		// 	}
		// }

		void keepExisters()
		{
			for (auto &[key, list] : entityList)
			{
				for (auto it = list.begin(); it != list.end(); it++)
				{
					if (!(*it)->exists)
					{
						auto next = std::next(it, -1);

						agl::Vec<int, 2> gridPosition = toGridPosition((*it)->position);

						auto &list = grid.at(gridPosition.x).at(gridPosition.y)[key].list;

						auto i = list.begin();

						for (; i != list.end(); i++)
						{
							if (*i == *it)
							{
								break;
							}
						}

						list.erase(i);

						entityList[key].erase(it);

						it = next;
					}
				}
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
