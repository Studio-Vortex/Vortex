#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/Math/Math.h"
#include "Vortex/Core/ReferenceCounting/SharedRef.h"

#include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h>

namespace Vortex {

	using Microsoft::WRL::ComPtr;

	namespace Utils {

		inline static void DirectXCheckResult(HRESULT hr, const char* message)
		{
			if (hr == S_OK)
				return;

			VX_CORE_ASSERT(false, message);
		}

	}

}

#define DX_CHECK(hr, ...) { ::Vortex::Utils::DirectXCheckResult((hr), __VA_ARGS__); }
