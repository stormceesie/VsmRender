#pragma once
#include <functional>
#include "unordered_dense.h"

namespace Voortman {
	// Hashing GLM types is experimental according to the documentation
	// Different hash functions can influence the performance
	template <typename T, typename... Rest>
	inline void hashCombine(std::size_t& seed, const T& v, const Rest&... rest) noexcept {
		seed ^= ankerl::unordered_dense::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		(hashCombine(seed, rest), ...);
	};
}