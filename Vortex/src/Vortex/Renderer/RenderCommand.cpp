#include "vxpch.h"
#include "RenderCommand.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace Vortex {

	RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI();

}