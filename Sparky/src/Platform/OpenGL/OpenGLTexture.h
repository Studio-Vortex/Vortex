#pragma once

#include "Sparky/Renderer/Texture.h"

typedef unsigned int GLenum;

namespace Sparky {

	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(uint32_t width, uint32_t height);
		OpenGLTexture2D(const std::string& path, bool flipVertical);
		~OpenGLTexture2D() override;

		inline uint32_t GetWidth() const override { return m_Width; }
		inline uint32_t GetHeight() const override { return m_Height; }

		const std::string& GetPath() const override { return m_Path; }

		void SetData(const void* data, uint32_t size) override;
		void SetData(void* data, uint32_t size) override;

		void Bind(uint32_t slot) const override;
		void Unbind() const override;

		bool IsLoaded() const override { return m_IsLoaded; }

		inline uint32_t GetRendererID() const override { return m_RendererID; }

		inline bool operator==(const Texture& other) const override
		{
			return m_RendererID == ((OpenGLTexture2D&)other).m_RendererID;
		}

	private:
		std::string m_Path;
		mutable uint32_t m_Slot;
		uint32_t m_Width;
		uint32_t m_Height;
		uint32_t m_RendererID = 0;
		GLenum m_InternalFormat, m_DataFormat;
		bool m_IsLoaded = false;
	};

}
