#pragma once

#include "Vortex/Platform/DirectX/DirectX.h"

namespace Vortex {

	class DirectXRenderTargetView
	{
	public:
		DirectXRenderTargetView() = default;
		~DirectXRenderTargetView() = default;

		static SharedRef<DirectXRenderTargetView> Create();

	private:

	};

}
