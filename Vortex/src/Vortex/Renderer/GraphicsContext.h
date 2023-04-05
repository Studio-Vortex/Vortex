#pragma once

#include "Vortex/Core/Base.h"

namespace Vortex {

	class GraphicsContext
	{
	public:
		virtual ~GraphicsContext() = default;

		virtual void Init() = 0;
		virtual void SwapFrameBuffers() = 0;

		static UniqueRef<GraphicsContext> Create(void* window);
	};

}