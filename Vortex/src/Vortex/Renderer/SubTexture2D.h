#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Asset/Asset.h"
#include "Vortex/Core/Math/Math.h"
#include "Vortex/Renderer/Texture.h"
#include "Vortex/Core/ReferenceCounting/SharedRef.h"

namespace Vortex {

	class VORTEX_API SubTexture2D : public Asset
	{
	public:
		SubTexture2D() = default;
		SubTexture2D(const SharedReference<Texture2D>& texture, const Math::vec2& min, const Math::vec2& max);
		~SubTexture2D() = default;

		VX_FORCE_INLINE const SharedReference<Texture2D>& GetTexure() const { return m_Texture; }
		VX_FORCE_INLINE const Math::vec2* GetTextureCoords() const { return m_TexCoords; }

		static AssetType GetStaticType() { return AssetType::SubTexture; }
		AssetType GetAssetType() const override { return GetStaticType(); }

		static SharedReference<SubTexture2D> CreateFromCoords(const SharedReference<Texture2D>& texture, const Math::vec2& coords, const Math::vec2& cellSize, const Math::vec2& spriteSize = Math::vec2(1.0f));

	private:
		SharedReference<Texture2D> m_Texture = nullptr;
		Math::vec2 m_TexCoords[4] = { {} };
	};

}
