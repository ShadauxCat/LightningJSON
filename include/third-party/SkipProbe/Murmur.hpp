#pragma once
#include <time.h>
#include <stdint.h>

namespace SkipProbe
{
	namespace Murmur3
	{
#if !defined(_WIN64) && !defined(_M_X64) && !defined(__x86_64__)
		namespace detail_
		{
			static inline int32_t RotateLeft(const uint32_t x, const uint8_t r)
			{
				return (x << r) | (x >> (32 - r));
			}
		}
		inline size_t Hash(const void* const data, const size_t size, const size_t seed)
		{
			const uint32_t c1 = 0xcc9e2d51;
			const uint32_t c2 = 0x1b873593;
			const uint8_t r1 = 15;
			const uint8_t r2 = 13;
			const uint32_t m = 5;
			const uint32_t n = 0xe6546b64;

			const uint8_t* buffer = (const uint8_t*)data;
			const uint32_t remainder = size % sizeof(uint32_t);
			const uint32_t numChunks = (size - remainder) / sizeof(uint32_t);

			uint32_t outputHash = seed;

			for (uint32_t i = 0; i < numChunks; ++i, buffer += sizeof(uint32_t))
			{
				uint32_t k = *((uint32_t*)buffer);

				k *= c1;
				k = detail_::RotateLeft(k, r1);
				k *= c2;

				outputHash ^= k;
				outputHash = detail_::RotateLeft(outputHash, r2);
				outputHash = (outputHash * m) + n;
			}

			if (remainder > 0)
			{
				uint32_t finalChunk = 0;

				// Intentional fallthrough cases to calculate the final chunk value.
				switch (remainder)
				{
				case 3: finalChunk |= uint32_t(*buffer); finalChunk <<= 8; ++buffer;
				case 2: finalChunk |= uint32_t(*buffer); finalChunk <<= 8; ++buffer;
				case 1: finalChunk |= uint32_t(*buffer); ++buffer;
				}

				finalChunk *= c1;
				finalChunk = detail_::RotateLeft(finalChunk, r1);
				finalChunk *= c2;

				outputHash ^= finalChunk;
			}

			outputHash ^= uint32_t(size);
			outputHash ^= (outputHash >> 16);
			outputHash *= 0x85ebca6b;
			outputHash ^= (outputHash >> 13);
			outputHash *= 0xc2b2ae35;
			outputHash ^= (outputHash >> 16);

			return outputHash;
		}
#else
		namespace detail_
		{
			static inline int64_t RotateLeft(const uint64_t x, const uint8_t r)
			{
				return (x << r) | (x >> (64 - r));
			}
		}
		inline size_t Hash(const void* const data, const size_t size, const size_t seed)
		{
			const uint64_t c1 = 0x87c37b91114253d5ULL;
			const uint64_t c2 = 0x4cf5ad432745937fULL;
			const uint8_t r1 = 15;
			const uint8_t r2 = 13;
			const uint64_t m = 5;
			const uint64_t n = 0x52dce72938495ab5ULL;

			const uint8_t* buffer = (const uint8_t*)data;
			const uint64_t remainder = size % sizeof(uint64_t);
			const uint64_t numChunks = (size - remainder) / sizeof(uint64_t);

			uint64_t outputHash = seed;

			for (uint64_t i = 0; i < numChunks; ++i, buffer += sizeof(uint64_t))
			{
				uint64_t k = *((uint64_t*)buffer);

				k *= c1;
				k = detail_::RotateLeft(k, r1);
				k *= c2;

				outputHash ^= k;
				outputHash = detail_::RotateLeft(outputHash, r2);
				outputHash = (outputHash * m) + n;
			}

			if (remainder > 0)
			{
				uint64_t finalChunk = 0;

				// Intentional fallthrough cases to calculate the final chunk value.
				switch (remainder)
				{
				case 7: finalChunk |= uint64_t(*buffer); finalChunk <<= 8; ++buffer;
				case 6: finalChunk |= uint64_t(*buffer); finalChunk <<= 8; ++buffer;
				case 5: finalChunk |= uint64_t(*buffer); finalChunk <<= 8; ++buffer;
				case 4: finalChunk |= uint64_t(*buffer); finalChunk <<= 8; ++buffer;
				case 3: finalChunk |= uint64_t(*buffer); finalChunk <<= 8; ++buffer;
				case 2: finalChunk |= uint64_t(*buffer); finalChunk <<= 8; ++buffer;
				case 1: finalChunk |= uint64_t(*buffer); ++buffer;
				}

				finalChunk *= c1;
				finalChunk = detail_::RotateLeft(finalChunk, r1);
				finalChunk *= c2;

				outputHash ^= finalChunk;
			}

			outputHash ^= uint64_t(size);
			outputHash ^= (outputHash >> 33);
			outputHash *= 0xff51afd7ed558ccdULL;
			outputHash ^= (outputHash >> 33);
			outputHash *= 0xc4ceb9fe1a85ec53ULL;
			outputHash ^= (outputHash >> 33);

			return outputHash;
		}
#endif
		inline size_t Hash(const void* const data, const size_t size)
		{
			static size_t seed = size_t(time(nullptr));
			return Hash(data, size, seed);
		}

		inline uint32_t HashInt32(uint32_t data)
		{
			data ^= (data >> 16);
			data *= 0x85ebca6b;
			data ^= (data >> 13);
			data *= 0xc2b2ae35;
			data ^= (data >> 16);
			return data;
		}

		inline uint64_t HashInt64(uint64_t data)
		{
			data ^= (data >> 33);
			data *= 0xff51afd7ed558ccdULL;
			data ^= (data >> 33);
			data *= 0xc4ceb9fe1a85ec53ULL;
			data ^= (data >> 33);
			return data;
		}

		inline size_t HashPointer(intptr_t data)
		{
#if defined(_WIN64) || defined(_M_X64) || defined(__x86_64__)
			return HashInt64(data);
#else
			return HashInt32(data);
#endif
		}
	}
}
