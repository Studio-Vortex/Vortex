#pragma once

#ifdef SP_PLATFORM_WINDOWS
	#ifdef SP_BUILD_DLL
		#define SPARKY_API __declspec(dllexport)
	#else
		#define SPARKY_API __declspec(dllimport)
	#endif
#else
	#error Sparky only supports Windows!
#endif