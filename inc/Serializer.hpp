#pragma once

#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

template <typename T, typename U, typename = std::enable_if_t<std::is_arithmetic<T>::value>>
void recurse(U processor, T &v, std::string name = "null")
{
	processor.process(name, v);
}

#define RECSER(x) recurse(processor, x, std::string(#x).substr(2))

class Output
{
	public:
		std::ostream &stream;
		int			  indent = 0;

		Output(std::ostream &stream) : stream(stream)
		{
		}

		Output(Output &o) : stream(o.stream), indent(o.indent + 1)
		{
		}

		template <typename T> void process(std::string name, T &value)
		{
			for (int i = 0; i < indent; i++)
			{
				stream << "\t";
			}

			if constexpr (std::is_arithmetic<T>::value)
			{
				stream << name << " = " << value << '\n';
			}
			else
			{
				stream << name << " :\n";
			}
		}
};

class Input
{
	public:
		std::istream &stream;

		Input(std::istream &stream) : stream(stream)
		{
		}

		template <typename T> void process(std::string name, T &value)
		{
			std::string buf;

			std::getline(stream, buf);

			buf = trim(buf);

			auto vec = stringSplit(buf, ' ');

			if (vec[1] == "=")
			{
				value = unstring<T>(vec[2]);
			}
		}

		static std::vector<std::string> stringSplit(const std::string &str, char delimiter)
		{
			std::istringstream		 iss(str);
			std::string				 token;
			std::vector<std::string> tokens;

			while (std::getline(iss, token, delimiter))
			{
				tokens.push_back(token);
			}

			return tokens;
		}

		static std::string trim(const std::string &str)
		{
			const auto start = str.find_first_not_of(" \t");
			if (start == std::string::npos)
			{
				return "";
			}

			const auto end = str.find_last_not_of(" \t");
			return str.substr(start, end - start + 1);
		}

		template <typename T> T unstring(std::string &val)
		{
			if constexpr (std::is_same<T, char>())
			{
				return val[0];
			}
			if constexpr (std::is_integral<T>())
			{
				return std::stoll(val);
			}
			else if constexpr (std::is_same<T, float>())
			{
				return std::stof(val);
			}
			else if constexpr (std::is_same<T, double>())
			{
				return std::stod(val);
			}

			return T();
		}
};
