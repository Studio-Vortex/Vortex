#include "vxpch.h"
#include "ProjectType.h"

#include "Vortex/Core/String.h"

namespace Vortex {

	namespace Utils {

		ProjectType ProjectTypeFromString(const char* type)
		{
			if (String::FastCompare(type, "2D")) return ProjectType::e2D;
			if (String::FastCompare(type, "3D")) return ProjectType::e3D;

			VX_CORE_ASSERT(false, "Unknown Project Type!");
			return (ProjectType)0;
		}

		const char* ProjectTypeToString(ProjectType type)
		{
			switch (type)
			{
				case ProjectType::e2D: return "2D";
				case ProjectType::e3D: return "3D";
			}

			VX_CORE_ASSERT(false, "Unknown Project Type!");
			return "";
		}

	}

}
