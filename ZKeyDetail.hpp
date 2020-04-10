#pragma once

namespace spezi::detail
{
    namespace
    {
        //ZKey constexpr seed = 0x42F0E1EBA9EA3693; // 925
        //ZKey constexpr seed = 0xABCDEFFEDCBA9875; // 925 
        ZKey constexpr seed =   0x123456789ABCDEF1; // 752
    }

    ZKey constexpr generateZKey(ZKey const previous = seed)
    {
        ZKey const p = seed;
        return previous & 0x8000000000000000 ? (previous << 1) ^ p : (previous << 1);
    }

    template<int number>
    auto constexpr generateZKeys(ZKey previous = seed)
    {
        std::array<ZKey, number> result {};
        for(int i = 0; i < number; ++i)
        {
            result[i] = generateZKey(previous);
            previous = result[i];
        }
        return result;
    }

    auto constexpr pieceKeys(ZKey previous = seed)
    {
        std::array<std::array<std::array<ZKey, NumberOfSquares>, NumberOfPieceTypes>, NumberOfColors> result {};
        for(int color = 0; color < NumberOfColors; ++color)
        {
            for(int piece = 0; piece < NumberOfPieceTypes; ++piece)
            {
                for(int square = 0; square < NumberOfSquares; ++square)
                {
                    result[color][piece][square] = generateZKey(previous);
                    previous = result[color][piece][square];
                }
            }
        }
        return result;
    }
}