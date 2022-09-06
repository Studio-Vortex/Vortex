#pragma once

#ifdef SP_PLATFORM_WINDOWS
#if SP_DYNAMIC_LINK
	#ifdef SP_BUILD_DLL
		#define SPARKY_API __declspec(dllexport)
	#else
		#define SPARKY_API __declspec(dllimport)
	#endif // SP_BUILD_DLL
#else
	#define SPARKY_API
#endif // SP_DYNAMIC_LINK
#else
	#error Sparky only supports Windows!
#endif // SP_PLATFORM_WINDOWS

#ifdef SP_DEBUG
	#define SP_ENABLE_ASSERTS
#endif // SP_DEBUG

#ifdef SP_ENABLE_ASSERTS
	#define SP_ASSERT(x, ...) { if(!(x)) { SP_ERROR("Assertion Failed: {}", __VA_ARGS__); __debugbreak(); } }
	#define SP_CORE_ASSERT(x, ...) { if(!(x)) { SP_CORE_ERROR("Assertion Failed: {}", __VA_ARGS__); __debugbreak(); } }
#else
	#define SP_ASSERT(x, ...)
	#define SP_CORE_ASSERT(x, ...)
#endif // SP_ENABLE_ASSERTS

#define SP_BIND_CALLBACK(ident) std::bind(&ident, this, std::placeholders::_1)

#define BIT(x) (1 << x)