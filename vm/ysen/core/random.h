#pragma once
#include <random>

namespace ysen::core {

	namespace details {
		struct RandomStorage
		{
		private:
			RandomStorage();
		public:
			static RandomStorage& the();
			auto& random_device() { return m_random_device; }
			auto& mersenne_twister() { return m_mersenne_twister; }
		private:
			std::random_device m_random_device;
			std::mt19937 m_mersenne_twister;
		};
		
	}

	template<typename T>
	static T random(T min, T max)
	{
		if constexpr (std::is_floating_point_v<T>) {
			return std::uniform_real_distribution<T>(min, max)(details::RandomStorage::the().mersenne_twister());
		}
		else if constexpr (std::is_integral_v<T>) {
			return std::uniform_int_distribution<T>(min, max)(details::RandomStorage::the().mersenne_twister());
		}
		else {
			static_assert("Cannot pick distribution method for type T");
		}
	}

	template<typename T>
	static T random()
	{
		return random<T>(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
	}

	static void random_reseed(uint32_t seed)
	{
		details::RandomStorage::the().mersenne_twister().seed(seed);
	}

	static void random_reseed()
	{
		random_reseed(details::RandomStorage::the().random_device()());
	}
}