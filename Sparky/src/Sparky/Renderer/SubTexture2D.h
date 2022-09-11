#pragma once

#include "Sparky/Core/Base.h"

#include "Sparky/Renderer/Texture.h"
#include "Sparky/Core/Math.h"

namespace Sparky {

	class SPARKY_API SubTexture2D
	{
	public:
		SubTexture2D(const SharedRef<Texture2D>& texture, const Math::vec2& min, const Math::vec2& max);
		~SubTexture2D() = default;

		const SharedRef<Texture2D>& GetTexure() const { return m_Texture; }
		const Math::vec2* GetTextureCoords() const { return m_TexCoords; }

		static SharedRef<SubTexture2D> CreateFromCoords(const SharedRef<Texture2D>& texture, const Math::vec2& coords, const Math::vec2& cellSize, const Math::vec2& spriteSize = Math::vec2(1.0f));

	private:
		SharedRef<Texture2D> m_Texture;

		Math::vec2 m_TexCoords[4];
	};

}
