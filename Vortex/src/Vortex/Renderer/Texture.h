#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Renderer/Image.h"

#include <string>

namespace Vortex {

	class VORTEX_API Texture
	{
	public:
		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		virtual const ImageProperties& GetProperties() const = 0;

		virtual const std::string& GetPath() const = 0;

		virtual void SetData(const void* data, uint32_t size) = 0;
		virtual void SetData(void* data, uint32_t size) = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;
		virtual void Unbind() const = 0;

		virtual bool IsLoaded() const = 0;

		virtual uint32_t GetRendererID() const = 0;

		virtual void SaveToFile() const = 0;

		virtual bool operator==(const Texture& other) const = 0;
	};

	class VORTEX_API Texture2D : public Texture
	{
	public:
		virtual ~Texture2D() override = default;

		static SharedRef<Texture2D> Create(const ImageProperties& imageProps);
	};

}
