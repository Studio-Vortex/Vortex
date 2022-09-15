#include "sppch.h"
#include "RendererAPI.h"

namespace Sparky {

	RendererAPI::API RendererAPI::s_API = RendererAPI::API::OpenGL;

	RendererAPI::RendererInfo RendererAPI::s_RendererInfo;

}