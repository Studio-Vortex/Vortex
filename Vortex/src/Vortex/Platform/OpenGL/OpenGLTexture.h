#pragma once

#include "Vortex/Renderer/Texture.h"

typedef unsigned int GLenum;

namespace Vortex {

	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(const TextureProperties& imageProps);
		~OpenGLTexture2D() override;

		const TextureProperties& GetProperties() const override { return m_Properties; }

		inline uint32_t GetWidth() const override { return m_Properties.Width; }
		inline uint32_t GetHeight() const override { return m_Properties.Height; }

		const std::string& GetPath() const override { return m_Properties.Filepath; }

		void SetData(const void* data, uint32_t size) override;
		void SetData(void* data, uint32_t size) override;

		void Bind(uint32_t slot) const override;
		void Unbind() const override;

		bool IsLoaded() const override { return m_Properties.IsLoaded; }

		inline uint32_t GetRendererID() const override { return m_RendererID; }

		void SaveToFile() const override;

		inline bool operator==(const Texture& other) const override
		{
			return m_RendererID == ((OpenGLTexture2D&)other).m_RendererID;
		}

	private:
		void CreateImageFromWidthAndHeight();
		void CreateImageFromHDRFile();
		void CreateImageFromFile();

	private:
		TextureProperties m_Properties;
		mutable uint32_t m_Slot;
		uint32_t m_RendererID = 0;
		GLenum m_InternalFormat, m_DataFormat;
	};

}
