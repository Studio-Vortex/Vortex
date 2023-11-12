#include "vxpch.h"
#include "Font.h"

#include "Vortex/Project/Project.h"

#include "Vortex/Renderer/Font/MSDFData.h"

#include "Vortex/Utils/FileSystem.h"

namespace Vortex {

	static constexpr const char* DEFAULT_FONT_PATH = "Resources/Fonts/opensans/OpenSans-Regular.ttf";

	using namespace msdf_atlas;

	struct FontInput
	{
		const char* fontFilename;
		GlyphIdentifierType glyphIdentifierType;
		const char* charsetFilename;
		double fontScale;
		const char* fontName;
	};

	struct Configuration {
		ImageType imageType;
		msdf_atlas::ImageFormat imageFormat;
		YDirection yDirection;
		int width, height;
		double emSize;
		double pxRange;
		double angleThreshold;
		double miterLimit;
		void (*edgeColoring)(msdfgen::Shape&, double, unsigned long long);
		bool expensiveColoring;
		unsigned long long coloringSeed;
		GeneratorAttributes generatorAttributes;
	};

#define DEFAULT_ANGLE_THRESHOLD 3.0
#define DEFAULT_MITER_LIMIT 1.0
#define LCG_MULTIPLIER 6364136223846793005ull
#define LCG_INCREMENT 1442695040888963407ull
#define THREADS 8

	namespace Utils {

		static std::filesystem::path GetCacheDirectory()
		{
			return "Resources/Cache/FontAtlases";
		}

		static void CreateCacheDirectoryIfNeeded()
		{
			std::filesystem::path cacheDirectory = GetCacheDirectory();
			if (!std::filesystem::exists(cacheDirectory))
				std::filesystem::create_directories(cacheDirectory);
		}

	}

	struct AtlasHeader
	{
		uint32_t Type = 0;
		uint32_t Width, Height;
	};

	static bool TryReadFontAtlasFromCache(const std::string& fontName, float fontSize, AtlasHeader& header, void*& pixels, Buffer& storageBuffer)
	{
		std::string filename = fmt::format("{}-{}.vfa", fontName, fontSize);
		std::filesystem::path filepath = Utils::GetCacheDirectory() / filename;

		if (std::filesystem::exists(filepath))
		{
			storageBuffer = FileSystem::ReadBinary(filepath);
			header = *storageBuffer.As<AtlasHeader>();
			pixels = (uint8_t*)storageBuffer.Data + sizeof(AtlasHeader);
			return true;
		}

		return false;
	}

	static void CacheFontAtlas(const std::string& fontName, float fontSize, AtlasHeader header, const void* pixels)
	{
		Utils::CreateCacheDirectoryIfNeeded();

		std::string filename = fmt::format("{}-{}.vfa", fontName, fontSize);
		std::filesystem::path filepath = Utils::GetCacheDirectory() / filename;

		std::ofstream stream(filepath, std::ios::binary | std::ios::trunc);
		if (!stream)
		{
			stream.close();
			VX_CORE_ERROR_TAG("Font", "Failed to cache font atlas to {0}", filepath.string());
			return;
		}

		stream.write((char*)&header, sizeof(AtlasHeader));
		stream.write((char*)pixels, header.Width * header.Height * sizeof(float) * 4);

		VX_CORE_INFO_TAG("Font", "Font Atlas successfully cached: {}", filepath.string());
	}

	template <typename T, typename S, int N, GeneratorFunction<S, N> GenFunc>
	static SharedReference<Texture2D> CreateAndCacheAtlas(const std::string& fontName, float fontSize, const std::vector<GlyphGeometry>& glyphs, const FontGeometry& fontGeometry, const Configuration& config)
	{
		ImmediateAtlasGenerator<S, N, GenFunc, BitmapAtlasStorage<T, N>> generator(config.width, config.height);
		generator.setAttributes(config.generatorAttributes);
		generator.setThreadCount(THREADS);
		generator.generate(glyphs.data(), (int)glyphs.size());

		msdfgen::BitmapConstRef<T, N> bitmap = (msdfgen::BitmapConstRef<T, N>) generator.atlasStorage();

		AtlasHeader header;
		header.Width = bitmap.width;
		header.Height = bitmap.height;
		CacheFontAtlas(fontName, fontSize, header, bitmap.pixels);

		TextureProperties imageProps;
		imageProps.Width = header.Width;
		imageProps.Height = header.Height;
		imageProps.TextureFormat = ImageFormat::RGBA32F;

		SharedReference<Texture2D> texture = Texture2D::Create(imageProps);
		texture->SetData(bitmap.pixels, header.Width * header.Height * 4);
		return texture;
	}

	static SharedReference<Texture2D> CreateCachedAtlas(AtlasHeader header, const void* pixels)
	{
		TextureProperties imageProps;
		imageProps.Width = header.Width;
		imageProps.Height = header.Height;
		imageProps.TextureFormat = ImageFormat::RGBA32F;

		SharedReference<Texture2D> texture = Texture2D::Create(imageProps);
		texture->SetData(pixels, header.Width * header.Height * 4);
		return texture;
	}

	Font::Font(const std::filesystem::path& filepath)
		: m_Filepath(filepath), m_MSDFData(new MSDFData())
	{
		int result{};
		FontInput fontInput{};
		Configuration config{};
		fontInput.glyphIdentifierType = GlyphIdentifierType::UNICODE_CODEPOINT;
		fontInput.fontScale = -1;
		config.imageType = ImageType::MSDF;
		config.imageFormat = msdf_atlas::ImageFormat::BINARY_FLOAT;
		config.yDirection = YDirection::BOTTOM_UP;
		config.edgeColoring = msdfgen::edgeColoringInkTrap;
		const char* imageFormatName = nullptr;
		int fixedWidth = -1, fixedHeight = -1;
		config.generatorAttributes.config.overlapSupport = true;
		config.generatorAttributes.scanlinePass = true;
		double minEmSize = 0;
		double rangeValue = 2.0;
		TightAtlasPacker::DimensionsConstraint atlasSizeConstraint = TightAtlasPacker::DimensionsConstraint::MULTIPLE_OF_FOUR_SQUARE;
		config.angleThreshold = DEFAULT_ANGLE_THRESHOLD;
		config.miterLimit = DEFAULT_MITER_LIMIT;
		config.imageType = ImageType::MTSDF;

		std::string fontFilepath = m_Filepath.string();
		fontInput.fontFilename = fontFilepath.c_str();

		config.emSize = 40;

		// Load fonts
		bool anyCodepointsAvailable = false;
		class FontHolder {
		public:
			FontHolder()
				: ft(msdfgen::initializeFreetype()), font(nullptr), fontFilename(nullptr) { }

			~FontHolder() {
				if (ft) {
					if (font)
						msdfgen::destroyFont(font);
					msdfgen::deinitializeFreetype(ft);
				}
			}

			bool load(const char* fontFilename) {
				if (ft && fontFilename) {
					if (this->fontFilename && !strcmp(this->fontFilename, fontFilename))
						return true;
					if (font)
						msdfgen::destroyFont(font);
					if ((font = msdfgen::loadFont(ft, fontFilename))) {
						this->fontFilename = fontFilename;
						return true;
					}
					this->fontFilename = nullptr;
				}
				return false;
			}

			operator msdfgen::FontHandle* () const {
				return font;
			}

		private:
			msdfgen::FreetypeHandle* ft;
			msdfgen::FontHandle* font;
			const char* fontFilename;
		} font;

		bool success = font.load(fontInput.fontFilename);
		VX_CORE_ASSERT(success, "");

		if (fontInput.fontScale <= 0)
			fontInput.fontScale = 1;

		// Load character set
		fontInput.glyphIdentifierType = GlyphIdentifierType::UNICODE_CODEPOINT;
		Charset charset;

		// From ImGui
		static const uint32_t charsetRanges[] =
		{
			0x0020, 0x00FF, // Basic Latin + Latin Supplement
			0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
			0x2DE0, 0x2DFF, // Cyrillic Extended-A
			0xA640, 0xA69F, // Cyrillic Extended-B
			0,
		};

		for (int range = 0; range < 8; range += 2)
		{
			for (uint32_t c = charsetRanges[range]; c <= charsetRanges[range + 1]; c++)
				charset.add(c);
		}

		// Load glyphs
		m_MSDFData->FontGeometry = FontGeometry(&m_MSDFData->Glyphs);
		int glyphsLoaded = -1;
		switch (fontInput.glyphIdentifierType)
		{
			case GlyphIdentifierType::GLYPH_INDEX:
				glyphsLoaded = m_MSDFData->FontGeometry.loadGlyphset(font, fontInput.fontScale, charset);
				break;
			case GlyphIdentifierType::UNICODE_CODEPOINT:
				glyphsLoaded = m_MSDFData->FontGeometry.loadCharset(font, fontInput.fontScale, charset);
				anyCodepointsAvailable |= glyphsLoaded > 0;
				break;
		}

		VX_CORE_ASSERT(glyphsLoaded >= 0, "No geometry glyphs were loaded!");
		VX_CORE_TRACE_TAG("Font", "Loaded geometry of {} out of {} glyphs", glyphsLoaded, (int)charset.size());

		// List missing glyphs
		if (glyphsLoaded < (int)charset.size())
		{
			VX_CORE_WARN_TAG("Font", "Missing {} {}", (int)charset.size() - glyphsLoaded, fontInput.glyphIdentifierType == GlyphIdentifierType::UNICODE_CODEPOINT ? "codepoints" : "glyphs");
		}

		if (fontInput.fontName)
			m_MSDFData->FontGeometry.setName(fontInput.fontName);

			// Determine final atlas dimensions, scale and range, pack glyphs
		double pxRange = rangeValue;
		bool fixedDimensions = fixedWidth >= 0 && fixedHeight >= 0;
		bool fixedScale = config.emSize > 0;
		TightAtlasPacker atlasPacker;
		if (fixedDimensions)
			atlasPacker.setDimensions(fixedWidth, fixedHeight);
		else
			atlasPacker.setDimensionsConstraint(atlasSizeConstraint);
		atlasPacker.setPadding(config.imageType == ImageType::MSDF || config.imageType == ImageType::MTSDF ? 0 : -1);
		// TODO: In this case (if padding is -1), the border pixels of each glyph are black, but still computed. For floating-point output, this may play a role.
		if (fixedScale)
			atlasPacker.setScale(config.emSize);
		else
			atlasPacker.setMinimumScale(minEmSize);
		atlasPacker.setPixelRange(pxRange);
		atlasPacker.setMiterLimit(config.miterLimit);

		if (int remaining = atlasPacker.pack(m_MSDFData->Glyphs.data(), (int)m_MSDFData->Glyphs.size()))
		{
			if (remaining < 0)
			{
				VX_CORE_ASSERT(false, "");
			}
			else
			{
				VX_CORE_ERROR_TAG("Font", "Error: Could not fit {0} out of {1} glyphs into the atlas.", remaining, (int)m_MSDFData->Glyphs.size());
				VX_CORE_ASSERT(false,  "");
			}
		}

		atlasPacker.getDimensions(config.width, config.height);
		VX_CORE_ASSERT(config.width > 0 && config.height > 0, "");
		config.emSize = atlasPacker.getScale();
		config.pxRange = atlasPacker.getPixelRange();
		if (!fixedScale)
			VX_CORE_TRACE_TAG("Font", "Glyph size: {0} pixels/EM", config.emSize);
		if (!fixedDimensions)
			VX_CORE_TRACE_TAG("Font", "Atlas dimensions: {0} x {1}", config.width, config.height);


		// Edge coloring
		if (config.imageType == ImageType::MSDF || config.imageType == ImageType::MTSDF)
		{
			if (config.expensiveColoring) {
				Workload([&glyphs = m_MSDFData->Glyphs, &config](int i, int threadNo) -> bool
					{
						unsigned long long glyphSeed = (LCG_MULTIPLIER * (config.coloringSeed ^ i) + LCG_INCREMENT) * !!config.coloringSeed;
						glyphs[i].edgeColoring(config.edgeColoring, config.angleThreshold, glyphSeed);
						return true;
					}, (int)m_MSDFData->Glyphs.size()).finish(THREADS);
			}
			else
			{
				unsigned long long glyphSeed = config.coloringSeed;
				for (GlyphGeometry& glyph : m_MSDFData->Glyphs)
				{
					glyphSeed *= LCG_MULTIPLIER;
					glyph.edgeColoring(config.edgeColoring, config.angleThreshold, glyphSeed);
				}
			}
		}

		std::string fontName = filepath.filename().string();

		// Check cache here
		Buffer storageBuffer;
		AtlasHeader header;
		void* pixels = nullptr;

		if (TryReadFontAtlasFromCache(fontName, (float)config.emSize, header, pixels, storageBuffer))
		{
			m_TextureAtlas = CreateCachedAtlas(header, pixels);
			storageBuffer.Release();
		}
		else
		{
			bool floatingPointFormat = true;
			SharedReference<Texture2D> texture = nullptr;

			switch (config.imageType)
			{
				case ImageType::MSDF:
					if (floatingPointFormat)
						texture = CreateAndCacheAtlas<float, float, 3, msdfGenerator>(fontName, (float)config.emSize, m_MSDFData->Glyphs, m_MSDFData->FontGeometry, config);
					else
						texture = CreateAndCacheAtlas<byte, float, 3, msdfGenerator>(fontName, (float)config.emSize, m_MSDFData->Glyphs, m_MSDFData->FontGeometry, config);
					break;
				case ImageType::MTSDF:
					if (floatingPointFormat)
						texture = CreateAndCacheAtlas<float, float, 4, mtsdfGenerator>(fontName, (float)config.emSize, m_MSDFData->Glyphs, m_MSDFData->FontGeometry, config);
					else
						texture = CreateAndCacheAtlas<byte, float, 4, mtsdfGenerator>(fontName, (float)config.emSize, m_MSDFData->Glyphs, m_MSDFData->FontGeometry, config);
					break;
			}

			m_TextureAtlas = texture;
		}
	}

	Font::~Font()
	{
		delete m_MSDFData;
	}

	void Font::Init()
	{
		s_DefaultFont = Font::Create(DEFAULT_FONT_PATH);
	}

	void Font::Shutdown()
	{
		s_DefaultFont.Reset();
	}

	SharedReference<Font>& Font::GetDefaultFont()
	{
		return s_DefaultFont;
	}

	SharedReference<Font> Font::Create(const std::filesystem::path& filepath)
	{
		return SharedReference<Font>::Create(filepath);
	}

}
