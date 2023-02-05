#include "vxpch.h"
#include "DirectXDevice.h"

namespace Vortex {
	
	SharedRef<DirectXDevice> DirectXDevice::Create()
	{
		return CreateShared<DirectXDevice>();
	}

}