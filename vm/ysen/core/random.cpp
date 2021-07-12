#include "random.h"

#include "SharedPtr.h"

ysen::core::details::RandomStorage::RandomStorage()
	: m_random_device(), m_mersenne_twister(m_random_device())
{
}

ysen::core::details::RandomStorage& ysen::core::details::RandomStorage::the()
{
	static RandomStorage storage{};
	return storage;
}
