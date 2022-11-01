#pragma once

#include "Sparky/Core/Base.h"
#include "Sparky/Renderer/RendererAPI.h"
#include "Sparky/Renderer/OrthographicCamera.h"
#include "Sparky/Renderer/Camera.h"
#include "Sparky/Renderer/EditorCamera.h"
#include "Sparky/Renderer/Texture.h"
#include "Sparky/Renderer/SubTexture2D.h"
#include "Sparky/Renderer/Shader.h"
#include "Sparky/Renderer/Color.h"
#include "Sparky/Core/Math.h"

#include "Sparky/Scene/Components.h"

#include <vector>

namespace Sparky {

	class Renderer2D
	{
	public:
		static void Init(RendererAPI::TriangleCullMode cullMode = RendererAPI::TriangleCullMode::None);
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

		static void DrawQuad(const Math::mat4& transform, const SharedRef<Texture2D>& texture, const Math::vec2& scale, const Math::vec3& tintColor);
		static void DrawQuad(const Math::mat4& transform, const SharedRef<Texture2D>& texture, const Math::vec2& scale, const Math::vec4& tintColor, int entityID = -1);
		static void DrawQuad(const Math::mat4& transform, const SharedRef<Texture2D>& texture, const Math::vec2& scale, Color tintColor = Color::White);

		static void DrawQuad(const Math::vec2& position, const Math::vec2& size, const Math::vec3& color);
		static void DrawQuad(const Math::vec2& position, const Math::vec2& size, const Math::vec4& color);
		static void DrawQuad(const Math::vec3& position, const Math::vec2& size, const Math::vec3& color);
		static void DrawQuad(const Math::vec3& position, const Math::vec2& size, const Math::vec4& color, int entityID = -1);
		static void DrawQuad(const Math::vec2& position, const Math::vec2& size, Color color = Color::White);
		static void DrawQuad(const Math::vec3& position, const Math::vec2& size, Color color = Color::White);

		static void DrawQuad(const Math::vec2& position, const Math::vec2& size, const SharedRef<Texture2D>& texture, const Math::vec2& scale, const Math::vec4& tintColor);
		static void DrawQuad(const Math::vec3& position, const Math::vec2& size, const SharedRef<Texture2D>& texture, const Math::vec2& scale, const Math::vec4& tintColor);
		static void DrawQuad(const Math::vec2& position, const Math::vec2& size, const SharedRef<Texture2D>& texture, const Math::vec2& scale, Color tintColor = Color::White);
		static void DrawQuad(const Math::vec3& position, const Math::vec2& size, const SharedRef<Texture2D>& texture, const Math::vec2& scale, Color tintColor = Color::White);

		static void DrawQuad(const Math::vec2& position, const Math::vec2& size, const SharedRef<SubTexture2D>& subtexture, const Math::vec2& scale, const Math::vec3& tintColor);
		static void DrawQuad(const Math::vec2& position, const Math::vec2& size, const SharedRef<SubTexture2D>& subtexture, const Math::vec2& scale, const Math::vec4& tintColor);
		static void DrawQuad(const Math::vec3& position, const Math::vec2& size, const SharedRef<SubTexture2D>& subtexture, const Math::vec2& scale, const Math::vec4& tintColor);
		static void DrawQuad(const Math::vec2& position, const Math::vec2& size, const SharedRef<SubTexture2D>& subtexture, const Math::vec2& scale, Color tintColor = Color::White);
		static void DrawQuad(const Math::vec3& position, const Math::vec2& size, const SharedRef<SubTexture2D>& subtexture, const Math::vec2& scale, Color tintColor = Color::White);

		// Rotation is in radians
		static void DrawRotatedQuad(const Math::mat4& transform, const Math::vec3& color);
		static void DrawRotatedQuad(const Math::mat4& transform, const Math::vec4& color, int entityID = -1);
		static void DrawRotatedQuad(const Math::mat4& transform, Color color = Color::White);

		static void DrawRotatedQuad(const Math::mat4& transform, const SharedRef<Texture2D>& texture, const Math::vec2& scale, const Math::vec3& tintColor);
		static void DrawRotatedQuad(const Math::mat4& transform, const SharedRef<Texture2D>& texture, const Math::vec2& scale, const Math::vec4& tintColor);
		static void DrawRotatedQuad(const Math::mat4& transform, const SharedRef<Texture2D>& texture, const Math::vec2& scale, Color tintColor = Color::White);

		static void DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const Math::vec3& color, int entityID = -1);
		static void DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const Math::vec4& color, int entityID = -1);
		static void DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const Math::vec3& color, int entityID = -1);
		static void DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const Math::vec4& color, int entityID = -1);

		static void DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, Color color = Color::White);
		static void DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, Color color = Color::White);

		static void DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const SharedRef<Texture2D>& texture, const Math::vec2& scale, const Math::vec3& tintColor);
		static void DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const SharedRef<Texture2D>& texture, const Math::vec2& scale, const Math::vec4& tintColor);
		static void DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const SharedRef<Texture2D>& texture, const Math::vec2& scale, const Math::vec3& tintColor);
		static void DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const SharedRef<Texture2D>& texture, const Math::vec2& scale, const Math::vec4& tintColor);

		static void DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const SharedRef<SubTexture2D>& subtexture, const Math::vec2& scale, const Math::vec3& tintColor);
		static void DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const SharedRef<SubTexture2D>& subtexture, const Math::vec2& scale, const Math::vec4& tintColor);
		static void DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const SharedRef<SubTexture2D>& subtexture, const Math::vec2& scale, const Math::vec3& tintColor);
		static void DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const SharedRef<SubTexture2D>& subtexture, const Math::vec2& scale, const Math::vec4& tintColor);

		static void DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const SharedRef<Texture2D>& texture, const Math::vec2& scale, Color tintColor = Color::White);
		static void DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const SharedRef<Texture2D>& texture, const Math::vec2& scale, Color tintColor = Color::White);

		static void DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const SharedRef<SubTexture2D>& subtexture, const Math::vec2& scale, Color tintColor = Color::White);
		static void DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const SharedRef<SubTexture2D>& subtexture, const Math::vec2& scale, Color tintColor = Color::White);

		static void DrawSprite(const Math::mat4& transform, SpriteRendererComponent& sprite, int entityID = -1);

		static void DrawCircle(const Math::mat4& transform, const Math::vec4& color, float thickness = 1.0f, float fade = 0.005f, int entityID = -1);

		static void DrawLine(const Math::vec3& start, const Math::vec3& end, const Math::vec4& color, int entityID = -1);

		static void DrawRect(const Math::mat4& transform, const Math::vec4& color = ColorToVec4(Color::LightGreen), int entityID = -1);
		// position is the center, size is the full size of the entity
		static void DrawRect(const Math::vec3& position, const Math::vec2& size, const Math::vec4& color = ColorToVec4(Color::LightGreen), int entityID = -1);

		static float GetLineWidth();
		static void SetLineWidth(float width);

		static RendererAPI::TriangleCullMode GetCullMode();
		static void SetCullMode(RendererAPI::TriangleCullMode cullMode);

		static RenderStatistics GetStats();
		static void ResetStats();

		static std::vector<SharedRef<Shader>> GetLoadedShaders();

	private:
		static void StartBatch();
		static void NextBatch();

		static void AddToQuadVertexBuffer(const Math::mat4& transform, const Math::vec4& color, const Math::vec2* textureCoords, float textureIndex, const Math::vec2& textureScale, int entityID = -1);
		static void AddToCircleVertexBuffer(const Math::mat4& transform, const Math::vec4& color, float thickness, float fade, int entityID = -1);
	};

}
