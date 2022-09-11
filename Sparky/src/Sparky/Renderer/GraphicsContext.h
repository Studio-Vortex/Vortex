#pragma once

#include "Sparky/Core/Base.h"

namespace Sparky {

	class GraphicsContext
	{
	public:
		virtual void Init() = 0;
		virtual void SwapFrameBuffers() = 0;

		static UniqueRef<GraphicsContext> Create(void* window);
	};

}