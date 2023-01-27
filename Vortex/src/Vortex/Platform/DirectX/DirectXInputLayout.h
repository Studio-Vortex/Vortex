#pragma once

#include "Vortex/Platform/DirectX/DirectX.h"

namespace Vortex {

	class DirectXInputLayout : public RefCounted
	{
	public:
		DirectXInputLayout() = default;
		~DirectXInputLayout() = default;

		static SharedRef<DirectXInputLayout> Create();

	private:

	};

}
