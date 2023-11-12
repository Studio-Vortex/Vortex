#include "vxpch.h"
#include "Module.h"

namespace Vortex {

	void SubModuleLibrary::Add(const SubModule& submodule)
	{
		VX_CORE_ASSERT(!Exists(submodule.GetName()), "SubModule already exists!");
		m_LibraryData.push_back(submodule);
	}

	uint8_t SubModuleLibrary::Remove(const std::string& name)
	{
		VX_CORE_ASSERT(Exists(name), "Unknown module name!");

		for (size_t i = 0; i < Size(); i++)
		{
			if (name == m_LibraryData[i].GetName())
			{
				m_LibraryData.erase(m_LibraryData.begin() + i);
				return 0;
			}
		}

		return 1;
	}

	bool SubModuleLibrary::Exists(const std::string& name) const
	{
		for (const auto& submodule : *this)
		{
			if (name == submodule.GetName())
			{
				return true;
			}
		}

		return false;
	}

	SubModule& SubModuleLibrary::Get(const std::string& name)
	{
		if (!Exists(name)) {
			VX_CORE_ASSERT(false, "Error: Unknown module '{}', cannot access invalid module", name);
			return m_LibraryData[0];
		}

		for (auto& submodule : *this)
		{
			if (name == submodule.GetName())
			{
				return submodule;
			}
		}
	}

	const SubModule& SubModuleLibrary::Get(const std::string& name) const
	{
		return Get(name);
	}

}
