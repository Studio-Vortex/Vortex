#pragma once

#include "Vortex/Core/Base.h"

#include <string>

namespace Vortex {

	class VORTEX_API Skybox
	{
	public:
		virtual ~Skybox() = default;

		virtual void SetFilepath(const std::string& filepath) = 0;
		virtual const std::string& GetFilepath() const = 0;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual bool IsLoaded() const = 0;

		virtual bool PathChanged() const = 0;
		virtual void SetPathChanged(bool changed) = 0;

		virtual bool IsDirty() const = 0;
		virtual void SetIsDirty(bool dirty) = 0;

		virtual void Reload() = 0;

		virtual uint32_t GetRendererID() const = 0;

		static void Copy(SharedRef<Skybox> dstSkybox, const SharedRef<Skybox>& srcSkybox);

		static SharedRef<Skybox> Create();
		static SharedRef<Skybox> Create(const std::string& filepath);
	};

}
