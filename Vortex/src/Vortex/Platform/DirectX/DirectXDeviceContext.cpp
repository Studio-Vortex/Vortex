#include "vxpch.h"
#include "DirectXDeviceContext.h"

namespace Vortex {

	namespace Utils {
		
		D3D11_MAP VortexResourceMapTypeToDXMapType(ResourceMapType mapType)
		{
			switch (mapType)
			{
				case ResourceMapType::Read:             return D3D11_MAP_READ;
				case ResourceMapType::ReadWrite:        return D3D11_MAP_READ_WRITE;
				case ResourceMapType::Write:            return D3D11_MAP_WRITE;
				case ResourceMapType::WriteDiscard:     return D3D11_MAP_WRITE_DISCARD;
				case ResourceMapType::WriteNoOverWrite: return D3D11_MAP_WRITE_NO_OVERWRITE;
			}

			VX_CORE_ASSERT(false, "Unknown Resource Map Type!");
			return (D3D11_MAP)0;
		}

	}

	void DirectXDeviceContext::Draw(uint32_t vertexCount, uint32_t offset)
	{
		m_DeviceContext->Draw(vertexCount, offset);
	}

	void DirectXDeviceContext::DrawIndexed(uint32_t indexCount, uint32_t offset, int32_t baseVertexOffset)
	{
		m_DeviceContext->DrawIndexed(indexCount, offset, baseVertexOffset);
	}

	std::pair<UniqueRef<DirectXMappedSubresource>, HRESULT> DirectXDeviceContext::MapResource(ID3D11Resource* resource, ResourceMapType type)
	{
		UniqueRef<DirectXMappedSubresource> mappedSubresource = CreateUnique<DirectXMappedSubresource>();
		D3D11_MAP mapType = Utils::VortexResourceMapTypeToDXMapType(type);
		HRESULT result = m_DeviceContext->Map(resource, 0, mapType, 0, mappedSubresource->GetAddressOf());
		return std::make_pair(std::move(mappedSubresource), result);
	}

	void DirectXDeviceContext::UnmapResource(ID3D11Resource* resource)
	{
		m_DeviceContext->Unmap(resource, 0);
	}

	SharedRef<DirectXDeviceContext> DirectXDeviceContext::Create()
	{
		return CreateShared<DirectXDeviceContext>();
	}

}