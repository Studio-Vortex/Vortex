#include "vxpch.h"
#include "SystemTypes.h"

#include "Vortex/Core/String.h"

namespace Vortex {

	namespace Utils {

		SystemType SystemTypeFromString(const std::string& type)
		{
			if (String::FastCompare(type, "UI")) return SystemType::UI;

			VX_CORE_ASSERT(false, "Uknown System Type!");
			return SystemType::None;
		}

		std::string StringFromSystemType(SystemType type)
		{
			switch (type)
			{
				case SystemType::UI: return "UI";
			}

			VX_CORE_ASSERT(false, "Uknown System Type!");
			return "None";
		}

	}

}
