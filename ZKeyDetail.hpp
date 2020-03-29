#pragma once

namespace spezi::detail
{
    ZKey constexpr generateZKey(ZKey const previous = 0x42F0E1EBA9EA3693)
    {
        ZKey const p = 0x42F0E1EBA9EA3693;
        return previous & 0x8000000000000000 ? (previous << 1) ^ p : (previous << 1);
    }

    template<int number>
    auto constexpr generateZKeys(ZKey previous = 0x42F0E1EBA9EA3693)
    {
        std::array<ZKey, number> result {};
        for(int i = 0; i < number; ++i)
        {
            result[i] = generateZKey(previous);
            previous = result[i];
        }
        return result;
    }

    auto constexpr pieceKeys(ZKey previous = 0x42F0E1EBA9EA3693)
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