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
		static void Flush();

		// Primitives

		static void DrawQuad(const Math::vec2& position, const Math::vec2& size, const Math::vec3& color);
		static void DrawQuad(const Math::vec2& position, const Math::vec2& size, const Math::vec4& color);
		static void DrawQuad(const Math::vec3& position, const Math::vec2& size, const Math::vec3& color);
		static void DrawQuad(const Math::vec3& position, const Math::vec2& size, const Math::vec4& color);
		static void DrawQuad(const Math::vec2& position, const Math::vec2& size, Color color = Color::White);
		static void DrawQuad(const Math::vec3& position, const Math::vec2& size, Color color = Color::White);

		static void DrawQuad(const Math::vec2& position, const Math::vec2& size, uint32_t scale = 1, const SharedRef<Texture>& texture = nullptr, const Math::vec4& color = Math::vec4(1.0f));
		static void DrawQuad(const Math::vec3& position, const Math::vec2& size, uint32_t scale = 1, const SharedRef<Texture>& texture = nullptr, const Math::vec4& color = Math::vec4(1.0f));
		static void DrawQuad(const Math::vec2& position, const Math::vec2& size, uint32_t scale = 1, const SharedRef<Texture>& texture = nullptr, Color color = Color::White);
		static void DrawQuad(const Math::vec3& position, const Math::vec2& size, uint32_t scale = 1, const SharedRef<Texture>& texture = nullptr, Color color = Color::White);

		static void DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const Math::vec3& color);
		static void DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const Math::vec4& color);
		static void DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const Math::vec3& color);
		static void DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const Math::vec4& color);

		static void DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, Color color = Color::White);
		static void DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, Color color = Color::White);

		static void DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, uint32_t scale = 1, const SharedRef<Texture>&texture = nullptr, const Math::vec3 & color = Math::vec3(1.0f));
		static void DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, uint32_t scale = 1, const SharedRef<Texture>&texture = nullptr, const Math::vec4 & color = Math::vec4(1.0f));
		static void DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, uint32_t scale = 1, const SharedRef<Texture>&texture = nullptr, const Math::vec3 & color = Math::vec3(1.0f));
		static void DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, uint32_t scale = 1, const SharedRef<Texture>& texture = nullptr, const Math::vec4& color = Math::vec4(1.0f));

		static void DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, uint32_t scale = 1, const SharedRef<Texture>&texture = nullptr, Color color = Color::White);
		static void DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, uint32_t scale = 1, const SharedRef<Texture>& texture = nullptr, Color color = Color::White);
	};

}
