#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Module/Version.h"

#include <string>
#include <vector>

namespace Vortex {

	struct VORTEX_API SubModuleProperties
	{
		std::string ModuleName;
		Version APIVersion;
		std::vector<std::string> RequiredModules;
	};

	class VORTEX_API SubModule
	{
	public:
		void Init(const SubModuleProperties& moduleProps)
		{
			m_Properties = moduleProps;
		}

		void Shutdown()
		{

		}

		const SubModuleProperties& GetProperties() const { return m_Properties; }

	private:
		SubModuleProperties m_Properties;
	};

}
