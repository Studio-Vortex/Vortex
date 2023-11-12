#pragma once

#include "Vortex/Core/Base.h"

#include <unordered_map>
#include <vector>

namespace Vortex {

	template <typename V, typename K>
	class VORTEX_API LibraryBase
	{
	public:
		virtual ~LibraryBase() = default;

		virtual void Add(const V& value) = 0;
		virtual uint8_t Remove(const K& identifier) = 0;

		virtual bool Exists(const K& identifier) const = 0;

		virtual size_t Size() const { return m_LibraryData.size(); }

		virtual const V& Get(const K& identifier) const = 0;
		virtual V& Get(const K& identifier) = 0;

		VX_FORCE_INLINE std::vector<V>::iterator begin() { return m_LibraryData.begin(); }
		VX_FORCE_INLINE std::vector<V>::iterator end() { return m_LibraryData.end(); }

		VX_FORCE_INLINE std::vector<V>::const_iterator begin() const { return m_LibraryData.begin(); }
		VX_FORCE_INLINE std::vector<V>::const_iterator end() const { return m_LibraryData.end(); }

	protected:
		std::vector<V> m_LibraryData;
	};

	template <typename V, typename K>
	class VORTEX_API LibraryMapBase
	{
	public:
		virtual ~LibraryMapBase() = default;

		virtual void Add(const V& value) = 0;
		virtual uint8_t Remove(const K& identifier) = 0;

		virtual bool Exists(const K& identifier) const { return m_LibraryData.find(identifier) != m_LibraryData.end(); }

		virtual size_t Size() const { return m_LibraryData.size(); }

		virtual const V& Get(const K& identifier) const = 0;
		virtual V& Get(const K& identifier) = 0;

		VX_FORCE_INLINE std::unordered_map<K, V>::iterator begin() { return m_LibraryData.begin(); }
		VX_FORCE_INLINE std::unordered_map<K, V>::iterator end() { return m_LibraryData.end(); }

		VX_FORCE_INLINE std::unordered_map<K, V>::const_iterator begin() const { return m_LibraryData.begin(); }
		VX_FORCE_INLINE std::unordered_map<K, V>::const_iterator end() const { return m_LibraryData.end(); }

	protected:
		std::unordered_map<K, V> m_LibraryData;
	};

}
