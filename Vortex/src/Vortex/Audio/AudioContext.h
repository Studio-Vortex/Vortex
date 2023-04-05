#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/ReferenceCounting/SharedRef.h"

namespace Vortex {

	class AudioContext : public RefCounted
	{
	public:
		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		virtual void* GetLogger() = 0;

		static SharedReference<AudioContext> Create();
	};

}