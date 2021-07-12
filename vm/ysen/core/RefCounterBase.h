#pragma once
#include <atomic>

namespace ysen::core {
	
	class RefCounterBase
	{
	public:
		RefCounterBase();
		RefCounterBase(const RefCounterBase&);
		RefCounterBase(RefCounterBase&&) noexcept;
		virtual ~RefCounterBase();
	protected:
		void increment();
		bool decrement(); // Returns true if content should be deallocated
		bool decrement_and_reset(); // Returns true if (previous) content should be deallocated

		void ref_reset_to(RefCounterBase&&);
		void ref_copy(const RefCounterBase&);
	private:
		void allocate_ref();
	private:
		std::atomic_uint *m_cnt{nullptr};
	};

	inline RefCounterBase::RefCounterBase()
	{
		allocate_ref();
	}

	inline RefCounterBase::RefCounterBase(const RefCounterBase& other)
		: m_cnt(other.m_cnt)
	{
		increment();
	}

	inline RefCounterBase::RefCounterBase(RefCounterBase&& other) noexcept
		: m_cnt(other.m_cnt)
	{
		other.m_cnt = nullptr;
	}

	inline RefCounterBase::~RefCounterBase() = default;

	inline void RefCounterBase::allocate_ref()
	{
		m_cnt = new std::atomic_uint{1};
	}

	inline void RefCounterBase::increment()
	{
		if (m_cnt) *m_cnt += 1;
	}

	inline bool RefCounterBase::decrement()
	{
		if (!m_cnt) return true;
		*m_cnt -= 1;
		if (*m_cnt == 0) {
			delete m_cnt;
			m_cnt = nullptr;
			return true;
		}
		return false;
	}

	inline bool RefCounterBase::decrement_and_reset()
	{
		auto deallocate = false;
		if (decrement()) {
			deallocate = true;
		}
		allocate_ref();
		return deallocate;
	}

	inline void RefCounterBase::ref_reset_to(RefCounterBase&& other)
	{
		m_cnt = other.m_cnt;
		other.m_cnt = nullptr;
	}

	inline void RefCounterBase::ref_copy(const RefCounterBase& other)
	{
		m_cnt = other.m_cnt;
		increment();
	}

}
