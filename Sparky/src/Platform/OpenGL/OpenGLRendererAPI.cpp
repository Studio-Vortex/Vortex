#include "sppch.h"
#include "OpenGLRendererAPI.h"

#include <Glad/glad.h>

namespace Sparky {

    void OpenGLRendererAPI::Init()
    {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void OpenGLRendererAPI::SetClearColor(const Math::vec3& color)
	{
		glClearColor(color.r, color.g, color.b, 1.0f);
	}

	void OpenGLRendererAPI::Clear() const
	{
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void OpenGLRendererAPI::DrawIndexed(const SharedRef<VertexArray>& vertexArray) const
	{
		glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
	}

}