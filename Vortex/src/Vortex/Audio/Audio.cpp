#include "vxpch.h"
#include "Audio.h"

#include "Vortex/Core/Application.h"

#include "Vortex/Audio/AudioUtils.h"
#include "Vortex/Audio/AudioLogger.h"

#include "Vortex/Module/Module.h"

#include <Wave/Wave.h>

namespace Vortex {

	struct AudioInternalData
	{
		SharedRef<Wave::Context> Context;
		SubModule Module;
	};

	static AudioInternalData* s_Data = nullptr;

	void Audio::Init()
	{
		s_Data = new AudioInternalData();
		s_Data->Context = Wave::CreateContext();

		Wave::ContextSettings settings;
#ifdef VX_DEBUG
		settings.EnumerateDevices = true;
		settings.EnableDebugLogging = true;
		settings.LogCallback = AudioLogger::DebugLogCallback;
#endif
		settings.pUserData = (void*)s_Data->Context.get();
		
		Wave::ContextResult result = s_Data->Context->Init(settings);
		VX_CORE_ASSERT(result.Success, "Failed to initialize Wave Audio!");

		if (settings.EnumerateDevices)
		{
			VX_CONSOLE_LOG_INFO("[Audio] Enumerating Devices:");

			uint32_t i = 1;
			for (const auto& deviceInfo : result.PlaybackDeviceInfos) {
				VX_CONSOLE_LOG_INFO(" {}  Device-Name: {}", i++, deviceInfo.Name);
				VX_CONSOLE_LOG_INFO("    Devive Type: {}", Utils::WaveDeviceTypeToString(deviceInfo.Type));
				VX_CONSOLE_LOG_INFO("    Is Default: {}", deviceInfo.IsDefault ? "true" : "false");
			}

			i = 1;
			for (const auto& deviceInfo : result.CaptureDeviceInfos) {
				VX_CONSOLE_LOG_INFO(" {}  Device-Name: {}", i++, deviceInfo.Name);
				VX_CONSOLE_LOG_INFO("    Devive Type: {}", Utils::WaveDeviceTypeToString(deviceInfo.Type));
				VX_CONSOLE_LOG_INFO("    Is Default: {}", deviceInfo.IsDefault ? "true" : "false");
			}
		}

#ifdef VX_DEBUG
		VX_CONSOLE_LOG_INFO("[Audio] Audio submodule initialized");
#endif // VX_DEBUG

		SubModuleProperties moduleProps;
		moduleProps.ModuleName = "Audio";
		moduleProps.APIVersion = Version(1, 3, 0);
		moduleProps.RequiredModules = {};
		s_Data->Module.Init(moduleProps);

		Application::Get().AddModule(s_Data->Module);
	}

	void Audio::Shutdown()
	{
		const bool success = s_Data->Context->Shutdown();
		VX_CORE_ASSERT(success, "Failed to shutdown Wave Audio!");
		Wave::DestroyContext(s_Data->Context);

		Application::Get().RemoveModule(s_Data->Module);
		s_Data->Module.Shutdown();

		delete s_Data;
		s_Data = nullptr;
	}

	SharedRef<Wave::Context> Audio::GetContext()
	{
		VX_CORE_ASSERT(s_Data->Context != nullptr, "Audio submodule wasn't initialized!");
		return s_Data->Context;
	}

}
