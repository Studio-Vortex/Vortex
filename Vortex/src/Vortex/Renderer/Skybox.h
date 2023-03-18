#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Asset/Asset.h"
#include "Vortex/Core/ReferenceCounting/SharedRef.h"

#include <string>

namespace Vortex {

	class VORTEX_API Skybox : public Asset
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

		static void Copy(SharedReference<Skybox>& dstSkybox, const SharedReference<Skybox>& srcSkybox);

		ASSET_CLASS_TYPE(EnvironmentAsset)

		static SharedReference<Skybox> Create();
		static SharedReference<Skybox> Create(const std::string& filepath);
	};

}
