#pragma once

#include "Sparky/Core/base.h"

#include <string>

namespace Sparky {

	class SPARKY_API Texture
	{
	public:
		virtual ~Texture() = default;

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
	};

	class SPARKY_API Texture2D : public Texture
	{
	public:
		static SharedRef<Texture2D> Create(uint32_t width, uint32_t height);
		static SharedRef<Texture2D> Create(const std::string& path, bool flipVertical = true);
	};

}
