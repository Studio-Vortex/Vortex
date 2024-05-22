#pragma once

#include "Vortex/Asset/Asset.h"

#include "Vortex/ReferenceCounting/SharedRef.h"

#include "Vortex/Renderer/Texture.h"

#include "Vortex/Utils/FileSystem.h"

namespace Vortex {

	class VORTEX_API Skybox : public Asset
	{
	public:
		virtual ~Skybox() = default;

		virtual void LoadFromFilepath(const std::filesystem::path& filepath) = 0;
		virtual const std::filesystem::path& GetFilepath() const = 0;

		virtual SharedReference<Texture2D> GetEnvironmentMap() const = 0;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual bool IsLoaded() const = 0;

		virtual bool ShouldReload() const = 0;
		virtual void SetShouldReload(bool reload) = 0;

		virtual uint32_t GetRendererID() const = 0;

		static void Copy(SharedReference<Skybox>& dstSkybox, const SharedReference<Skybox>& srcSkybox);

		ASSET_CLASS_TYPE(EnvironmentAsset)

		static SharedReference<Skybox> Create();
		static SharedReference<Skybox> Create(const std::filesystem::path& filepath);
	};

}
