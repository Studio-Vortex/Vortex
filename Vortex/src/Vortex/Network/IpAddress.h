#pragma once

#include "Vortex/Core/Base.h"

#include <string>
#include <format>

namespace Vortex {

	struct IpAddress
	{
		uint8_t AddressBuf[4];

		IpAddress(uint8_t nid0 = 127, uint8_t nid1 = 0, uint8_t nid2 = 0, uint8_t hid0 = 1) {
			AddressBuf[0] = nid0;
			AddressBuf[1] = nid1;
			AddressBuf[2] = nid2;
			AddressBuf[3] = hid0;
		}

		std::string ToString() const
		{
			return std::format("{}.{}.{}.{}",
				AddressBuf[0],
				AddressBuf[1],
				AddressBuf[2],
				AddressBuf[3]
			);
		}

		static IpAddress GetLoopbackAddr()
		{
			IpAddress addr;
			addr.AddressBuf[0] = 127;
			addr.AddressBuf[1] = 0;
			addr.AddressBuf[2] = 0;
			addr.AddressBuf[3] = 1;
			return addr;
		}
	};

}
