#include "BitBoard.hpp"

#include <x86intrin.h>

namespace spezi
{
    BitBoard pdep(BitBoard const source, BitBoard const mask)
    {
        return _pdep_u64(source, mask);
    }

    BitBoard pext(BitBoard const source, BitBoard const mask)
    {
        return _pext_u64(source, mask);
    }
}