#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Renderer/RendererAPI.h"
#include "Vortex/Renderer/Camera.h"
#include "Vortex/Renderer/Framebuffer.h"
#include "Vortex/Editor/EditorCamera.h"
#include "Vortex/Renderer/Texture.h"
#include "Vortex/Renderer/SubTexture2D.h"
#include "Vortex/Renderer/Shader.h"
#include "Vortex/Renderer/Color.h"
#include "Vortex/Renderer/Font/Font.h"
#include "Vortex/Core/Math/Math.h"

#include "Vortex/Scene/Components.h"

#include <string>
#include <vector>

namespace Vortex {

	class Renderer2D
	{
	public:
		static void Init(RendererAPI::TriangleCullMode cullMode = RendererAPI::TriangleCullMode::None);
		static void Shutdown();

		static void BeginScene(const Camera& camera, const Math::mat4& transform);
		static void BeginScene(const EditorCamera* camera);
		static void EndScene();
		static void Flush();

		static void RenderLightSource(const TransformComponent& transform, const LightSource2DComponent& lightSourceComponent);

		// Primitives
		static void DrawUnitQuad();

		static void DrawQuad(const Math::mat4& transform, const Math::vec3& color);
		static void DrawQuad(const Math::mat4& transform, const Math::vec4& color, int entityID = -1);
		static void DrawQuad(const Math::mat4& transform, Color color = Color::White);

		static void DrawQuad(const Math::mat4& transform, const SharedReference<Texture2D>& texture, const Math::vec2& scale, const Math::vec3& tintColor);
		static void DrawQuad(const Math::mat4& transform, const SharedReference<Texture2D>& texture, const Math::vec2& scale, const Math::vec4& tintColor, int entityID = -1);
		static void DrawQuad(const Math::mat4& transform, const SharedReference<Texture2D>& texture, const Math::vec2& scale, Color tintColor = Color::White);

		static void DrawQuad(const Math::vec2& position, const Math::vec2& size, const Math::vec3& color);
		static void DrawQuad(const Math::vec2& position, const Math::vec2& size, const Math::vec4& color);
		static void DrawQuad(const Math::vec3& position, const Math::vec2& size, const Math::vec3& color);
		static void DrawQuad(const Math::vec3& position, const Math::vec2& size, const Math::vec4& color, int entityID = -1);
		static void DrawQuad(const Math::vec2& position, const Math::vec2& size, Color color = Color::White);
		static void DrawQuad(const Math::vec3& position, const Math::vec2& size, Color color = Color::White);

		static void DrawQuad(const Math::vec2& position, const Math::vec2& size, const SharedReference<Texture2D>& texture, const Math::vec2& scale, const Math::vec4& tintColor);
		static void DrawQuad(const Math::vec3& position, const Math::vec2& size, const SharedReference<Texture2D>& texture, const Math::vec2& scale, const Math::vec4& tintColor);
		static void DrawQuad(const Math::vec2& position, const Math::vec2& size, const SharedReference<Texture2D>& texture, const Math::vec2& scale, Color tintColor = Color::White);
		static void DrawQuad(const Math::vec3& position, const Math::vec2& size, const SharedReference<Texture2D>& texture, const Math::vec2& scale, Color tintColor = Color::White);

		static void DrawQuad(const Math::vec2& position, const Math::vec2& size, const SharedReference<SubTexture2D>& subtexture, const Math::vec2& scale, const Math::vec3& tintColor);
		static void DrawQuad(const Math::vec2& position, const Math::vec2& size, const SharedReference<SubTexture2D>& subtexture, const Math::vec2& scale, const Math::vec4& tintColor);
		static void DrawQuad(const Math::vec3& position, const Math::vec2& size, const SharedReference<SubTexture2D>& subtexture, const Math::vec2& scale, const Math::vec4& tintColor);
		static void DrawQuad(const Math::vec2& position, const Math::vec2& size, const SharedReference<SubTexture2D>& subtexture, const Math::vec2& scale, Color tintColor = Color::White);
		static void DrawQuad(const Math::vec3& position, const Math::vec2& size, const SharedReference<SubTexture2D>& subtexture, const Math::vec2& scale, Color tintColor = Color::White);

		static void DrawAABB(const Math::AABB& aabb, const Math::mat4& transform, const Math::vec4& color);

		static void DrawQuadBillboard(const Math::mat4& cameraView, const Math::vec3& translation, const Math::vec2& size, const Math::vec4& color);
		static void DrawQuadBillboard(const Math::mat4& cameraView, const Math::vec3& translation, const SharedReference<Texture2D>& texture, const Math::vec2& size, const Math::vec4& color, int entityID = -1);

		// Rotation is in radians
		static void DrawRotatedQuad(const Math::mat4& transform, const Math::vec3& color);
		static void DrawRotatedQuad(const Math::mat4& transform, const Math::vec4& color, int entityID = -1);
		static void DrawRotatedQuad(const Math::mat4& transform, Color color = Color::White);

		static void DrawRotatedQuad(const Math::mat4& transform, const SharedReference<Texture2D>& texture, const Math::vec2& scale, const Math::vec3& tintColor);
		static void DrawRotatedQuad(const Math::mat4& transform, const SharedReference<Texture2D>& texture, const Math::vec2& scale, const Math::vec4& tintColor);
		static void DrawRotatedQuad(const Math::mat4& transform, const SharedReference<Texture2D>& texture, const Math::vec2& scale, Color tintColor = Color::White);

		static void DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const Math::vec3& color, int entityID = -1);
		static void DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const Math::vec4& color, int entityID = -1);
		static void DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const Math::vec3& color, int entityID = -1);
		static void DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const Math::vec4& color, int entityID = -1);

		static void DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, Color color = Color::White);
		static void DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, Color color = Color::White);

		static void DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const SharedReference<Texture2D>& texture, const Math::vec2& scale, const Math::vec3& tintColor);
		static void DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const SharedReference<Texture2D>& texture, const Math::vec2& scale, const Math::vec4& tintColor);
		static void DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const SharedReference<Texture2D>& texture, const Math::vec2& scale, const Math::vec3& tintColor);
		static void DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const SharedReference<Texture2D>& texture, const Math::vec2& scale, const Math::vec4& tintColor);

		static void DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const SharedReference<SubTexture2D>& subtexture, const Math::vec2& scale, const Math::vec3& tintColor);
		static void DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const SharedReference<SubTexture2D>& subtexture, const Math::vec2& scale, const Math::vec4& tintColor);
		static void DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const SharedReference<SubTexture2D>& subtexture, const Math::vec2& scale, const Math::vec3& tintColor);
		static void DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const SharedReference<SubTexture2D>& subtexture, const Math::vec2& scale, const Math::vec4& tintColor);

		static void DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const SharedReference<Texture2D>& texture, const Math::vec2& scale, Color tintColor = Color::White);
		static void DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const SharedReference<Texture2D>& texture, const Math::vec2& scale, Color tintColor = Color::White);

		static void DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const SharedReference<SubTexture2D>& subtexture, const Math::vec2& scale, Color tintColor = Color::White);
		static void DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const SharedReference<SubTexture2D>& subtexture, const Math::vec2& scale, Color tintColor = Color::White);

		static void DrawSprite(const Math::mat4& transform, SpriteRendererComponent& sprite, SharedReference<Texture2D> texture, int entityID = -1);

		static void DrawCircle(const Math::vec2& position, const Math::vec2& size, float rotation, const Math::vec4& color, float thickness, float fade, int entityID = -1);
		static void DrawCircle(const Math::vec3& position, const Math::vec3& size, float rotation, const Math::vec4& color, float thickness, float fade, int entityID = -1);
		static void DrawCircle(const Math::mat4& transform, const Math::vec4& color, float thickness, float fade = 0.005f, int entityID = -1);
		static void DrawCircle(const Math::vec3& position, const Math::vec3& rotation, float radius, const Math::vec4& color);
		static void DrawCircle(const Math::mat4& transform, const Math::vec4& color);

		static void DrawLine(const Math::vec3& start, const Math::vec3& end, const Math::vec4& color, int entityID = -1);

		static void DrawRect(const Math::mat4& transform, const Math::vec4& color = ColorToVec4(Color::LightGreen), int entityID = -1);
		// position is the center, size is the full size of the entity
		static void DrawRect(const Math::vec3& position, const Math::vec2& size, const Math::vec4& color = ColorToVec4(Color::LightGreen), int entityID = -1);

		static void DrawString(const std::string& string, SharedReference<Font>& font, const Math::vec3& position, float maxWidth, const Math::vec4& color = Math::vec4(1.0f), const Math::vec4& bgColor = Math::vec4(0.0f), int entityID = -1);
		static void DrawString(const std::string& string, SharedReference<Font>& font, const Math::mat4& transform, float maxWidth, const Math::vec4& color = Math::vec4(1.0f), const Math::vec4& bgColor = Math::vec4(0.0f), float lineHeightOffset = 0.0f, float kerningOffset = 0.0f, int entityID = -1);

		static float GetLineWidth();
		static void SetLineWidth(float width);

		static RendererAPI::TriangleCullMode GetCullMode();
		static void SetCullMode(RendererAPI::TriangleCullMode cullMode);

		static RenderStatistics GetStats();
		static void ResetStats();

		static SharedReference<Texture2D> GetWhiteTexture();

		static const ShaderLibrary& GetShaderLibrary();

	private:
		static void SetShaderViewProjectionMatrix(const Math::mat4& viewProjection);
		static void StartBatch();
		static void NextBatch();

		static void AddToQuadVertexBuffer(const Math::mat4& transform, const Math::vec4& color, const Math::vec2* textureCoords, uint32_t textureIndex, const Math::vec2& textureScale, int entityID = -1);
		static void AddToCircleVertexBuffer(const Math::mat4& transform, const Math::vec4& color, float thickness, float fade, int entityID = -1);
	};

}
