#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Collections/ILibraryMap.h"

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
		std::string RequiredModuleName;
	};

	struct ModuleResult
	{
		bool Success;
		uint32_t UnresolvedModules;
		std::vector<RequiredModule> FailedModuleReferences;
	};

	class VORTEX_API ModuleLibrary : public ILibrary<std::string, SubModule>
	{
	public:
		ModuleLibrary() = default;
		~ModuleLibrary() override = default;

		virtual void Add(const Value& submodule) override;
		virtual uint8_t Remove(const Key& name) override;

		virtual bool Exists(const Key& name) const override;

		virtual Value& Get(const Key& name) override;
		virtual const Value& Get(const Key& name) const override;

		ModuleResult ResolveModules() const;
	};

}
