#pragma once

#include "Vortex/Platform/DirectX/DirectX.h"

#include "Vortex/Platform/DirectX/DirectXMappedSubresource.h"

namespace Vortex {

	enum class ResourceMapType
	{
		None = 0,
		Read,
		ReadWrite,
		Write,
		WriteDiscard,
		WriteNoOverWrite,
	};

	class DirectXDeviceContext : public RefCounted
	{
	public:
		DirectXDeviceContext() = default;
		~DirectXDeviceContext() = default;

		VX_FORCE_INLINE ID3D11DeviceContext** GetAddressOf() { return m_DeviceContext.GetAddressOf(); }
		VX_FORCE_INLINE ID3D11DeviceContext* const* GetAddressOf() const { return m_DeviceContext.GetAddressOf(); }
		VX_FORCE_INLINE ID3D11DeviceContext* Get() const { return m_DeviceContext.Get(); }

		void Draw(uint32_t vertexCount, uint32_t offset);
		void DrawIndexed(uint32_t indexCount, uint32_t offset, int32_t baseVertexOffset);

		std::pair<UniqueRef<DirectXMappedSubresource>, HRESULT> MapResource(ID3D11Resource* resource, ResourceMapType type);
		void UnmapResource(ID3D11Resource* resource);

		static SharedRef<DirectXDeviceContext> Create();

	private:
		ComPtr<ID3D11DeviceContext> m_DeviceContext = nullptr;
	};

}
