#pragma once

#include <vector>
#include <string>

namespace Vortex {

	class String
	{
	public:
		static std::vector<std::string> SplitString(const std::string_view string, const std::string_view& delimiters);
		static std::vector<std::string> SplitString(const std::string_view string, const char delimiter);
	};

}
