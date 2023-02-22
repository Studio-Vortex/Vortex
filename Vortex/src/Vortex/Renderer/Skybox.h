#pragma once

#include "Vortex/Core/Base.h"

#include <string>

namespace Vortex {

	class VORTEX_API Skybox
	{
	public:
		virtual ~Skybox() = default;

		virtual void LoadFromFilepath(const std::string& filepath) = 0;
		virtual const std::string& GetFilepath() const = 0;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual bool IsLoaded() const = 0;

		virtual bool ShouldReload() const = 0;
		virtual void SetShouldReload(bool reload) = 0;

		virtual uint32_t GetRendererID() const = 0;

		static void Copy(SharedRef<Skybox> dstSkybox, const SharedRef<Skybox>& srcSkybox);

		static SharedRef<Skybox> Create();
		static SharedRef<Skybox> Create(const std::string& filepath);
	};

}
