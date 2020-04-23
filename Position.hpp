#pragma once

#include "BitBoard.hpp"
#include "Color.hpp"
#include "HashTable.hpp"
#include "History.hpp"
#include "Mobility.hpp"
#include "Piece.hpp"
#include "Square.hpp"
#include "ZKey.hpp"

#include <array>
#include <limits>
#include <string>
#include <vector>

namespace spezi
{    
    MilliSquare constexpr LOSS[NumberOfColors] = {-MateValue, MateValue};   
    MilliSquare constexpr DRAW = 0;

    struct EvaluationStatistics
    {
        MilliSquare evaluation;
        int maximumRegularDepth;
        int maximumReachedDepth;
        int64_t numberOfNodes;
        int64_t numberOfQuiescenceNodes;
        float seconds;
    };

    class Position
    {
    public:
        Position() = default;
        Position(std::string fen);

        Position(Position & other) = delete;
        Position(Position && other) = delete;

        std::string getZKey() const;
        std::string getBoardDisplay(int indent = 0) const;
        std::string getPrincipalVariation() const;

        EvaluationStatistics evaluateRecursively(int depth);
        MilliSquare evaluateStatically() const;        

    private:
        void evaluate(int depth);

        bool repetition(int depth);

        bool isAttacked(Color attacking, Square square);

        bool evaluateHashMove(int depth);

        bool evaluateNullMove(int depth);

        bool evaluateCaptures(int depth);

        bool evaluateNonCaptures(int depth);

        bool updateWindowOrCutoff(
            ZKey originalZKey,
            int depth,
            unsigned char originalCastling,
            BitBoard originalEnPassant,
            Square origin, 
            Square target,
            Piece moved,
            Piece captured = KING,
            Piece promoted = KING,            
            unsigned char castlingUpdate = 0xFu);

        BitBoard generateNonCaptureSquares(Piece piece, Square origin) const;

        BitBoard empty = A3|B3|C3|D3|E3|F3|G3|H3|A4|B4|C4|D4|E4|F4|G4|H4
                        |A5|B5|C5|D5|E5|F5|G5|H5|A6|B6|C6|D6|E6|F6|G6|H6;
        BitBoard allPieces[NumberOfColors] =
        {
            A1|B1|C1|D1|E1|F1|G1|H1|A2|B2|C2|D2|E2|F2|G2|H2,    // white
            A8|B8|C8|D8|E8|F8|G8|H8|A7|B7|C7|D7|E7|F7|G7|H7     // black
        };
        BitBoard individualPieces[NumberOfPieceTypes] = 
        {
            A2|B2|C2|D2|E2|F2|G2|H2|A7|B7|C7|D7|E7|F7|G7|H7,    // pawns
            B1|G1|B8|G8,                                        // knights
            C1|F1|C8|F8,                                        // bishops
            A1|H1|A8|H8,                                        // rooks
            D1|D8,                                              // queens
            E1|E8,                                              // kings
        };

        int halfMoves = 0;
        int fullMoves = 0;

        unsigned char castlingRights = 0xF;

        Color sideToMove = WHITE;
        BitBoard enPassant = EMPTY;

        ZKey zKey;

        int maxDepth = 0;
        int nullMoveDepth = 0;

        History history;

        // draft = maxDepth - depth =>
        // draft = -64...63 for depth = 0...maxDepth + MAX_QUIESCENCE_DEPTH at maxDepth = MAX_DEPTH
        // draft will fit into 7bit segment of hash table entry
        static int constexpr MAX_DEPTH = 63;
        static int constexpr MAX_QUIESCENCE_DEPTH = 64;

        std::array<std::array<MilliSquare, MAX_DEPTH + MAX_QUIESCENCE_DEPTH>, NumberOfColors> alphaBetaAtDepth;
        std::array<int64_t, MAX_DEPTH + MAX_QUIESCENCE_DEPTH> numberOfNodesAtDepth;

        HashTable transpositionTable {1 << 22};
    };
}