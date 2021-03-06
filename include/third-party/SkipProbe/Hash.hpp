#pragma once

#include <string>
#if __cplusplus >= 201703L
#include <string_view>
#endif
#include <memory>
#include <vector>
#include <limits>
#include "Murmur.hpp"
#include "CityHash.hpp"

namespace SkipProbe
{
	template<typename t_Hashable>
	class Hash;

	namespace detail_
	{
		template<typename t_Hashable, typename t_Enable = void>
		class HashImpl;
	}
#if defined(_WIN64) || defined(_M_X64) || defined(__x86_64__)
	constexpr size_t c_fnvOffset = 14695981039346656037ULL;
	constexpr size_t c_fnvPrime = 1099511628211ULL;
#else // defined(_WIN64)
	constexpr size_t c_fnvOffset = 2166136261U;
	constexpr size_t c_fnvPrime = 16777619U;
#endif // defined(_WIN64)
}

template<typename t_Hashable>
class SkipProbe::Hash
{
public:
	typedef typename std::remove_const<typename std::remove_reference<t_Hashable>::type>::type adjusted_type;

	size_t operator()(t_Hashable const& value) const noexcept
	{
		return detail_::HashImpl<adjusted_type>::Compute(value);
	}
};

template <typename t_Hashable, typename t_Enable>
class SkipProbe::detail_::HashImpl
{
public:
	inline static size_t Compute(t_Hashable const& value) noexcept
	{
		return std::hash<t_Hashable>()(value);
	}
};

// STL Strings

template <>
class SkipProbe::detail_::HashImpl<std::string>
{
public:
	inline static size_t Compute(std::string const& value) noexcept
	{
		return CityHash(value.data(), value.size());
	}
};

template <>
class SkipProbe::detail_::HashImpl<std::wstring>
{
public:
	inline static size_t Compute(std::wstring const& value) noexcept
	{
		return CityHash(value.data(), value.size());
	}
};

template <>
class SkipProbe::detail_::HashImpl<std::u16string>
{
public:
	inline static size_t Compute(std::u16string const& value) noexcept
	{
		return CityHash(value.data(), value.size());
	}
};

template <>
class SkipProbe::detail_::HashImpl<std::u32string>
{
public:
	inline static size_t Compute(std::u32string const& value) noexcept
	{
		return CityHash(value.data(), value.size());
	}
};

#if __cplusplus >= 201703L
template <>
class SkipProbe::detail_::HashImpl<std::string_view>
{
public:
	inline static size_t Compute(std::string_view const& value) noexcept
	{
		return CityHash(value.data(), value.size());
	}
};

template <>
class SkipProbe::detail_::HashImpl<std::wstring_view>
{
public:
	inline static size_t Compute(std::wstring_view const& value) noexcept
	{
		return CityHash(value.data(), value.size());
	}
};

template <>
class SkipProbe::detail_::HashImpl<std::u16string_view>
{
public:
	inline static size_t Compute(std::u16string_view const& value) noexcept
	{
		return CityHash(value.data(), value.size());
	}
};

template <>
class SkipProbe::detail_::HashImpl<std::u32string_view>
{
public:
	inline static size_t Compute(std::u32string_view const& value) noexcept
	{
		return CityHash(value.data(), value.size());
	}
};
#endif

// Pointer types

template <typename t_Hashable>
class SkipProbe::detail_::HashImpl<t_Hashable*>
{
public:
	inline static size_t Compute(t_Hashable* const& value) noexcept
	{
		return HashImpl<uintptr_t>::Compute(reinterpret_cast<uintptr_t>(value));
	}
};

template <typename t_PtrType>
class SkipProbe::detail_::HashImpl<std::shared_ptr<t_PtrType>>
{
public:
	inline static size_t Compute(std::shared_ptr<t_PtrType> const& value) noexcept
	{
		return HashImpl<uintptr_t>::Compute(reinterpret_cast<uintptr_t>(value.get()));
	}
};

template <typename t_PtrType>
class SkipProbe::detail_::HashImpl<std::unique_ptr<t_PtrType>>
{
public:
	inline static size_t Compute(std::unique_ptr<t_PtrType> const& value) noexcept
	{
		return HashImpl<uintptr_t>::Compute(reinterpret_cast<uintptr_t>(value.get()));
	}
};

// Integer types

template<typename t_IntegerType>
class SkipProbe::detail_::HashImpl<t_IntegerType, typename std::enable_if<std::is_integral<t_IntegerType>::value && sizeof(t_IntegerType) <= 4>::type>
{
public:
	inline static size_t Compute(t_IntegerType const& value) noexcept
	{
		// 32-bit integers use FNV rather than Murmur3 integer finisher due to superior performance characteristics.
		size_t result = c_fnvOffset;
		unsigned char const* const c = reinterpret_cast<unsigned char const* const>(&value);
		for (size_t i = 0; i < sizeof(value); ++i)
		{
			result ^= static_cast<size_t>(c[i]);
			result *= c_fnvPrime;
		}

		return size_t(result);
	}
};

template<typename t_IntegerType>
class SkipProbe::detail_::HashImpl<t_IntegerType, typename std::enable_if<std::is_integral<t_IntegerType>::value && sizeof(t_IntegerType) == 8>::type>
{
public:
	inline static size_t Compute(t_IntegerType const& value) noexcept
	{
#if defined(_WIN64) || defined(_M_X64) || defined(__x86_64__)
		return Murmur3::HashInt64(uint64_t(value));
#else
		return Murmur3::HashInt64(uint64_t(value)) % std::numeric_limits<size_t>::max();
#endif
	}
};

// Floating point types

template <>
class SkipProbe::detail_::HashImpl<float>
{
public:
	inline static size_t Compute(float const& value) noexcept
	{
		union { uint32_t i; float f; } convert;
		convert.f = value;
		return HashImpl<uint32_t>::Compute(convert.i);
	}
};

template <>
class SkipProbe::detail_::HashImpl<double>
{
public:
	inline static size_t Compute(double const& value) noexcept
	{
		union { uint64_t i; double f; } convert;
		convert.f = value;
		return HashImpl<uint64_t>::Compute(convert.i);
	}
};

template <>
class SkipProbe::detail_::HashImpl<long double>
{
public:
	inline static size_t Compute(double const& value) noexcept
	{
		return CityHash(&value, sizeof(value));
	}
};