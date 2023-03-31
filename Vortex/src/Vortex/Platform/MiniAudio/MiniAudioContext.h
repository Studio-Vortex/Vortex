#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Audio/AudioContext.h"

namespace Vortex {

	class MiniAudioContext : public AudioContext
	{
	public:
		virtual void Init() override;
		virtual void Shutdown() override;

		virtual void* GetLogger() override;

	private:
		void EnumerateDevices();
	};

}