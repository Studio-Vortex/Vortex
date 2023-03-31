#pragma once

#include "Vortex/Core/ReferenceCounting/InstancePool.h"
#include "Vortex/Core/ReferenceCounting/RefCounted.h"

namespace Vortex {

	template <typename T>
	class SharedReference
	{
	public:
		SharedReference()
			: m_Instance(nullptr) { }

		SharedReference(std::nullptr_t)
			: m_Instance(nullptr) { }

		SharedReference(T* instance)
			: m_Instance(instance)
		{
			static_assert(std::is_base_of<RefCounted, T>::value, "Class is not reference counted!");
			IncRef();
		}

		template <typename U>
		SharedReference(const SharedReference<U>& other)
		{
			m_Instance = (T*)other.m_Instance;
			IncRef();
		}

		template <typename U>
		SharedReference(SharedReference<U>&& other)
		{
			m_Instance = (T*)other.m_Instance;
			other.m_Instance = nullptr;
		}

		SharedReference(const SharedReference<T>& other)
			: m_Instance(other.m_Instance)
		{
			IncRef();
		}

		~SharedReference()
		{
			DecRef();
		}

	public:
		void Reset(T* instance = nullptr)
		{
			DecRef();
			m_Instance = instance;
		}

		template <typename U>
		constexpr SharedReference<U> As() const
		{
			return SharedReference<U>(*this);
		}

		template <typename U>
		constexpr SharedReference<U> Is() const
		{
			if constexpr (std::is_base_of<T, U>::value)
				return As<U>();

			return nullptr;
		}

		void Swap(const SharedReference<T>& other)
		{
			T* temp = m_Instance;
			m_Instance = other.m_Instance;
			other.m_Instance = temp;
			temp = nullptr;
		}

		bool EqualsObject(const SharedReference<T>& other)
		{
			if (!m_Instance || !other.m_Instance)
				return false;

			return *m_Instance == *other.m_Instance;
		}

	public:
		template <typename... Args>
		static SharedReference<T> Create(Args&&... args)
		{
			return SharedReference<T>(new T(std::forward<Args>(args)...));
		}

		static SharedReference<T> CopyWithoutIncrement(const SharedReference<T>& other)
		{
			SharedReference<T> result = nullptr;
			result->m_Instance = other.m_Instance;
			return result;
		}

	public:
		SharedReference& operator=(std::nullptr_t)
		{
			DecRef();
			m_Instance = nullptr;
			return *this;
		}

		SharedReference& operator=(const SharedReference<T>& other)
		{
			other.IncRef();
			DecRef();

			m_Instance = other.m_Instance;
			return *this;
		}

		template <typename U>
		SharedReference& operator=(const SharedReference<T>& other)
		{
			other.IncRef();
			DecRef();

			m_Instance = (T*)other.m_Instance;
			return *this;
		}

		template <typename U>
		SharedReference& operator=(SharedReference<U>&& other)
		{
			DecRef();

			m_Instance = (T*)other.m_Instance;
			other.m_Instance = nullptr;
			return *this;
		}

		operator bool() { return m_Instance != nullptr; }
		operator bool() const { return m_Instance != nullptr; }

		T* operator->() { return m_Instance; }
		const T* operator->() const { return m_Instance; }

		T& operator*() { return *m_Instance; }
		const T& operator*() const { return *m_Instance; }

		T* Raw() { return  m_Instance; }
		const T* Raw() const { return  m_Instance; }

		bool operator==(const SharedReference<T>& other) const
		{
			return m_Instance == other.m_Instance;
		}

		bool operator!=(const SharedReference<T>& other) const
		{
			return !(*this == other);
		}

	private:
		void IncRef() const
		{
			if (m_Instance)
			{
				m_Instance->IncRefCount();
				s_InstancePool.AddInstance((void*)m_Instance);
			}
		}

		void DecRef() const
		{
			if (m_Instance)
			{
				m_Instance->DecRefCount();

				if (m_Instance->GetRefCount() == 0)
				{
					delete m_Instance;
					s_InstancePool.RemoveInstance((void*)m_Instance);
					m_Instance = nullptr;
				}
			}
		}

	private:
		inline static InstancePool s_InstancePool;

		template <typename U>
		friend class SharedReference;

		mutable T* m_Instance = nullptr;
	};

}
