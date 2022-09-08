#pragma once

#include "Sparky/Core.h"

#include <string>

namespace Sparky {

	class Texture
	{
	public:
		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;
	};

	class Texture2D : public Texture
	{
	public:
		static SharedRef<Texture2D> Create(const std::string& path, bool flipVertical = true);
	};

}