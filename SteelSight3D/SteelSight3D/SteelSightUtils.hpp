#pragma once
#include <functional>
#include "unordered_dense.h"

namespace Voortman {
	/// <summary>
	/// Hashcombine function to hash a vertex. Please note that the GLM library is experimental with hashing.
	/// </summary>
	/// <typeparam name="T">Type that needs to be hashed</typeparam>
	/// <typeparam name="...Rest">The next types</typeparam>
	/// <param name="seed">The seed.</param>
	/// <param name="v">Type T value</param>
	/// <param name="...rest">Type Rest value</param>
	template <typename T, typename... Rest>
	inline void hashCombine(std::size_t& seed, const T& v, const Rest&... rest) noexcept {
		seed ^= ankerl::unordered_dense::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		(hashCombine(seed, rest), ...);
	};
}