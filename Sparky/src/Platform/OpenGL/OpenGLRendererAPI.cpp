#include "sppch.h"
#include "OpenGLRendererAPI.h"

#include <Glad/glad.h>

namespace Sparky {

    void OpenGLRendererAPI::Init() const
    {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEPTH_TEST);
    }

    void OpenGLRendererAPI::SetViewport(const Viewport& viewport) const
    {
		glViewport(viewport.XPos, viewport.YPos, viewport.Width, viewport.Height);
    }

    void OpenGLRendererAPI::SetClearColor(const Math::vec3& color)
	{
		glClearColor(color.r, color.g, color.b, 1.0f);
	}

	void OpenGLRendererAPI::Clear() const
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRendererAPI::DrawIndexed(const SharedRef<VertexArray>& vertexArray) const
	{
		glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
	}

}