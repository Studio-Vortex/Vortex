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
#endif // DLL support

#ifdef VX_DEBUG
	#if defined(VX_PLATFORM_WINDOWS)
		#define VX_DEBUGBREAK() __debugbreak()
	#elif defined(VX_PLATFORM_LINUX)
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
	#define VX_ASSERT(x, ...) { if(!(x)) { VX_ERROR("Assertion Failed: {}", __VA_ARGS__); VX_DEBUGBREAK(); } }
	#define VX_CORE_ASSERT(x, ...) { if(!(x)) { VX_CONSOLE_LOG_ERROR("Assertion Failed: {}", __VA_ARGS__); VX_DEBUGBREAK(); } }
#else
	#define VX_ASSERT(x, ...)
	#define VX_CORE_ASSERT(x, ...)
#endif // VX_ENABLE_ASSERTS

#ifdef VX_64BIT
	#define VX_POINTER_WIDTH 8
#elif
	#define VX_POINTER_WIDTH 4
#endif // VX_64

#define VX_VERIFY(x) { if(!(x)) { VX_ERROR("Verify Failed: {} {}", __FILE__, __LINE__); } }
#define VX_CORE_VERIFY(x) { if (!(x)) { VX_ERROR("Verify Failed: {} {}", __FILE__, __LINE__); } }

#define VX_FORCE_INLINE inline

#define VX_ARRAYSIZE(ident) sizeof(ident) / sizeof(ident[0])

#define VX_STRINGIFY(input) #input

#define VX_BIND_CALLBACK(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

#define VORTEX_BUILD_ID "v0.1a"

#define BIT(x) (1 << x)

namespace Vortex {

	VORTEX_API void InitalizeEngine();
	VORTEX_API void ShutdownEngine();

	template <typename T>
	using VORTEX_API UniqueRef = std::unique_ptr<T>;

	template <typename T>
	using VORTEX_API SharedRef = std::shared_ptr<T>;

	template <typename T>
	using VORTEX_API WeakRef = std::weak_ptr<T>;

	template <typename T, typename... Args>
	VORTEX_API constexpr UniqueRef<T> CreateUnique(Args&&... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template <typename T, typename... Args>
	VORTEX_API constexpr SharedRef<T> CreateShared(Args&&... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	template <typename T, typename... Args>
	VORTEX_API constexpr WeakRef<T> CreateWeak(SharedRef<T> ptr)
	{
		return WeakRef<T>(ptr);
	}

}
