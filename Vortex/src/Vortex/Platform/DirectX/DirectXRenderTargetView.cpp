#include "vxpch.h"
#include "DirectXRenderTargetView.h"

namespace Vortex {

	SharedRef<DirectXRenderTargetView> DirectXRenderTargetView::Create()
	{
		return CreateShared<DirectXRenderTargetView>();
	}

}
