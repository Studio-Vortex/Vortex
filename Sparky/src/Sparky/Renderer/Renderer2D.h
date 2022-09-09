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

		static void DrawQuad(const Math::vec2& position, const Math::vec2& size, const Math::vec3& color = Math::vec3(1.0f));
		static void DrawQuad(const Math::vec2& position, const Math::vec2& size, const Math::vec4& color = Math::vec4(1.0f));
		static void DrawQuad(const Math::vec3& position, const Math::vec2& size, const Math::vec3& color = Math::vec3(1.0f));
		static void DrawQuad(const Math::vec3& position, const Math::vec2& size, const Math::vec4& color = Math::vec4(1.0f));
		static void DrawQuad(const Math::vec2& position, const Math::vec2& size, Color color = Color::White);
		static void DrawQuad(const Math::vec3& position, const Math::vec2& size, Color color = Color::White);

		static void DrawQuad(const Math::vec2& position, const Math::vec2& size, const SharedRef<Texture>& texture, const Math::vec4& color = Math::vec4(1.0f), uint32_t scale = 1);
		static void DrawQuad(const Math::vec3& position, const Math::vec2& size, const SharedRef<Texture>& texture, const Math::vec4& color = Math::vec4(1.0f), uint32_t scale = 1);
		static void DrawQuad(const Math::vec2& position, const Math::vec2& size, const SharedRef<Texture>& texture, Color color = Color::White, uint32_t scale = 1);
		static void DrawQuad(const Math::vec3& position, const Math::vec2& size, const SharedRef<Texture>& texture, Color color = Color::White, uint32_t scale = 1);
	};

}
