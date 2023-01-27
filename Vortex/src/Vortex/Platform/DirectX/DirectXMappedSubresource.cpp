#include "vxpch.h"
#include "DirectXMappedSubresource.h"

namespace Vortex {
	
	DirectXMappedSubresource::DirectXMappedSubresource()
	{
		memset(&m_Subresource, 0, sizeof(D3D11_MAPPED_SUBRESOURCE));
	}

	DirectXMappedSubresource::~DirectXMappedSubresource()
	{

	}

	D3D11_MAPPED_SUBRESOURCE* DirectXMappedSubresource::GetAddressOf()
	{
		return &m_Subresource;
	}

}