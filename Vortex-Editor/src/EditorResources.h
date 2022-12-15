#pragma once

#include <Vortex.h>

namespace Vortex {

	class EditorResources
	{
	public:
		static void Init();

		inline static SharedRef<Texture2D> PlayIcon = nullptr;
		inline static SharedRef<Texture2D> PauseIcon = nullptr;
		inline static SharedRef<Texture2D> StopIcon = nullptr;
		inline static SharedRef<Texture2D> SimulateIcon = nullptr;
		inline static SharedRef<Texture2D> StepIcon = nullptr;

		inline static SharedRef<Texture2D> LocalModeIcon = nullptr;
		inline static SharedRef<Texture2D> WorldModeIcon = nullptr;

		inline static SharedRef<Texture2D> SelectToolIcon = nullptr;
		inline static SharedRef<Texture2D> TranslateToolIcon = nullptr;
		inline static SharedRef<Texture2D> RotateToolIcon = nullptr;
		inline static SharedRef<Texture2D> ScaleToolIcon = nullptr;

		inline static SharedRef<Texture2D> ShowGridIcon = nullptr;
		inline static SharedRef<Texture2D> MaximizeOnPlayIcon = nullptr;
		inline static SharedRef<Texture2D> TopDownViewIcon = nullptr;
		inline static SharedRef<Texture2D> TwoDViewIcon = nullptr;
		inline static SharedRef<Texture2D> DisplayPhysicsCollidersIcon = nullptr;
		inline static SharedRef<Texture2D> DisplaySceneIconsIcon = nullptr;
	};

}
