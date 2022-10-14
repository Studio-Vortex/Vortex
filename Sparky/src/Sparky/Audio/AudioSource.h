#pragma once

namespace Sparky {

	class AudioSource
	{
	public:
		AudioSource(const std::string& filepath)
			: m_Path(filepath) { }

		~AudioSource() = default;

		inline const std::string& GetPath() const { return m_Path; }

	private:
		std::string m_Path;
	};

}
