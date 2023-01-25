#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/ReferenceCounting/InstancePool.h"
#include "Vortex/Core/ReferenceCounting/RefCounted.h"

namespace Vortex {

	template <typename T>
	class VORTEX_API SharedRef
	{
	public:
		VX_FORCE_INLINE SharedRef()
			: m_Instance(nullptr) { }

		VX_FORCE_INLINE SharedRef(std::nullptr_t)
			: m_Instance(nullptr) { }

		VX_FORCE_INLINE SharedRef(T* instance)
			: m_Instance(instance)
		{
			//static_assert(std::is_base_of<RefCounted, T>::value, "Class is not reference counted!");
			IncRef();
		}

		template <typename U>
		VX_FORCE_INLINE SharedRef(const SharedRef<U>& other)
		{
			m_Instance = (T*)other.m_Instance;
			IncRef();
		}

		template <typename U>
		VX_FORCE_INLINE SharedRef(SharedRef<U>&& other)
		{
			m_Instance = (T*)other.m_Instance;
			other.m_Instance = nullptr;
		}

		VX_FORCE_INLINE SharedRef(const SharedRef<T>& other)
			: m_Instance(other.m_Instance)
		{
			IncRef();
		}

		VX_FORCE_INLINE ~SharedRef()
		{
			DecRef();
		}

	public:
		VX_FORCE_INLINE void Reset(T* instance = nullptr)
		{
			DecRef();
			m_Instance = instance;
		}

		template <typename U>
		VX_FORCE_INLINE SharedRef<U> As() const
		{
			return SharedRef<U>(*this);
		}

		VX_FORCE_INLINE void Swap(const SharedRef<T>& other)
		{
			T* temp = m_Instance;
			m_Instance = other.m_Instance;
			other.m_Instance = temp;
			temp = nullptr;
		}

		VX_FORCE_INLINE bool EqualsObject(const SharedRef<T>& other)
		{
			if (!m_Instance || !other.m_Instance)
				return false;

			return *m_Instance == *other.m_Instance;
		}

	public:
		template <typename... Args>
		VX_FORCE_INLINE static SharedRef<T> Create(Args&&... args)
		{
			return SharedRef<T>(new T(std::forward<Args>(args)...));
		}

		VX_FORCE_INLINE static SharedRef<T> CopyWithoutIncrement(const SharedRef<T>& other)
		{
			SharedRef<T> result = nullptr;
			result->m_Instance = other.m_Instance;
			return result;
		}

	public:
		VX_FORCE_INLINE SharedRef& operator=(std::nullptr_t)
		{
			DecRef();
			m_Instance = nullptr;
			return *this;
		}

		VX_FORCE_INLINE SharedRef& operator=(const SharedRef<T>& other)
		{
			other.IncRef();
			DecRef();

			m_Instance = other.m_Instance;
			return *this;
		}

		template <typename U>
		VX_FORCE_INLINE SharedRef& operator=(const SharedRef<T>& other)
		{
			other.IncRef();
			DecRef();

			m_Instance = (T*)other.m_Instance;
			return *this;
		}

		template <typename U>
		VX_FORCE_INLINE SharedRef& operator=(SharedRef<U>&& other)
		{
			DecRef();

			m_Instance = (T*)other.m_Instance;
			other.m_Instance = nullptr;
			return *this;
		}

		VX_FORCE_INLINE operator bool() { return m_Instance != nullptr; }
		VX_FORCE_INLINE operator bool() const { return m_Instance != nullptr; }

		VX_FORCE_INLINE T* operator->() { return m_Instance; }
		VX_FORCE_INLINE const T* operator->() const { return m_Instance; }

		VX_FORCE_INLINE T& operator*() { return *m_Instance; }
		VX_FORCE_INLINE const T& operator*() const { return *m_Instance; }

		VX_FORCE_INLINE T* Raw() { return  m_Instance; }
		VX_FORCE_INLINE const T* Raw() const { return  m_Instance; }

		VX_FORCE_INLINE bool operator==(const SharedRef<T>& other) const
		{
			return m_Instance == other.m_Instance;
		}

		VX_FORCE_INLINE bool operator!=(const SharedRef<T>& other) const
		{
			return !(*this == other);
		}

	private:
		VX_FORCE_INLINE void IncRef() const
		{
			if (m_Instance)
			{
				m_Instance->IncRefCount();
				s_InstancePool.AddInstance((void*)m_Instance);
			}
		}

		VX_FORCE_INLINE void DecRef() const
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
		friend class SharedRef;

		mutable T* m_Instance = nullptr;
	};

}
