#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/Math.h"
#include "Vortex/Asset/Asset.h"
#include "Vortex/Renderer/Texture.h"

namespace Vortex {

	class VORTEX_API SubTexture2D : public Asset
	{
	public:
		SubTexture2D() = default;
		SubTexture2D(const SharedRef<Texture2D>& texture, const Math::vec2& min, const Math::vec2& max);
		~SubTexture2D() override = default;

		VX_FORCE_INLINE const SharedRef<Texture2D>& GetTexure() const { return m_Texture; }
		VX_FORCE_INLINE const Math::vec2* GetTextureCoords() const { return m_TexCoords; }

		static AssetType GetStaticType() { return AssetType::Texture; }
		AssetType GetAssetType() const override { return AssetType::Texture; }

		static SharedRef<SubTexture2D> CreateFromCoords(const SharedRef<Texture2D>& texture, const Math::vec2& coords, const Math::vec2& cellSize, const Math::vec2& spriteSize = Math::vec2(1.0f));

	private:
		SharedRef<Texture2D> m_Texture = nullptr;
		Math::vec2 m_TexCoords[4] = { {} };
	};

}
