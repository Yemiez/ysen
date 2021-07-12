#pragma once
#include <type_traits>
#include <ysen/core/SharedPtr.h>

namespace ysen::core {

	template<typename T>
	class NonnullOwnPtr
	{
	public:
		NonnullOwnPtr() = delete;
		NonnullOwnPtr(const NonnullOwnPtr&) = delete;
		NonnullOwnPtr(std::nullptr_t) = delete;
		NonnullOwnPtr(T* ptr);
		NonnullOwnPtr(NonnullOwnPtr&&) noexcept;
		~NonnullOwnPtr();

		const T* ptr() const { return m_ptr; }
		T* ptr() { return m_ptr; }
		T* release();
		T* operator->();
		const T* operator->() const;
		T& operator*();
		const T& operator*() const;

		SharedPtr<T> as_shared();

		NonnullOwnPtr& operator=(const NonnullOwnPtr&) = delete;
		NonnullOwnPtr& operator=(std::nullptr_t) = delete;
		NonnullOwnPtr& operator=(NonnullOwnPtr&&) noexcept;
		NonnullOwnPtr& operator=(T*);
	private:
		T* m_ptr;
	};

	template <typename T>
	NonnullOwnPtr<T>::NonnullOwnPtr(T* ptr)
		: m_ptr(ptr)
	{}

	template <typename T>
	NonnullOwnPtr<T>::NonnullOwnPtr(NonnullOwnPtr&& other) noexcept
		: m_ptr(other.m_ptr)
	{
		other.m_ptr = nullptr;
	}

	template <typename T>
	NonnullOwnPtr<T>::~NonnullOwnPtr()
	{
		if (m_ptr) {
			delete m_ptr;
			m_ptr = nullptr;	
		}
	}

	template <typename T>
	T* NonnullOwnPtr<T>::release()
	{
		auto tmp = m_ptr;
		m_ptr = nullptr;
		return tmp;
	}

	template <typename T>
	T* NonnullOwnPtr<T>::operator->()
	{
		return m_ptr;
	}

	template <typename T>
	const T* NonnullOwnPtr<T>::operator->() const
	{
		return m_ptr;
	}

	template <typename T>
	T& NonnullOwnPtr<T>::operator*()
	{
		return *m_ptr;
	}

	template <typename T>
	const T& NonnullOwnPtr<T>::operator*() const
	{
		return *m_ptr;
	}

	template <typename T>
	SharedPtr<T> NonnullOwnPtr<T>::as_shared()
	{
		auto tmp = make_shared<T>(m_ptr);
		m_ptr = nullptr;
		return tmp;
	}

	template <typename T>
	NonnullOwnPtr<T>& NonnullOwnPtr<T>::operator=(NonnullOwnPtr&& other) noexcept
	{
		m_ptr = other.m_ptr;
		other.m_ptr = nullptr;
		return *this;
	}

	template <typename T>
	NonnullOwnPtr<T>& NonnullOwnPtr<T>::operator=(T* other)
	{
		m_ptr = other;
		return *this;
	}

	template <typename T>
	static NonnullOwnPtr<T> adopt_nonnull(T *ptr)
	{
		return NonnullOwnPtr<T>(ptr);
	}

	template <typename T, typename...Args>
	static NonnullOwnPtr<T> make_nonnull(Args&& ...args)
	{
		return adopt_nonnull(new T(std::forward<Args>(args)...));
	}

	
}
