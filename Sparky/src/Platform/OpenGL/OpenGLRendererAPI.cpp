#include "sppch.h"
#include "OpenGLRendererAPI.h"

#include <Glad/glad.h>

namespace Sparky {

	static void OpenGLMessageCallback(
		unsigned source,
		unsigned type,
		unsigned id,
		unsigned severity,
		int length,
		const char* message,
		const void* userParam
	) {
		switch (severity)
		{
			case GL_DEBUG_SEVERITY_HIGH:         SP_CORE_CRITICAL(message); return;
			case GL_DEBUG_SEVERITY_MEDIUM:       SP_CORE_ERROR(message); return;
			case GL_DEBUG_SEVERITY_LOW:          SP_CORE_WARN(message); return;
			case GL_DEBUG_SEVERITY_NOTIFICATION: SP_CORE_TRACE(message); return;
		}

		SP_CORE_ASSERT(false, "Unknown severity level!");
	}

    void OpenGLRendererAPI::Init() const
    {
		SP_PROFILE_FUNCTION();

#ifdef SP_DEBUG
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(OpenGLMessageCallback, nullptr);

		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, NULL, NULL, GL_FALSE);
#endif

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEPTH_TEST);
		//glEnable(GL_LINE_SMOOTH);

		glEnable(GL_CULL_FACE);
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
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

	void OpenGLRendererAPI::SetLineWidth(float thickness) const
	{
		glLineWidth(thickness);
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
