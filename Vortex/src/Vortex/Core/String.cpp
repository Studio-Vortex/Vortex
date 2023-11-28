#include "vxpch.h"
#include "String.h"

namespace Vortex {

	std::vector<std::string> String::SplitString(const std::string_view string, const std::string_view& delimiters)
	{
		size_t first = 0;

		std::vector<std::string> result;

		while (first <= string.size())
		{
			const auto second = string.find_first_of(delimiters, first);

			if (first != second)
				result.emplace_back(string.substr(first, second - first));

			if (second == std::string::npos)
				break;

			first = second + 1;
		}

		return result;
	}

	std::vector<std::string> String::SplitString(const std::string_view string, const char delimiter)
	{
		return SplitString(string, std::string(1, delimiter));
	}

    std::string String::ToUpper(std::string& string)
    {
		std::transform(string.begin(), string.end(), string.begin(),
		[](const unsigned char c)
		{
			return std::toupper(c);
		});

		return string;
    }

    std::string String::ToUpperCopy(std::string_view& string)
    {
		std::string result(string);
		ToUpper(result);
		return result;
    }

    std::string String::ToLower(std::string& string)
    {
		std::transform(string.begin(), string.end(), string.begin(),
		[](const unsigned char c)
		{
			return std::tolower(c);
		});

		return string;
    }

    std::string String::ToLowerCopy(std::string_view& string)
    {
		std::string result(string);
		ToLower(result);
		return result;
    }

	void String::ReplaceToken(std::string& str, const char* token, const std::string& value)
	{
		size_t pos = 0;

		while ((pos = str.find(token, pos)) != std::string::npos)
		{
			size_t length = strlen(token);
			str.replace(pos, length, value);
			pos += length;
		}
	}

}
