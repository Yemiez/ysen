#pragma once

namespace ysen::core {

	namespace details {
		static constexpr auto PRIME = 0x01000193u;
		static constexpr auto SEED = 0x811C9DC5u;
	}

	// ReSharper disable once CppInconsistentNaming
	inline size_t fnv1a(const unsigned char* ptr, size_t length)
	{
		auto hash = details::SEED;
		const auto *end = ptr + length;
		while (ptr < end) {
			hash = (*ptr++ ^ hash) * details::PRIME;
		}
		return hash;
	}

	template<typename T>
	// ReSharper disable once CppInconsistentNaming
	size_t fnv1a_trivial(T t)
	{
		return fnv1a(reinterpret_cast<const unsigned char*>(&t), sizeof(T));
	}
}