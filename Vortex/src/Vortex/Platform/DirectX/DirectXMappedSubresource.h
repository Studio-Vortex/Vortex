#pragma once

#include "Vortex/Platform/DirectX/DirectX.h"

namespace Vortex {

	class DirectXMappedSubresource
	{
	public:
		DirectXMappedSubresource();
		~DirectXMappedSubresource();

		D3D11_MAPPED_SUBRESOURCE* GetAddressOf();

		template <typename TData>
		VX_FORCE_INLINE void CopyData(const TData* data)
		{
			memcpy(&m_Subresource.pData, data, sizeof(TData));
		}

	private:
		D3D11_MAPPED_SUBRESOURCE m_Subresource;
	};

}
