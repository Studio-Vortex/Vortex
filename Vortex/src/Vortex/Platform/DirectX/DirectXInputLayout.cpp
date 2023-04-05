#include "vxpch.h"
#include "DirectXInputLayout.h"

namespace Vortex {

	SharedRef<DirectXInputLayout> DirectXInputLayout::Create()
	{
		return CreateShared<DirectXInputLayout>();
	}

}
