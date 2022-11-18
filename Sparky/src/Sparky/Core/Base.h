#pragma once

#include "Sparky/Core/PlatformDetection.h"

#include <memory>

// DLL support
#ifdef SP_PLATFORM_WINDOWS
	#if SP_DYNAMIC_LINK
		#ifdef SP_BUILD_DLL
			#define SPARKY_API __declspec(dllexport)
		#else
			#define SPARKY_API __declspec(dllimport)
		#endif
	#else
		#define SPARKY_API
	#endif
#else
	#error "Sparky only supports Windows!"
#endif // End of DLL support

#ifdef SP_DEBUG
	#if defined(SP_PLATFORM_WINDOWS)
		#define SP_DEBUGBREAK() __debugbreak()
	#elif defined(SP_PLATFORM_LINUX)
		#include <signal.h>
		#define SP_DEBUGBREAK() raise(SIGTRAP)
	#else
		#error "Platform doesn't support debugbreak!"
	#endif
	#define SP_ENABLE_ASSERTS
#else
	#define SP_DEBUGBREAK()
#endif // SP_DEBUG

#ifdef SP_ENABLE_ASSERTS
	#define SP_ASSERT(x, ...) { if(!(x)) { SP_ERROR("Assertion Failed: {}", __VA_ARGS__); SP_DEBUGBREAK(); } }
	#define SP_CORE_ASSERT(x, ...) { if(!(x)) { SP_CORE_ERROR("Assertion Failed: {}", __VA_ARGS__); SP_DEBUGBREAK(); } }
#else
	#define SP_ASSERT(x, ...)
	#define SP_CORE_ASSERT(x, ...)
#endif // SP_ENABLE_ASSERTS

#define SP_ARRAYCOUNT(ident) sizeof(ident) / sizeof(ident[0])

#define SP_BIND_CALLBACK(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

#define BIT(x) (1 << x)

namespace Sparky {

	template <typename T>
	using UniqueRef = std::unique_ptr<T>;

	template <typename T, typename ... Args>
	constexpr UniqueRef<T> CreateUnique(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template <typename T>
	using SharedRef = std::shared_ptr<T>;
	
	template <typename T, typename ... Args>
	constexpr SharedRef<T> CreateShared(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

}
