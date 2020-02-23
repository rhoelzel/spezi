#pragma once

#include "Piece.hpp"
#include "Square.hpp"

#include "MobilityDetail.hpp"

#include <array>
#include <cstdint>

namespace spezi
{
    using Mobility = float;
    using MobilityDistribution = std::array<Mobility, detail::MaxBoardPopulation-2>;
    using MobilityArray = std::array<MobilityDistribution, NumberOfSquares>;

    template<Piece piece>
    MobilityArray constexpr AverageMobilities = detail::averageMobilities<piece>(); 

    long toCentiPawns(Mobility const mobility);
    float toPawns(Mobility const mobility);

    void prettyPrint(MobilityArray const & mobilityArray, Square const populatedSquares);
    void prettyPrint(MobilityDistribution const & mobilityDistribution);
}
