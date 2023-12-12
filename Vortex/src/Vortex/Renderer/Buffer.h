#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/ReferenceCounting/RefCounted.h"
#include "Vortex/ReferenceCounting/SharedRef.h"

namespace Vortex {

	enum class VORTEX_API ShaderDataType
	{
		None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Boolean,
	};

	static uint32_t ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
			case Vortex::ShaderDataType::Float:    return 4;
			case Vortex::ShaderDataType::Float2:   return 4 * 2;
			case Vortex::ShaderDataType::Float3:   return 4 * 3;
			case Vortex::ShaderDataType::Float4:   return 4 * 4;
			case Vortex::ShaderDataType::Mat3:     return 4 * 3 * 3;
			case Vortex::ShaderDataType::Mat4:     return 4 * 4 * 4;
			case Vortex::ShaderDataType::Int:      return 4;
			case Vortex::ShaderDataType::Int2:     return 4 * 2;
			case Vortex::ShaderDataType::Int3:     return 4 * 3;
			case Vortex::ShaderDataType::Int4:     return 4 * 4;
			case Vortex::ShaderDataType::Boolean:  return 1;
		}

		VX_CORE_ASSERT(false, "Unknown Shader Data Type!");
		return 0;
	}

	struct VORTEX_API BufferElement
	{
		std::string Name;
		ShaderDataType Type;
		uint32_t Size;
		uint32_t Offset;
		bool Normalized;

		BufferElement(const ShaderDataType& type, const std::string& name, bool normalized = false)
			: Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized)
		{
		}

		uint32_t GetComponentCount() const
		{
			switch (Type)
			{
				case Vortex::ShaderDataType::Float:    return 1;
				case Vortex::ShaderDataType::Float2:   return 2;
				case Vortex::ShaderDataType::Float3:   return 3;
				case Vortex::ShaderDataType::Float4:   return 4;
				case Vortex::ShaderDataType::Mat3:     return 3 * 3;
				case Vortex::ShaderDataType::Mat4:     return 4 * 4;
				case Vortex::ShaderDataType::Int:      return 1;
				case Vortex::ShaderDataType::Int2:     return 2;
				case Vortex::ShaderDataType::Int3:     return 3;
				case Vortex::ShaderDataType::Int4:     return 4;
				case Vortex::ShaderDataType::Boolean:  return 1;
			}

			VX_CORE_ASSERT(false, "Unknown Shader Data Type!");
			return 0;
		}
	};

	class VORTEX_API BufferLayout
	{
	public:
		BufferLayout() = default;

		BufferLayout(const std::initializer_list<BufferElement>& elements)
			: m_Elements(elements)
		{
			CalculateOffsetsAndStride();
		}

		inline uint32_t GetStride() const { return m_Stride; }
		inline const std::vector<BufferElement>& GetElements() const { return m_Elements; }

		inline std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
		inline std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
		inline std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		inline std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }

	private:
		inline void CalculateOffsetsAndStride()
		{
			uint32_t offset = 0;
			m_Stride = 0;

			for (auto& element : m_Elements)
			{
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
			}
		}

	private:
		std::vector<BufferElement> m_Elements;
		uint32_t m_Stride = 0;
	};

	class VORTEX_API VertexBuffer : public RefCounted
	{
	public:
		virtual ~VertexBuffer() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetData(const void* data, uint32_t size) const = 0;

		virtual const BufferLayout& GetLayout() const = 0;
		virtual void SetLayout(const BufferLayout& layout) = 0;

		static SharedReference<VertexBuffer> Create(float* vertices, uint32_t size);
		static SharedReference<VertexBuffer> Create(const void* vertices, uint32_t size);
		static SharedReference<VertexBuffer> Create(uint32_t size);
	};

	// Only supports 32 bit unsigned integers
	class VORTEX_API IndexBuffer : public RefCounted
	{
	public:
		virtual ~IndexBuffer() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual uint32_t GetCount() const = 0;

		static SharedReference<IndexBuffer> Create(uint32_t* indices, uint32_t count);
		static SharedReference<IndexBuffer> Create(uint16_t* indices, uint32_t count);
	};

}
