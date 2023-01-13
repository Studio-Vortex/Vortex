#include "vxpch.h"
#include "OpenGLRendererAPI.h"

#include <Glad/glad.h>

namespace Vortex {

	namespace Utils {

		static GLenum ToGLStencilOperation(RendererAPI::StencilOperation stencilOperation)
		{
			switch (stencilOperation)
			{
				case Vortex::RendererAPI::StencilOperation::None:     return GL_NONE;     break;
				case Vortex::RendererAPI::StencilOperation::Keep:     return GL_KEEP;     break;
				case Vortex::RendererAPI::StencilOperation::Always:   return GL_ALWAYS;   break;
				case Vortex::RendererAPI::StencilOperation::NotEqual: return GL_NOTEQUAL; break;
				case Vortex::RendererAPI::StencilOperation::Replace:  return GL_REPLACE;  break;
			}
		}

		static void OpenGLMessageCallback(
			uint32_t source,
			uint32_t type,
			uint32_t id,
			uint32_t severity,
			int length,
			const char* message,
			const void* userParam
		) {
			switch (severity)
			{
				case GL_DEBUG_SEVERITY_HIGH:         VX_CORE_CRITICAL(message); return;
				case GL_DEBUG_SEVERITY_MEDIUM:       VX_CORE_ERROR(message); return;
				case GL_DEBUG_SEVERITY_LOW:          VX_CORE_WARN(message); return;
				case GL_DEBUG_SEVERITY_NOTIFICATION: VX_CORE_TRACE(message); return;
			}

			VX_CORE_ASSERT(false, "Unknown severity level!");
		}

	}

    void OpenGLRendererAPI::Init() const
    {
		VX_PROFILE_FUNCTION();

#ifdef VX_DEBUG
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(Utils::OpenGLMessageCallback, nullptr);

		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, NULL, NULL, GL_FALSE);
#endif

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEPTH_TEST);
		//glEnable(GL_LINE_SMOOTH);
    }

    void OpenGLRendererAPI::SetViewport(const Viewport& viewport) const
    {
		glViewport(viewport.TopLeftXPos, viewport.TopLeftYPos, viewport.Width, viewport.Height);
    }

    void OpenGLRendererAPI::SetClearColor(const Math::vec3& color) const
	{
		glClearColor(color.r, color.g, color.b, 1.0f);
	}

    void OpenGLRendererAPI::SetWireframeMode(bool enabled) const
    {
		if (enabled)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

	void OpenGLRendererAPI::Clear() const
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}

	void OpenGLRendererAPI::DrawTriangles(const SharedRef<VertexArray>& vertexArray, uint32_t vertexCount) const
	{
		vertexArray->Bind();
		glDrawArrays(GL_TRIANGLES, 0, vertexCount);
	}

	void OpenGLRendererAPI::DrawIndexed(const SharedRef<VertexArray>& vertexArray, uint32_t indexCount) const
	{
		vertexArray->Bind();
		uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
	}
	
	void OpenGLRendererAPI::DrawLines(const SharedRef<VertexArray>& vertexArray, uint32_t vertexCount) const
	{
		vertexArray->Bind();
		glDrawArrays(GL_LINES, 0, vertexCount);
	}

	void OpenGLRendererAPI::DrawTriangleStrip(const SharedRef<VertexArray>& vertexArray, uint32_t indexCount) const
	{
		vertexArray->Bind();
		glDrawArrays(GL_TRIANGLE_STRIP, 0, indexCount);
	}

    void OpenGLRendererAPI::EnableDepthTest() const
    {
		glEnable(GL_DEPTH_TEST);
    }

    void OpenGLRendererAPI::DisableDepthTest() const
    {
		glDisable(GL_DEPTH_TEST);
    }

	void OpenGLRendererAPI::EnableStencilTest() const
	{
		glEnable(GL_STENCIL_TEST);
	}

	void OpenGLRendererAPI::DisableStencilTest() const
	{
		glDisable(GL_STENCIL_TEST);
	}

	void OpenGLRendererAPI::EnableDepthMask() const
	{
		glDepthFunc(GL_LESS);
		glDepthMask(GL_TRUE);
	}

	void OpenGLRendererAPI::DisableDepthMask() const
	{
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_LEQUAL);
	}

	void OpenGLRendererAPI::SetBlendMode(BlendMode blendMode) const
	{
		VX_CORE_ASSERT(blendMode != BlendMode::None, "Invalid Blend Mode!");

		int sFactor = 0, dFactor = 0;

		switch (blendMode)
		{
			case RendererAPI::BlendMode::SrcAlphaOneMinusSrcAlpha:
			{
				sFactor = GL_SRC_ALPHA;
				dFactor = GL_ONE_MINUS_SRC_ALPHA;
				break;
			}
			case RendererAPI::BlendMode::ZeroSrcColor:
			{
				sFactor = GL_SRC_COLOR;
				dFactor = GL_ZERO;
				break;
			}
		}

		glBlendFunc(sFactor, dFactor);
	}

	void OpenGLRendererAPI::SetLineWidth(float thickness) const
	{
		glLineWidth(thickness);
    }

    void OpenGLRendererAPI::SetStencilOperation(StencilOperation failOperation, StencilOperation zFailOperation, StencilOperation passOperation) const
    {
		glStencilOp(Utils::ToGLStencilOperation(failOperation), Utils::ToGLStencilOperation(zFailOperation), Utils::ToGLStencilOperation(passOperation));
    }

	void OpenGLRendererAPI::SetStencilFunc(StencilOperation func, int ref, int mask) const
	{
		glStencilFunc(Utils::ToGLStencilOperation(func), ref, mask);
	}

    void OpenGLRendererAPI::SetStencilMask(int mask) const
    {
		glStencilMask(mask);
    }

    void OpenGLRendererAPI::SetCullMode(TriangleCullMode cullMode) const
	{
		GLenum mode{};

		if (cullMode != TriangleCullMode::None)
			glEnable(GL_CULL_FACE);

		switch (cullMode)
		{
			case TriangleCullMode::None:         glDisable(GL_CULL_FACE);  break;
			case TriangleCullMode::Front:        mode = GL_FRONT;          break;
			case TriangleCullMode::Back:         mode = GL_BACK;           break;
			case TriangleCullMode::FrontAndBack: mode = GL_FRONT_AND_BACK; break;
		}

		if (cullMode != TriangleCullMode::None)
			glCullFace(mode);
	}

}
