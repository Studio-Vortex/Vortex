#include "sppch.h"
#include "OpenGLRendererAPI.h"

#include <Glad/glad.h>

namespace Sparky {

    void OpenGLRendererAPI::Init() const
    {
		SP_PROFILE_FUNCTION();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEPTH_TEST);
    }

    void OpenGLRendererAPI::SetViewport(const Viewport& viewport) const
    {
		glViewport(viewport.XPos, viewport.YPos, viewport.Width, viewport.Height);
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
		uint32_t count = indexCount ? vertexArray->GetIndexBuffer()->GetCount() : indexCount;
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
	}

}