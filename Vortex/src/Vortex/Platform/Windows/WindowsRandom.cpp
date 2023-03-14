#include "vxpch.h"
#include "Vortex/Utils/Random.h"

namespace Vortex {

	void Random::Init()
	{
		s_RandomEngine.seed(std::random_device()());
	}

	float Random::Float()
	{
		return (float)s_Distribution(s_RandomEngine) / (float)std::numeric_limits<uint32_t>::max();
	}

}
