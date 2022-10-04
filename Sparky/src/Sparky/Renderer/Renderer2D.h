#pragma once

#include "Sparky/Renderer/OrthographicCamera.h"
#include "Sparky/Renderer/Camera.h"
#include "Sparky/Renderer/EditorCamera.h"
#include "Sparky/Renderer/Texture.h"
#include "Sparky/Renderer/SubTexture2D.h"
#include "Sparky/Renderer/Color.h"
#include "Sparky/Core/Math.h"

#include "Sparky/Scene/Components.h"

namespace Sparky
{
	#ifdef SP_DEBUG
		#define SP_RENDERER_STATISTICS 1;
	#elif SP_RELEASE
		#define SP_RENDERER_STATISTICS 1; // Temporary
	#endif // SP_DEBUG

	static constexpr uint32_t VERTICES_PER_QUAD = 4;
	static constexpr uint32_t INDICES_PER_QUAD = 6;

	class Renderer2D
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const Camera& camera, const Math::mat4& transform);
		static void BeginScene(const EditorCamera& camera);
		static void BeginScene(const OrthographicCamera& camera);
		static void EndScene();
		static void Flush();

		// Primitives

		static void DrawQuad(const Math::mat4& transform, const Math::vec3& color);
		static void DrawQuad(const Math::mat4& transform, const Math::vec4& color, int entityID = -1);
		static void DrawQuad(const Math::mat4& transform, Color color = Color::White);

		static void DrawQuad(const Math::mat4& transform, const SharedRef<Texture2D>& texture, float scale, const Math::vec3& tintColor);
		static void DrawQuad(const Math::mat4& transform, const SharedRef<Texture2D>& texture, float scale, const Math::vec4& tintColor, int entityID = -1);
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

		static void DrawSprite(const Math::mat4& transform, SpriteRendererComponent& sprite, int entityID);

		static void DrawCircle(const Math::mat4& transform, const Math::vec4& color, float thickness = 1.0f, float fade = 0.005f, int entityID = -1);

		static void DrawLine(const Math::vec3& start, const Math::vec3& end, const Math::vec4& color, int entityID = -1);

		static void DrawRect(const Math::mat4& transform, const Math::vec4& color = ColorToVec4(Color::LightGreen), int entityID = -1);
		// position is the center, size is the full size of the entity
		static void DrawRect(const Math::vec3& position, const Math::vec2& size, const Math::vec4& color = ColorToVec4(Color::LightGreen), int entityID = -1);

		static float GetLineWidth();
		static void SetLineWidth(float width);

		struct Statistics
		{
			uint32_t DrawCalls;
			uint32_t QuadCount;
			uint32_t LineCount;

			uint32_t GetTriangleCount() const { return QuadCount * 2; }
			uint32_t GetVertexCount() const { return (QuadCount * VERTICES_PER_QUAD) + (LineCount * 2); }
			uint32_t GetIndexCount() const { return QuadCount * INDICES_PER_QUAD; }
		};

		static void ResetStats();
		static Statistics GetStats();

	private:
		static void StartBatch();
		static void NextBatch();

		static void AddToQuadVertexBuffer(const Math::mat4& transform, const Math::vec4& color, const Math::vec2* textureCoords, float textureIndex, float textureScale, int entityID = -1);
		static void AddToCircleVertexBuffer(const Math::mat4& transform, const Math::vec4& color, float thickness, float fade, int entityID = -1);
	};

}
