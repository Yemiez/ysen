#pragma once
#include <ysen/core/RefCounterBase.h>

namespace ysen::core {

	template<typename T>
	class SharedPtr;

	template<typename T2, typename T1>
	static SharedPtr<T2> dynamic_shared_cast(SharedPtr<T1> ptr);

	class SharedPtrDeleter
	{
	public:
		using DeleteFunction = void(void*);

		SharedPtrDeleter() = default;
		SharedPtrDeleter(DeleteFunction* function)
			: m_delete(function)
		{}
		SharedPtrDeleter(const SharedPtrDeleter&) = default;
		SharedPtrDeleter(SharedPtrDeleter&&) = default;

		void operator()(void *ptr) const noexcept
		{
			if (m_delete) {
				m_delete(ptr);
			}
		}

		
		SharedPtrDeleter& operator=(const SharedPtrDeleter&) = default;
		SharedPtrDeleter& operator=(SharedPtrDeleter&&) noexcept = default;

	private:
		DeleteFunction* m_delete{nullptr};
	};

	template<typename T>
	SharedPtrDeleter default_deleter()
	{
		return SharedPtrDeleter{[](void *ptr) {
			delete static_cast<T*>(ptr);
		}};
	}

	inline SharedPtrDeleter null_deleter() { return SharedPtrDeleter{}; }
	
	template<typename T>
	class SharedPtr : public RefCounterBase
	{
	public:
		SharedPtr();
		SharedPtr(const SharedPtr&);
		SharedPtr(SharedPtr&&) noexcept;
		SharedPtr(std::nullptr_t) noexcept;
		SharedPtr(T* ptr, const SharedPtrDeleter&);
		SharedPtr(T* ptr, const RefCounterBase&, const SharedPtrDeleter&);
		~SharedPtr();

		const T* ptr() const { return m_ptr; }
		T* ptr() { return m_ptr; }
		bool operator()() const;
		const T* operator->() const;
		T* operator->();
		const T& operator*() const;
		T& operator*();
		void release();
		const auto& deleter() const { return m_deleter; }

		bool is_null() const noexcept { return m_ptr == nullptr; }

		SharedPtr& operator=(const SharedPtr&);
		SharedPtr& operator=(SharedPtr&&) noexcept;
		SharedPtr& operator=(T* ptr);

		template<typename T1, typename = std::enable_if_t<std::is_convertible_v<T1, T>>>
		SharedPtr& operator=(const SharedPtr<T1>&);
		template<typename T1, typename = std::enable_if_t<std::is_convertible_v<T1, T>>>
		SharedPtr& operator=(SharedPtr<T1>&&);

		explicit operator bool() const noexcept
		{
			return !is_null();
		}

	protected:
		void deallocate_contents();
		
	private:
		T *m_ptr{nullptr};
		SharedPtrDeleter m_deleter;
	};

	template <typename T>
	SharedPtr<T>::SharedPtr()
		: RefCounterBase()
	{}

	template <typename T>
	SharedPtr<T>::SharedPtr(const SharedPtr& other)
		: RefCounterBase(other), m_ptr(other.m_ptr), m_deleter(other.m_deleter)
	{}

	template <typename T>
	SharedPtr<T>::SharedPtr(SharedPtr&& other) noexcept
		: RefCounterBase(std::move(other)), m_ptr(other.m_ptr), m_deleter(std::move(other.m_deleter))
	{
		other.m_ptr = nullptr;
		other.m_deleter = null_deleter();
	}

	template <typename T>
	SharedPtr<T>::SharedPtr(std::nullptr_t) noexcept
		: RefCounterBase(), m_ptr(nullptr), m_deleter(null_deleter())
	{}

	template <typename T>
	SharedPtr<T>::SharedPtr(T* ptr, const SharedPtrDeleter& deleter)
		: RefCounterBase(), m_ptr(ptr), m_deleter(deleter)
	{}

	template <typename T>
	SharedPtr<T>::SharedPtr(T* ptr, const RefCounterBase& other, const SharedPtrDeleter& deleter)
		: RefCounterBase(other), m_ptr(ptr), m_deleter(deleter)
	{}

	template <typename T>
	SharedPtr<T>::~SharedPtr()
	{
		release();
	}

	template <typename T>
	bool SharedPtr<T>::operator()() const
	{
		return m_ptr != nullptr;
	}

	template <typename T>
	const T* SharedPtr<T>::operator->() const
	{
		return m_ptr;
	}

	template <typename T>
	T* SharedPtr<T>::operator->()
	{
		return m_ptr;
	}

	template <typename T>
	const T& SharedPtr<T>::operator*() const
	{
		return *m_ptr;
	}

	template <typename T>
	T& SharedPtr<T>::operator*()
	{
		return *m_ptr;
	}

	template <typename T>
	void SharedPtr<T>::release()
	{
		if (decrement_and_reset()) {
			deallocate_contents();
		}

		m_ptr = nullptr;
		m_deleter = null_deleter();
	}

	template <typename T>
	SharedPtr<T>& SharedPtr<T>::operator=(const SharedPtr& other)
	{
		if (this == &other) {
			return *this;
		}
		
		if (decrement()) {
			deallocate_contents();
		}

		m_ptr = other.m_ptr;
		m_deleter = other.m_deleter;
		ref_copy(other);
		return *this;
	}

	template <typename T>
	SharedPtr<T>& SharedPtr<T>::operator=(SharedPtr&& other) noexcept
	{
		if (this == &other) {
			return *this;
		}
		
		if (decrement()) {
			deallocate_contents();
		}

		m_ptr = nullptr;
		m_deleter = null_deleter();
		std::swap(m_ptr, other.m_ptr);
		std::swap(m_deleter, other.m_deleter);
		ref_reset_to(std::move(other));
		return *this;
	}

	template <typename T>
	SharedPtr<T>& SharedPtr<T>::operator=(T* ptr)
	{
		if (ptr == m_ptr) {
			// Weird?
			return *this;
		}
		
		// Reset ptr
		if (decrement_and_reset()) {
			deallocate_contents();
		}

		m_ptr = ptr;
		m_deleter = default_deleter<T>();
		return *this;
	}

	template <typename T>
	template <typename T1, typename>
	SharedPtr<T>& SharedPtr<T>::operator=(const SharedPtr<T1>& other)
	{
		*this = dynamic_shared_cast<T>(other);
		return *this;
	}

	template <typename T>
	template <typename T1, typename>
	SharedPtr<T>& SharedPtr<T>::operator=(SharedPtr<T1>&& other)
	{
		*this = dynamic_shared_cast<T>(other);
		return *this;
	}

	template <typename T>
	void SharedPtr<T>::deallocate_contents()
	{
		m_deleter(m_ptr);
		m_ptr = nullptr;
		m_deleter = null_deleter();
	}


	template<typename T>
	static SharedPtr<T> adopt_shared(T* ptr)
	{
		return SharedPtr<T>(ptr, default_deleter<T>());
	}

	template<typename T, typename...Args>
	static SharedPtr<T> make_shared(Args&&...args)
	{
		return adopt_shared(new T(std::forward<Args>(args)...));
	}

	template<typename T2, typename T1>
	static SharedPtr<T2> dynamic_shared_cast(SharedPtr<T1> ptr)
	{
		T2* cast_ptr = dynamic_cast<T2*>(ptr.ptr());

		if (cast_ptr == nullptr) {
			return nullptr;
		}

		return SharedPtr<T2>(cast_ptr, std::move(ptr), ptr.deleter());
	}

	template<typename T2, typename T1>
	static SharedPtr<T2> reinterpret_shared_cast(SharedPtr<T1> ptr)
	{
		T2* cast_ptr = reinterpret_cast<T2*>(ptr.ptr());

		if (cast_ptr == nullptr) {
			return nullptr;
		}

		return SharedPtr<T2>(cast_ptr, std::move(ptr), ptr.deleter());
	}

	
}
