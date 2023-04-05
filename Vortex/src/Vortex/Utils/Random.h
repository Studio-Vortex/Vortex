#pragma once

#include "Vortex/Core/Base.h"

#include <random>

namespace Vortex {

	class VORTEX_API Random
	{
	public:
		static void Init();
		static float Float();

		inline static std::mt19937 s_RandomEngine;
		inline static std::uniform_int_distribution<std::mt19937::result_type> s_Distribution;
	};

}