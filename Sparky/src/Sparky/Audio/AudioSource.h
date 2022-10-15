#pragma once

namespace Sparky {

	class AudioSource
	{
	public:
		AudioSource(const std::string& filepath)
			: m_Path(filepath) { }

		~AudioSource() = default;

		inline const std::string& GetPath() const { return m_Path; }

		struct AudioProperties
		{
			bool Loop = false;
		};

		inline const AudioProperties& GetProperties() const { return m_Settings; }
		inline AudioProperties& GetProperties() { return m_Settings; }

	private:
		std::string m_Path;
		AudioProperties m_Settings;
	};

}
