#pragma once

#include "Vortex/Core/PlatformDetection.h"

#include <vector>
#include <memory>

// DLL support
#ifdef VX_PLATFORM_WINDOWS
	#if VX_DYNAMIC_LINK
		#ifdef VX_BUILD_DLL
			#define VORTEX_API __declspec(dllexport)
		#else
			#define VORTEX_API __declspec(dllimport)
		#endif
	#else
		#define VORTEX_API
	#endif
#else
	#error "Vortex only supports Windows!"
#endif // End of DLL support

#ifdef VX_DEBUG
	#if defined(VX_PLATFORM_WINDOWS)
		#define VX_DEBUGBREAK() __debugbreak()
	#elif defined(SP_PLATFORM_LINUX)
		#include <signal.h>
		#define VX_DEBUGBREAK() raise(SIGTRAP)
	#else
		#error "Platform doesn't support debugbreak!"
	#endif
	#define VX_ENABLE_ASSERTS
#else
	#define VX_DEBUGBREAK()
#endif // VX_DEBUG

#ifdef VX_ENABLE_ASSERTS
	#define SP_ASSERT(x, ...) { if(!(x)) { VX_ERROR("Assertion Failed: {}", __VA_ARGS__); VX_DEBUGBREAK(); } }
	#define VX_CORE_ASSERT(x, ...) { if(!(x)) { VX_CORE_ERROR("Assertion Failed: {}", __VA_ARGS__); VX_DEBUGBREAK(); } }
#else
	#define SP_ASSERT(x, ...)
	#define VX_CORE_ASSERT(x, ...)
#endif // VX_ENABLE_ASSERTS

#define SP_ARRAYCOUNT(ident) sizeof(ident) / sizeof(ident[0])

#define SP_BIND_CALLBACK(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

#define BIT(x) (1 << x)

namespace Vortex {

	void InitalizeEngine();
	void ShutdownEngine();

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
