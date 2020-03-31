#pragma once

#include "ZKey.hpp"

#include <vector>

namespace spezi
{
    struct HistoryNode
    {
        ZKey zKey;
        int halfMoves;
    };

    using History = std::vector<HistoryNode>; 
}