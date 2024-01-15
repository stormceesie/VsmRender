#pragma once
#include <functional>

namespace Voortman {

	// Hashing GLM types is experimental according to the documentation
	template <typename T, typename... Rest>
	inline void hashCombine(std::size_t& seed, const T& v, const Rest&... rest) noexcept {
		seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		(hashCombine(seed, rest), ...);
	};
}