#pragma once

#include "ZKey.hpp"

#include <vector>

namespace spezi
{
    struct HistoryNode
    {
        ZKey zKey;
    };

    using History = std::vector<HistoryNode>; 
}