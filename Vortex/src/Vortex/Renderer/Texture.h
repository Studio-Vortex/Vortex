#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Asset/Asset.h"

#include <string>

namespace Vortex {

	enum class VORTEX_API TextureWrap
	{
		None = 0,
		Clamp,
		Repeat,
	};

	class VORTEX_API Texture : public Asset
	{
	public:
		virtual ~Texture() override = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		virtual const std::string& GetPath() const = 0;

		virtual void SetData(const void* data, uint32_t size) = 0;
		virtual void SetData(void* data, uint32_t size) = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;
		virtual void Unbind() const = 0;

		virtual bool IsLoaded() const = 0;

		virtual uint32_t GetRendererID() const = 0;

		virtual bool operator==(const Texture& other) const = 0;

		static AssetType GetStaticType() { return AssetType::Texture; }
		AssetType GetAssetType() const override { return AssetType::Texture; }
	};

	class VORTEX_API Texture2D : public Texture
	{
	public:
		virtual ~Texture2D() override = default;

		// TODO: rework the last parameter
		static SharedRef<Texture2D> Create(uint32_t width, uint32_t height, bool rgba32f = false);
		static SharedRef<Texture2D> Create(const std::string& path, TextureWrap wrapMode = TextureWrap::Repeat, bool flipVertical = true);
	};

}
