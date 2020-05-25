#pragma once

#include "HashTable.hpp"

#include <map>

namespace spezi
{  
    std::map<ZKey, std::array<std::string, 4>> const OpeningBook 
    {   
        { 
            0x1F16291AE0EBD6E5, // initial position
            {
                "e2e4", "d2d4", "c2c4", "g1f3"
            }
        },
        {
            0xC269EF7CE049CD1C, // 1. e4
            {
                "e7e5", "c7c5", "c7c6" ,"e7e6"
            }
        },
        {
            0xED2BC3B89A1EB4ED, // 1. d4
            {
                "d7d5", "g8f6", "e7e6", "c7c5"
            }
        },
        {
            0x5F2DD832E484639B, // 1. c4
            {
                "c7c5", "e7e5", "g8f6", "d7d6"
            }
        },
        {
            0x4EAB25F04EB9A5F3, // 1. Nf3
            {
                "g8f6", "c7c5", "e7e6", "g7g6"
            }
        }
    };
}