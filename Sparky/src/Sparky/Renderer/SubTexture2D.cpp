#include "sppch.h"
#include "SubTexture2D.h"

namespace Sparky {

	SubTexture2D::SubTexture2D(const SharedRef<Texture2D>& texture, const Math::vec2& min, const Math::vec2& max)
		: m_Texture(texture)
	{
		m_TexCoords[0] = { min.x, min.y };
		m_TexCoords[1] = { max.x, min.y };
		m_TexCoords[2] = { max.x, max.y };
		m_TexCoords[3] = { min.x, max.y };
	}

	SharedRef<SubTexture2D> SubTexture2D::CreateFromCoords(const SharedRef<Texture2D>& texture, const Math::vec2& coords, const Math::vec2& cellSize, const Math::vec2& spriteSize)
	{
		Math::vec2 min = { (coords.x * cellSize.x) / texture->GetWidth(), (coords.y * cellSize.y) / texture->GetHeight() };
		Math::vec2 max = { ((coords.x + spriteSize.x) * cellSize.x) / texture->GetWidth(), ((coords.y + spriteSize.y) * cellSize.y) / texture->GetHeight() };

		return CreateShared<SubTexture2D>(texture, min, max);
	}

}