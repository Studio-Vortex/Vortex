#pragma once

#ifdef SP_PLATFORM_WINDOWS
	#ifdef SP_BUILD_DLL
		#define SPARKY_API __declspec(dllexport)
	#else
		#define SPARKY_API __declspec(dllimport)
	#endif // SP_BUILD_DLL
#else
	#error Sparky only supports Windows!
#endif // SP_PLATFORM_WINDOWS

#define BIT(x) (1 << x)