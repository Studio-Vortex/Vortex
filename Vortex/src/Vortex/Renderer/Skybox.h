#pragma once

#include "Vortex/Core/Base.h"

#include <string>

namespace Vortex {

	class VORTEX_API Skybox
	{
	public:
		virtual ~Skybox() = default;

		virtual void SetDirectoryPath(const std::string& directoryPath) = 0;
		virtual const std::string& GetDirectoryPath() const = 0;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual bool IsLoaded() const = 0;

		static SharedRef<Skybox> Create();
		static SharedRef<Skybox> Create(const std::string& directoryPath);
	};

}
