#pragma once

#include "Sparky/Renderer/OrthographicCamera.h"
#include "Sparky/Renderer/Texture.h"
#include "Sparky/Renderer/Color.h"
#include "Sparky/Core/Math.h"

namespace Sparky {

	class Renderer2D
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const OrthographicCamera& camera);
		static void EndScene();

		// Primitives

		static void DrawQuad(const Math::vec2& position, const Math::vec2& size, Color color, float rotation = 0.0f);
		static void DrawQuad(const Math::vec3& position, const Math::vec2& size, Color color, float rotation = 0.0f);
		static void DrawQuad(const Math::vec2& position, const Math::vec2& size, const Math::vec3& color, float rotation = 0.0f);
		static void DrawQuad(const Math::vec2& position, const Math::vec2& size, const Math::vec4& color, float rotation = 0.0f);
		static void DrawQuad(const Math::vec3& position, const Math::vec2& size, const Math::vec3& color, float rotation = 0.0f);
		static void DrawQuad(const Math::vec3& position, const Math::vec2& size, const Math::vec4& color, float rotation = 0.0f);
		static void DrawQuad(const Math::vec2& position, const Math::vec2& size, const SharedRef<Texture>& texture, float rotation = 0.0f, int tileScale = 1, const Math::vec4 & tintColor = Math::vec4(1.0f));
		static void DrawQuad(const Math::vec3& position, const Math::vec2& size, const SharedRef<Texture>& texture, float rotation = 0.0f, int tileScale = 1, const Math::vec4& tintColor = Math::vec4(1.0f));
	};

}
