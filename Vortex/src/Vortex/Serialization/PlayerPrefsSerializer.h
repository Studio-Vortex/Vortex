#pragma once

#include "Vortex/Core/Base.h"

#include <unordered_map>
#include <string>

namespace YAML {

	class Emitter;

}

namespace Vortex {

	class VORTEX_API PlayerPrefsSerializer
	{
	public:
		PlayerPrefsSerializer() = default;
		~PlayerPrefsSerializer() = default;

		bool HasKey(const std::string& key) const;

		bool Remove(const std::string& key);

		void WriteInt(const std::string& key, int32_t value);
		bool ReadInt(const std::string& key, int32_t* value);

		void WriteString(const std::string& key, const std::string& value);
		bool ReadString(const std::string& key, std::string* value);

		bool Save(const std::string& filename) const;
		bool Load(const std::string& filename);

	private:
		void RemoveKey(const std::string& key);
		bool NoData() const;

	private:
		mutable YAML::Emitter* m_Serializer = nullptr;

		std::unordered_map<std::string, char> m_ByteData;

		std::unordered_map<std::string, float> m_F32Data;
		std::unordered_map<std::string, double> m_F64Data;

		std::unordered_map<std::string, int8_t> m_I8Data;
		std::unordered_map<std::string, int16_t> m_I16Data;
		std::unordered_map<std::string, int32_t> m_I32Data;
		std::unordered_map<std::string, int64_t> m_I64Data;

		std::unordered_map<std::string, uint8_t> m_U8Data;
		std::unordered_map<std::string, uint16_t> m_U16Data;
		std::unordered_map<std::string, uint32_t> m_U32Data;
		std::unordered_map<std::string, uint64_t> m_U64Data;

		std::unordered_map<std::string, std::string> m_StringData;
	};

}