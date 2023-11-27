#pragma once

#include <AGL/agl.hpp>
#include <cstddef>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <tuple>
#include <typeinfo>

class Environment;

class BaseEntity
{
	private:
		int test;
	public:
		bool			   exists = true;
		agl::Vec<float, 2> position;

		virtual ~BaseEntity()
		{
		}
};

template <typename... T> class Entity : virtual public BaseEntity, public T...
{
	public:
		const static std::tuple<T...> signature;

		Entity()
		{
		}

		virtual ~Entity()
		{
		}
};

template <typename... T> const std::tuple<T...> Entity<T...>::signature;

class Environment
{
	private:
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

		template <typename S, typename... Ts> void registerTraits(std::tuple<Ts...> signature)
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

	public:
		std::map<std::size_t, std::list<BaseEntity *>>							 entityList;
		std::map<std::size_t, std::vector<std::size_t>>							 traits;
		agl::Vec<float, 2>														 size;
		agl::Vec<int, 2>														 gridResolution;
		std::vector<std::vector<std::map<std::size_t, std::list<BaseEntity *>>>> grid;

		void setupGrid(agl::Vec<float, 2> size, agl::Vec<int, 2> gridResolution)
		{
			this->size			 = size;
			this->gridResolution = gridResolution;

			grid.resize(gridResolution.x);
			for (auto &vec : grid)
			{
				vec.resize(gridResolution.y);
			}
		}

		template <typename T> void setupTraits()
		{
			registerTraits<T>(T::signature);
		}

		template <typename T> T &addEntity()
		{
			std::size_t hash = typeid(T).hash_code();

			T *t = new T();

			entityList[hash].emplace_back(t);

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
				func(*dynamic_cast<T *>(*it), it);
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

			map[typeid(T).hash_code()].emplace_back(&entity);
		}

		template <typename T>
		void removeEntity(
			typename std::list<BaseEntity *>::iterator it, std::function<void(T &)> func = [](T &) {})
		{
			func(*dynamic_cast<T *>(*it));

			agl::Vec<int, 2> gridPosition = toGridPosition((*it)->position);

			auto &list = grid.at(gridPosition.x).at(gridPosition.y)[typeid(T).hash_code()];

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

					for (auto &entity : grid.at(gridPosition.x).at(gridPosition.y)[typeid(T).hash_code()])
					{
						if (!entity->exists)
						{
							continue;
						}

						func(*dynamic_cast<T *>(entity));
					}
				}
			}
		}

		std::list<BaseEntity *> &getListInGrid(agl::Vec<int, 2> pos, std::size_t hash)
		{
			return grid.at(pos.x).at(pos.y)[hash];
		}

		template <typename T, typename U, bool oneWay = false> void update(std::function<void(T &, U &)> func)
		{
			agl::Vec<int, 2> startGridOffset;
			if (!oneWay)
			{
				startGridOffset = {-1, -1};
			}

			agl::Vec<int, 2> endGridOffset = {1, 1};
			agl::Vec<int, 2> gridPosition  = {0, 0};

			for (gridPosition.x = 0; gridPosition.x < gridResolution.x; gridPosition.x++)
			{
				for (gridPosition.y = 0; gridPosition.y < gridResolution.y; gridPosition.y++)
				{
					for (int x = startGridOffset.x; x <= endGridOffset.x; x++)
					{
						if (gridPosition.x + x < 0 || gridPosition.x + x > (gridResolution.x - 1))
						{
							continue;
						}

						for (int y = startGridOffset.y; y <= endGridOffset.y; y++)
						{
							if (gridPosition.y + y < 0 || gridPosition.y + y > (gridResolution.y - 1))
							{
								continue;
							}

							for (auto hashT : traits[typeid(T).hash_code()])
							{
								for (auto hashU : traits[typeid(U).hash_code()])
								{
									auto &list1 = getListInGrid(gridPosition, hashT);
									auto &list2 = getListInGrid({x + gridPosition.x, y + gridPosition.y}, hashU);

									auto it1 = list1.begin();

									std::list<BaseEntity *>::iterator it2;

									if (std::is_same<T, U>() && &list1 == &list2)
									{
										it2 = it1;
									}
									else
									{
										it2 = list2.begin();
									}

									for (; it1 != list1.end(); it1++)
									{
										for (; it2 != list2.end(); it2++)
										{
											func(*dynamic_cast<T *>(*it1), *dynamic_cast<U *>(*it2));
										}
									}
								}
							}
						}
					}
				}
			}
		}

		template <typename T> void selfUpdate(std::function<void(T&)> func)
		{
			auto &list = entityList[typeid(T).hash_code()];

			for (auto it = list.begin(); it != list.end(); it++)
			{
				BaseEntity &entity = **it;

				func(*dynamic_cast<T*>(*it));

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
			float num  = 0;
			int	  size = 0;
			for (auto &x : grid)
			{
				for (auto &y : x)
				{
					for (auto &[key, list] : y)
					{
						num += list.size();
						list.clear();
					}
					size++;
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

						auto &list = grid.at(gridPosition.x).at(gridPosition.y)[key];

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
		}
};
