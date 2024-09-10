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

	public:
		TupleList<EntityTypes...> entityList;

		template <typename T> void addEntity()
		{
			entityList.template getList<T>().emplace_back();
		}

		template <typename T> void view(std::function<void(T &)> func)
		{
			privateView<T, EntityTypes...>(func);
		}
};
