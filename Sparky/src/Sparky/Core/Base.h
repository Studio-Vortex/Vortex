#pragma once

#include <memory>

// Platform detection using predefined macros
#ifdef _WIN32
	/* Windows x64/x86 */
	#ifdef _WIN64
		/* Windows x64  */
		#define SP_PLATFORM_WINDOWS
	#else
		/* Windows x86 */
		#error "x86 Builds are not supported!"
	#endif
#elif defined(__APPLE__) || defined(__MACH__)
	#include <TargetConditionals.h>
	/* TARGET_OS_MAC exists on all the platforms
	* so we must check all of them (in this order)
	* to ensure that we're running on MAC
	* and not some other Apple platform */
	#if TARGET_IPHONE_SIMULATOR == 1
		#error "IOS simulator is not supported!"
	#elif TARGET_OS_IPHONE == 1
		#define SP_PLATFORM_IOS
		#error "IOS is not supported!"
	#elif TARGET_OS_MAC == 1
		#define SP_PLATFORM_MACOS
		#error "MacOS is not supported!"
	#else
		#error "Unknown Apple platform!"
	#endif
/* We also have to check __ANDROID__ before __linux__
 * since android is based on the linux kernel
 * it has __linux__ defined */
#elif defined(__ANDROID__)
	#define SP_PLATFORM_ANDROID
	#error "Android is not supported!"
#elif defined(__linux__)
	#define SP_PLATFORM_LINUX
	#error "Linux is not supported!"
#else
	/* Unknown compiler/platform */
	#error "Unknown platform!"
#endif // End of platform detection


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

#define SP_BIND_CALLBACK(ident) std::bind(&ident, this, std::placeholders::_1)

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