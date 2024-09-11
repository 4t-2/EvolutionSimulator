#pragma once

#include <functional>
#include <list>
#include <tuple>
#include <type_traits>

template <typename... Ts> class TupleList
{
	private:
		std::tuple<std::list<Ts>...> tup;

	public:
		template <typename T> std::list<T> &getList()
		{
			return std::get<std::list<T>>(tup);
		}
};

template <typename... EntityTypes> class EnDex
{
	private:
		template <typename T, typename U, typename... Us> void privateView(std::function<void(T &)> &func)
		{
			if constexpr (std::is_base_of_v<T, U> || std::is_same_v<T, U>)
			{
				std::list<U> &list = entityList.template getList<U>();

				for (auto it = list.begin(); it != list.end(); it++)
				{
					func(*(T *)&*it);
				}
			}

			if constexpr (sizeof...(Us) > 0)
			{
				privateView<T, Us...>(func);
			}
		}

		template <typename T, typename U, typename Related, typename V, typename... Vs>
		void loopThroughU(std::function<void(T &, U &)> &func)
		{
			if constexpr (std::is_base_of_v<U, V> || std::is_same_v<U, V>)
			{
				if constexpr (!std::is_same_v<V, Related>)
				{
					std::list<V>	   &listV = entityList.template getList<V>();
					std::list<Related> &listR = entityList.template getList<Related>();

					for (auto &v : listV)
					{
						for (auto &r : listR)
						{
							func(*(T *)&r, *(V *)&v);
						}
					}
				}
			}

			if constexpr (sizeof...(Vs) > 0)
			{
				loopThroughU<T, U, Related, Vs...>(func);
			}
		}

		template <typename T, typename U, typename V, typename... Vs>
		void privateInteract(std::function<void(T &, U &)> &func)
		{
			if constexpr (std::is_base_of_v<T, V> || std::is_same_v<T, V>)
			{
				loopThroughU<T, U, V, V, Vs...>(func);
			}

			if constexpr (sizeof...(Vs) > 0)
			{
				privateInteract<T, U, Vs...>(func);
			}
		}

	public:
		TupleList<EntityTypes...> entityList;

		template <typename T> T &addEntity()
		{
			entityList.template getList<T>().emplace_back();

			return entityList.template getList<T>().back();
		}

		template <typename T> void view(std::function<void(T &)> func)
		{
			privateView<T, EntityTypes...>(func);
		}

		template <typename T, typename U> void interact(std::function<void(T &, U &)> func)
		{
			privateInteract<T, U, EntityTypes...>(func);
		}

		template <typename T> void removeEntity(typename std::list<T>::iterator it)
		{
			entityList.template getList<T>().erase(it);
		}

		template <typename T> void removeEntity(typename std::list<T>::iterator it, std::function<void(T &)> func)
		{
			func(*it);
			entityList.template getList<T>().erase(it);
		}

		template <typename T> std::list<T> &getList()
		{
			return entityList.template getList<T>();
		}
};
