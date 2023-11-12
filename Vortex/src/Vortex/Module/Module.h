#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Core/LibraryBase.h"

#include "Vortex/Module/Version.h"

#include <string>

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
		SubModule() = default;
		~SubModule() = default;

		void Init(const SubModuleProperties& moduleProps)
		{
			m_Properties = moduleProps;
		}

		void Shutdown()
		{

		}

		const std::string& GetName() const { return m_Properties.ModuleName; }
		const Version& GetAPIVersion() const { return m_Properties.APIVersion; }
		const std::vector<std::string>& GetRequiredModules() const { return m_Properties.RequiredModules; }

		const SubModuleProperties& GetProperties() const { return m_Properties; }

	private:
		SubModuleProperties m_Properties;
	};

	struct RequiredModule
	{
		std::string ModuleName;
		std::string RequiredModule;
	};

	struct ModuleResult
	{
		bool Success;
		uint32_t UnresolvedModules;
		std::vector<RequiredModule> FailedModuleReferences;
	};

	class VORTEX_API ModuleLibrary : public LibraryBase<SubModule, std::string>
	{
	public:
		ModuleLibrary() = default;
		~ModuleLibrary() override = default;

		void Add(const SubModule& submodule) override;
		uint8_t Remove(const std::string& name) override;

		bool Exists(const std::string& name) const override;

		SubModule& Get(const std::string& name) override;
		const SubModule& Get(const std::string& name) const override;

		ModuleResult ResolveModules() const;
	};

}
