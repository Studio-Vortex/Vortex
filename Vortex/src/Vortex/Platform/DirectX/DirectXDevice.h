#pragma once

#include "Vortex/Platform/DirectX/DirectX.h"

namespace Vortex {

	class DirectXDevice : public RefCounted
	{
	public:
		DirectXDevice() = default;
		~DirectXDevice() = default;

		VX_FORCE_INLINE ID3D11Device** GetAddressOf() { return m_Device.GetAddressOf(); }
		VX_FORCE_INLINE ID3D11Device* const* GetAddressOf() const { return m_Device.GetAddressOf(); }
		VX_FORCE_INLINE ID3D11Device* Get() const { return m_Device.Get(); }

		static SharedRef<DirectXDevice> Create();

	private:
		ComPtr<ID3D11Device> m_Device = nullptr;
	};

}
