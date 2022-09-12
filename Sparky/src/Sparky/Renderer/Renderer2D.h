#pragma once

#include "Sparky/Renderer/OrthographicCamera.h"
#include "Sparky/Renderer/Texture.h"
#include "Sparky/Renderer/SubTexture2D.h"
#include "Sparky/Renderer/Color.h"
#include "Sparky/Core/Math.h"

namespace Sparky
{
	#ifdef SP_DEBUG
		#define SP_RENDERER_STATISTICS 1;
	#endif // SP_DEBUG

	static constexpr uint32_t VERTICES_PER_QUAD = 4;
	static constexpr uint32_t INDICES_PER_QUAD = 6;

	class Renderer2D
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const OrthographicCamera& camera);
		static void EndScene();
		static void Flush();

		// Primitives

		static void DrawQuad(const Math::mat4& transform, const Math::vec3& color);
		static void DrawQuad(const Math::mat4& transform, const Math::vec4& color);
		static void DrawQuad(const Math::mat4& transform, Color color = Color::White);

		static void DrawQuad(const Math::mat4& transform, const SharedRef<Texture2D>& texture, float scale, const Math::vec3& tintColor);
		static void DrawQuad(const Math::mat4& transform, const SharedRef<Texture2D>& texture, float scale, const Math::vec4& tintColor);
		static void DrawQuad(const Math::mat4& transform, const SharedRef<Texture2D>& texture, float scale, Color tintColor = Color::White);

		static void DrawQuad(const Math::vec2& position, const Math::vec2& size, const Math::vec3& color);
		static void DrawQuad(const Math::vec2& position, const Math::vec2& size, const Math::vec4& color);
		static void DrawQuad(const Math::vec3& position, const Math::vec2& size, const Math::vec3& color);
		static void DrawQuad(const Math::vec3& position, const Math::vec2& size, const Math::vec4& color);
		static void DrawQuad(const Math::vec2& position, const Math::vec2& size, Color color = Color::White);
		static void DrawQuad(const Math::vec3& position, const Math::vec2& size, Color color = Color::White);

		static void DrawQuad(const Math::vec2& position, const Math::vec2& size, const SharedRef<Texture2D>& texture, float scale, const Math::vec4& tintColor);
		static void DrawQuad(const Math::vec3& position, const Math::vec2& size, const SharedRef<Texture2D>& texture, float scale, const Math::vec4& tintColor);
		static void DrawQuad(const Math::vec2& position, const Math::vec2& size, const SharedRef<Texture2D>& texture, float scale, Color tintColor = Color::White);
		static void DrawQuad(const Math::vec3& position, const Math::vec2& size, const SharedRef<Texture2D>& texture, float scale, Color tintColor = Color::White);

		static void DrawQuad(const Math::vec2& position, const Math::vec2& size, const SharedRef<SubTexture2D>& subtexture, float scale, const Math::vec3& tintColor);
		static void DrawQuad(const Math::vec2& position, const Math::vec2& size, const SharedRef<SubTexture2D>& subtexture, float scale, const Math::vec4& tintColor);
		static void DrawQuad(const Math::vec3& position, const Math::vec2& size, const SharedRef<SubTexture2D>& subtexture, float scale, const Math::vec4& tintColor);
		static void DrawQuad(const Math::vec2& position, const Math::vec2& size, const SharedRef<SubTexture2D>& subtexture, float scale, Color tintColor = Color::White);
		static void DrawQuad(const Math::vec3& position, const Math::vec2& size, const SharedRef<SubTexture2D>& subtexture, float scale, Color tintColor = Color::White);

		// Rotation is in radians
		static void DrawRotatedQuad(const Math::mat4& transform, const Math::vec3& color);
		static void DrawRotatedQuad(const Math::mat4& transform, const Math::vec4& color);
		static void DrawRotatedQuad(const Math::mat4& transform, Color color = Color::White);

		static void DrawRotatedQuad(const Math::mat4& transform, const SharedRef<Texture2D>& texture, float scale, const Math::vec3& tintColor);
		static void DrawRotatedQuad(const Math::mat4& transform, const SharedRef<Texture2D>& texture, float scale, const Math::vec4& tintColor);
		static void DrawRotatedQuad(const Math::mat4& transform, const SharedRef<Texture2D>& texture, float scale, Color tintColor = Color::White);

		static void DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const Math::vec3& color);
		static void DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const Math::vec4& color);
		static void DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const Math::vec3& color);
		static void DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const Math::vec4& color);

		static void DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, Color color = Color::White);
		static void DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, Color color = Color::White);

		static void DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const SharedRef<Texture2D>& texture, float scale, const Math::vec3& tintColor);
		static void DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const SharedRef<Texture2D>& texture, float scale, const Math::vec4& tintColor);
		static void DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const SharedRef<Texture2D>& texture, float scale, const Math::vec3& tintColor);
		static void DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const SharedRef<Texture2D>& texture, float scale, const Math::vec4& tintColor);

		static void DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const SharedRef<SubTexture2D>& subtexture, float scale, const Math::vec3& tintColor);
		static void DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const SharedRef<SubTexture2D>& subtexture, float scale, const Math::vec4& tintColor);
		static void DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const SharedRef<SubTexture2D>& subtexture, float scale, const Math::vec3& tintColor);
		static void DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const SharedRef<SubTexture2D>& subtexture, float scale, const Math::vec4& tintColor);

		static void DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const SharedRef<Texture2D>& texture, float scale, Color tintColor = Color::White);
		static void DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const SharedRef<Texture2D>& texture, float scale, Color tintColor = Color::White);

		static void DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const SharedRef<SubTexture2D>& subtexture, float scale, Color tintColor = Color::White);
		static void DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const SharedRef<SubTexture2D>& subtexture, float scale, Color tintColor = Color::White);

		struct Statistics
		{
			uint32_t DrawCalls;
			uint32_t QuadCount;

			uint32_t GetTriangleCount() const { return QuadCount * 2; }
			uint32_t GetVertexCount() const { return QuadCount * VERTICES_PER_QUAD; }
			uint32_t GetIndexCount() const { return QuadCount * INDICES_PER_QUAD; }
		};

		static void ResetStats();
		static Statistics GetStats();

	private:
		static void StartNewBatch();
		static void CopyDataToVertexBuffer();
		static void AddToVertexBuffer(const Math::mat4& transform, const Math::vec4& color, const Math::vec2* textureCoords, float textureIndex, float textureScale);
	};

}
