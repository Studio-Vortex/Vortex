#pragma once

#include <vector>
#include <string>
#include <string_view>

namespace Vortex {

	class VORTEX_API String
	{
	public:
		static bool FastCompare(const std::string_view& lhs, const std::string_view& rhs);

		static std::vector<std::string> SplitString(const std::string_view& string, const std::string_view& delimiters);
		static std::vector<std::string> SplitString(const std::string_view& string, const char delimiter);

		static std::string ToUpper(std::string& string);
		static std::string ToUpperCopy(std::string_view& string);

		static std::string ToLower(std::string& string);
		static std::string ToLowerCopy(std::string_view& string);

		static void ReplaceToken(std::string& str, const char* token, const std::string& value);
	};

}
