#pragma once

#include <cstdint>
#include <string>
#include <vector>

#define xi(x) x

using FNV1A_t = std::uint32_t;

/*
 * 32-BIT FNV1A HASH
 * use fnv1a instead crc32 with randomly access ~1kb much proper
 * @credits: underscorediscovery
 */
namespace FNV1A
{
	/* fnv1a constants */
	constexpr std::uint32_t ullBasis = 0x811C9DC5;
	constexpr std::uint32_t ullPrime = 0x1000193;

	// create compile time hash
	constexpr FNV1A_t HashConst(const char* szString, const FNV1A_t uValue = ullBasis) noexcept {
		return !*szString ? uValue : HashConst(&szString[1], (uValue ^ FNV1A_t(szString[0])) * ullPrime);
	}

	constexpr FNV1A_t HashConst(const wchar_t* szString, const FNV1A_t uValue = ullBasis) noexcept
	{
		return !*szString ? uValue : HashConst(&szString[1], (uValue ^ FNV1A_t(szString[0])) * ullPrime);
	}

	// create runtime hash
	__forceinline const FNV1A_t Hash(const char* szString)
	{
		FNV1A_t uHashed = xi(ullBasis);

		for (size_t i = xi(0U); i < strlen(szString); ++i)
		{

			uHashed ^= szString[i];
			uHashed *= xi(ullPrime);

		}

		return uHashed;
	}

	template <std::size_t N>
	struct CompileTimeHasher {
		constexpr CompileTimeHasher(const char(&str)[N]) : value(HashConst(str)) {}
		const FNV1A_t value;
	};

}

namespace enc
{

	__forceinline std::string encrypt(const std::string& in, bool reverse = false)
	{

		std::string ret_str = in;

		for (auto i = 0; i < ret_str.length(); i++)
		{

			auto key = reverse ? (i + (ret_str.length() + i)) : (i + (ret_str.length() - i));
			ret_str[i] ^= key;

		}

		return ret_str;

	}

	__forceinline std::string decrypt(const std::string& in, bool reverse = false)
	{

		std::string ret_str = in;

		for (auto i = 0; i < ret_str.length(); i++)
		{

			auto key = reverse ? (i + (ret_str.length() + i)) : (i + (ret_str.length() - i));
			ret_str[i] = ret_str[i] ^ key;

		}

		return ret_str;

	}

}

#define HASH(str) FNV1A::HashConst(str)
#define RUNTIME_HASH(x) (FNV1A::Hash(x))