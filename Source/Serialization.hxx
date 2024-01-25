#pragma once

#include <cstdint>
#include <fstream>

namespace Endianness
{
    class IsBE
    {
        constexpr static uint32_t U4 = 1;
        constexpr static uint8_t U1 = (const uint8_t&)U4;

    public:
        constexpr static bool Value = U1 == 0;
    };

    template <typename Uint>
    Uint Byteswap(Uint A);

    template <>
    constexpr uint16_t Byteswap(uint16_t const A)
    {
        constexpr uint8_t B[] = {
            8 * (2 - 1)
        };

        auto const X =
            (0x00ffULL & A) << B[0] | (0xff00ULL & A) >> B[0];

        return static_cast<uint16_t>(X);
    }

    template <>
    constexpr uint32_t Byteswap(uint32_t const A)
    {
        constexpr uint8_t B[] = {
            8 * (2 - 1),
            8 * (4 - 1)
        };

        // clang-format off
        auto const X =
            (0x000000ffULL & A) << B[1] |
            (0x0000ff00ULL & A) << B[0] |
            (0x00ff0000ULL & A) >> B[0] |
            (0xff000000ULL & A) >> B[1];
        // clang-format on

        return static_cast<uint32_t>(X);
    }

    template <>
    constexpr uint64_t Byteswap(uint64_t const A)
    {
        constexpr uint8_t B[] = {
            8 * (2 - 1),
            8 * (4 - 1),
            8 * (6 - 1),
            8 * (8 - 1)
        };

        // clang-format off
        auto const X =
            (0x00000000000000ffULL & A) << B[3] |
            (0x000000000000ff00ULL & A) << B[2] |
            (0x0000000000ff0000ULL & A) << B[1] |
            (0x00000000ff000000ULL & A) << B[0] |
            (0x000000ff00000000ULL & A) >> B[0] |
            (0x0000ff0000000000ULL & A) >> B[1] |
            (0x00ff000000000000ULL & A) >> B[2] |
            (0xff00000000000000ULL & A) >> B[3];
        // clang-format on

        return static_cast<uint64_t>(X);
    }

    template <bool bIsBigEndian>
    struct Endianness
    {
    };

    template <typename Uint>
    constexpr Uint HtoBE(Uint const a, Endianness<true>)
    {
        return a;
    }

    template <typename Uint>
    constexpr Uint HtoBE(Uint const a, Endianness<false>)
    {
        return Byteswap(a);
    }

    constexpr inline uint16_t HtoBE16(uint16_t const A)
    {
        return HtoBE(A, Endianness<IsBE::Value>{});
    }

    constexpr inline uint32_t HtoBE32(uint32_t const A)
    {
        return HtoBE(A, Endianness<IsBE::Value>{});
    }

    constexpr inline uint64_t HtoBE64(uint64_t const A)
    {
        return HtoBE(A, Endianness<IsBE::Value>{});
    }
}

namespace Serialization
{
    using namespace Endianness;

    static inline void Write16(std::ofstream& Stream, uint16_t Value)
    {
        uint16_t Temp16{};

        Temp16 = HtoBE16(Value);
        Stream.write(reinterpret_cast<char*>(&Temp16), 4);
    }

    static inline void Write32(std::ofstream& Stream, uint32_t Value)
    {
        uint32_t Temp32{};

        Temp32 = HtoBE32(Value);
        Stream.write(reinterpret_cast<char*>(&Temp32), 4);
    }

    template <typename T>
    static inline void Read16(std::istream& Stream, T& Value)
    {
        char Temp[2]{};

        Stream.read(Temp, 4);
        Value = static_cast<T>(HtoBE16(*reinterpret_cast<uint16_t*>(&Temp[0])));
    }

    template <typename T>
    static inline void Read32(std::istream& Stream, T& Value)
    {
        char Temp[4]{};

        Stream.read(Temp, 4);
        Value = static_cast<T>(HtoBE32(*reinterpret_cast<uint32_t*>(&Temp[0])));
    }

    struct MemoryBuf : std::streambuf
    {
        MemoryBuf(char const* Base, std::size_t Size)
        {
            char* P(const_cast<char*>(Base));
            this->setg(P, P, P + Size);
        }
    };

    struct MemoryStream : virtual MemoryBuf, std::istream
    {
        MemoryStream(char const* Base, std::size_t Size)
            : MemoryBuf(Base, Size)
            , std::istream(static_cast<std::streambuf*>(this))
        {
        }
    };
}
