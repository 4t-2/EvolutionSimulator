#pragma once

#include <cstddef>
#include <functional>
#include <list>
#include <map>
#include <tuple>
#include <typeinfo>
#include <iostream>

class BaseEntity
{
	public:
		bool exists = true;

		virtual ~BaseEntity()
		{
		}
};

template <typename... T> class Entity : public BaseEntity, public T...
{
	public:
		const std::tuple<T...> signature;

		Entity()
		{
		}

		virtual ~Entity()
		{
		}
};

class EntityPointer
{
	public:
		BaseEntity *data = nullptr;

		template <typename T> EntityPointer(T *data)
		{
			this->data = (BaseEntity *)data;
		}
};

class EntityData : public EntityPointer
{
	public:
		template <typename T> EntityData(T *data) : EntityPointer(data)
		{
		}

		~EntityData()
		{
			delete data;
		}
};

class Environment
{
	private:
		template <typename T> void registerTraits(void *p)
		{
			std::size_t hash = typeid(T).hash_code();

			entityTraits[hash].emplace_back(p);
		}

		template <typename T, typename... Ts, typename std::enable_if<sizeof...(Ts) != 0>::type * = nullptr>
		void registerTraits(void *p)
		{
			std::size_t hash = typeid(T).hash_code();

			entityTraits[hash].emplace_back(p);

			registerTraits<Ts...>(p);
		}

		template <typename... Ts> void registerTraits(void *p, std::tuple<Ts...> signature)
		{
			registerTraits<Ts...>(p);
		}

		template <typename T> void getListFromTypes(std::list<EntityPointer> **listArray)
		{
			std::size_t hash = typeid(T).hash_code();
			listArray[0]	 = &entityTraits[hash];
		}

		template <typename T, typename... Ts, typename std::enable_if<sizeof...(Ts) != 0>::type * = nullptr>
		void getListFromTypes(std::list<EntityPointer> **listArray)
		{
			std::size_t hash = typeid(T).hash_code();

			listArray[sizeof...(Ts)] = &entityTraits[hash];

			getListFromTypes<Ts...>(listArray);
		}

		template <int i = 0, typename... Ts, typename std::enable_if<(i == sizeof...(Ts))>::type * = nullptr>
		void getArgsFromIterators(std::tuple<Ts *...> &args, std::list<EntityPointer>::iterator it[])
		{
		}

		template <int i = 0, typename... Ts, typename std::enable_if<(i < sizeof...(Ts))>::type * = nullptr>
		void getArgsFromIterators(std::tuple<Ts *...> &args, std::list<EntityPointer>::iterator it[])
		{
			BaseEntity *ep = it[i]->data;

			std::get<i>(args) = std::remove_pointer_t<decltype(std::get<i>(args))>(ep);

			getArgsFromIterators<i + 1, Ts...>(args, it);
		}

	public:
		std::map<std::size_t, std::list<EntityData>>	entityList;
		std::map<std::size_t, std::list<EntityPointer>> entityTraits;

		template <typename T> T &addEntity()
		{
			std::size_t hash = typeid(T).hash_code();

			T *t = new T();

			entityList[hash].emplace_back(t);

			registerTraits(t, t->signature);
			entityTraits[hash].emplace_back(t);

			return *t;
		}

		template <typename T> std::list<EntityData> &getList()
		{
			return entityList[typeid(T).hash_code()];
		}

		template <typename T> void view(std::function<void(T&, std::list<EntityData>::iterator&)> func)
		{
			for(auto it = getList<T>().begin(); it != getList<T>().end(); it++)
			{
				func(*(T*)it->data, it);
			}
		}

		template <typename T> void removeEntity(typename std::list<EntityData>::iterator it, std::function<void(T&)> func = [](T&){})
		{
			func(*(T*)it->data);			

			for (auto &[num, list] : entityTraits)
			{
				for (auto i = list.begin(); i != list.end(); i++)
				{
					if(i->data == it->data)
					{
						list.erase(i);
						break;
					}
				}
			}

			entityList[typeid(T).hash_code()].erase(it);
		}

		template <typename... Ts> void update(std::function<void(Ts *...)> updateFunc)
		{
			std::list<EntityPointer> *toUpdate[sizeof...(Ts)];

			getListFromTypes<Ts...>(toUpdate);

			std::list<EntityPointer>::iterator index[sizeof...(Ts)];

			for (int i = 0; i < sizeof...(Ts); i++)
			{
				index[i] = toUpdate[i]->begin();
			}

			int i = 0;
			while (i < sizeof...(Ts))
			{
				std::tuple<Ts *...> entities{};

				getArgsFromIterators(entities, index);

				std::apply(updateFunc, entities);

				for (i = 0; i < sizeof...(Ts); i++)
				{
					index[i]++;

					if (index[i] != toUpdate[i]->end())
					{
						break;
					}
					else
					{
						index[i] = toUpdate[i]->begin();
					}
				}
			}
		}

		template <typename T, typename U> void twinUpdate(std::function<void(T *interactor, U *interactee)> func)
		{
			std::list<EntityPointer> &interactor = entityTraits[typeid(T).hash_code()];
			std::list<EntityPointer> &interactee = entityTraits[typeid(U).hash_code()];

			for (EntityPointer &interactor : interactor)
			{
				for (EntityPointer &interactee : interactee)
				{
					func((T *)interactor.data, (U *)interactee.data);
				}
			}
		}

		template <typename T> void selfUpdate(std::function<void(T *interactor, T *interactee)> func)
		{
			std::list<EntityPointer> &list = entityTraits[typeid(T).hash_code()];

			for (auto it1 = list.begin(); it1 != list.end(); it1++)
			{
				for (auto it2 = it1; it2 != list.end(); it2++)
				{
					func((T *)it1->data, (T *)it2->data);
				}
			}
		}

		void update()
		{
			// interupdate

			// update selves

			for (auto &pair : entityTraits)
			{
				for (EntityPointer &pointer : pair.second)
				{
					BaseEntity &entity = *pointer.data;
					// entity.update();
				}
			}

			for (auto &pair : entityList)
			{
				for (auto it = pair.second.begin(); it != pair.second.end(); it++)
				{
					BaseEntity &entity = *it->data;
					// entity.update();

					if (!entity.exists)
					{
						auto next = std::next(it, -1);

						it = next;
					}
				}
			}
		}

		void destroy()
		{
			entityList.clear();
			entityTraits.clear();
		}
};
