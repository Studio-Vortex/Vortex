#pragma once

#include <stdint.h>
#include <cstring> // for memcpy

namespace Vortex {

	// Non-owning buffer
	struct Buffer
	{
		uint8_t* Data = nullptr;
		uint64_t Size = 0;

		inline Buffer() = default;

		inline Buffer(uint64_t size)
		{
			Allocate(size);
		}

		inline Buffer(const Buffer&) = default;

		inline static [[nodiscard]] Buffer Copy(const Buffer& other)
		{
			Buffer result(other.Size);
			memcpy(result.Data, other.Data, other.Size);
			return result;
		}

		inline void Allocate(uint64_t size)
		{
			Release();

			Data = new uint8_t[size];
			Size = size;
		}

		inline void Release()
		{
			delete[] Data;
			Data = nullptr;
			Size = 0;
		}

		template <typename T>
		inline [[nodiscard]] T* As()
		{
			return (T*)Data;
		}

		inline [[nodiscard]] operator bool() const
		{
			return (bool)Data;
		}
	};

	struct UniqueBuffer
	{
		inline UniqueBuffer(Buffer buffer)
			: m_Buffer(buffer) { }

		inline ~UniqueBuffer()
		{
			m_Buffer.Release();
		}

		inline [[nodiscard]] uint8_t* Data() { return m_Buffer.Data; }
		inline [[nodiscard]] uint64_t Size() { return m_Buffer.Size; }

		template <typename T>
		inline [[nodiscard]] T* As()
		{
			return m_Buffer.As<T>();
		}

		inline [[nodiscard]] operator bool() const
		{
			return m_Buffer;
		}

	private:
		Buffer m_Buffer;
	};

}
