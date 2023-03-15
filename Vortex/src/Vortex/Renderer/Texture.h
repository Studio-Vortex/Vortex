#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Asset/Asset.h"
#include "Vortex/Renderer/Image.h"
#include "Vortex/Core/ReferenceCounting/SharedRef.h"

#include <string>

namespace Vortex {

	struct TextureProperties
	{
		std::string Filepath = "";
		uint32_t Width = 0;
		uint32_t Height = 0;
		ImageWrap WrapMode = ImageWrap::Repeat;
		ImageFormat TextureFormat = ImageFormat::RGBA8;
		ImageFilter TextureFilter = ImageFilter::Linear;

		bool GenerateMipmaps = true;
		bool FlipVertical = true;
		bool IsLoaded = false;

		// Only used for writing to file
		uint32_t Channels = 0;
		const void* Buffer = nullptr;
		uint32_t Stride = 0;
	};

	class VORTEX_API Texture : public Asset
	{
	public:
		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		virtual const TextureProperties& GetProperties() const = 0;

		virtual const std::string& GetPath() const = 0;

		virtual void SetData(const void* data, uint32_t size) = 0;
		virtual void SetData(void* data, uint32_t size) = 0;

		virtual void SetPixel(uint32_t xOffset, uint32_t yOffset, void* data) = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;
		virtual void Unbind() const = 0;

		virtual bool IsLoaded() const = 0;

		virtual uint32_t GetRendererID() const = 0;

		virtual void SaveToFile() const = 0;

		virtual bool operator==(const Texture& other) const = 0;

		ASSET_CLASS_TYPE(TextureAsset)
	};

	class VORTEX_API Texture2D : public Texture
	{
	public:
		virtual ~Texture2D() override = default;

		static SharedReference<Texture2D> Create(const TextureProperties& imageProps);
	};

}
