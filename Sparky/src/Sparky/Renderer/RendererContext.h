#pragma once

namespace Sparky {

	class RendererContext
	{
	public:
		virtual void Init() = 0;
		virtual void SwapFrameBuffers() = 0;
	};

}