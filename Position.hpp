#pragma once

#include "BitBoard.hpp"
#include "Color.hpp"
#include "HashTable.hpp"
#include "Mobility.hpp"
#include "Piece.hpp"
#include "Square.hpp"
#include "TimeManagement.hpp"
#include "ZKey.hpp"

#include <array>
#include <atomic>
#include <chrono>
#include <functional>
#include <limits>
#include <string>
#include <vector>

namespace spezi
{    
    MilliSquare constexpr LOSS[NumberOfColors] = {-MateValue, MateValue};   
    MilliSquare constexpr DRAW = 0;

    struct EvaluationParameters
    {
        int wtime = -1;
        int btime = -1;
        int winc = 0;
        int binc = 0;
        int movestogo = -1;
        int depth = -1;
        int nodes = -1;
        int mate = -1;
        int movetime = -1;
    };

    struct EvaluationStatistics
    {
        MilliSquare evaluation;
        int maximumRegularDepth;
        int maximumReachedDepth;
        int64_t numberOfNodes;
        int64_t numberOfQuiescenceNodes;
        float seconds;
    };

    constexpr char startingFen[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    class Position
    {
    public:
        Position() = delete;
        Position(std::string fen, std::function<void(std::string)> outputFunction);
        Position(Position & other) = delete;
        Position(Position && other) = delete;

        void setFen(std::string fen);
        void makeMoves(std::vector<std::string>::const_iterator begin,
                                std::vector<std::string>::const_iterator end);
        void setHashTableSize(unsigned int megaByte);
        void setMaxNumberOfNullMoves(unsigned int maxNumberOfNullMoves);
        void setMaxQuiescenceDepth(unsigned int quiescenceDepth);
        void setSuppressPv(bool suppressPv);
        void clearHashTable();
        void interrupt();

        std::string getZKey() const;
        std::string getBoardDisplay(int indent = 0) const;
        std::string getPrincipalVariation() const;
        std::string getPrincipalVariationII() const;

        EvaluationStatistics evaluateRecursively(EvaluationParameters const & parameters);
        MilliSquare evaluateStatically() const;       
        MilliSquare pawnUnitsOnBoard() const; 

    private:
        void evaluate(int depth);

        bool repetition();

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

        void storePrincipalVariation(HashEntry hashEntry, int depth);

        bool checkAbortingConditions();

        void sendInfo();

        void makeMove(std::string const & uciNotation);

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
        int maxQuiescenceDepth = 8;
        int nullMoveDepth = 0;

        int nullMovesOnBranch = 0;

        int maxConsecutiveNullMoves = 1;        

        int alphaRaises = 0;
        int betaCutoffs = 0;
        int cutEntries = 0;
        int allEntries = 0;
        int exactEntries = 0;
        int cutHashes = 0;
        int allHashes = 0;
        int exactHashes = 0;
        int hashCutoffs = 0;
        int nullMoveCutoffs = 0;

        int pvEntries = 0;
        //int pvMisses = 0;

        // draft = maxDepth - depth =>
        // draft = 63...-64 for depth = 0...MAX_DEPTH + MAX_QUIESCENCE_DEPTH
        // draft will fit into 7bit segment of hash table entry
        static int constexpr MAX_DEPTH = 63;                    
        static int constexpr MAX_QUIESCENCE_DEPTH = 64;         
        
        static int constexpr MAX_DEPTH_ARRAY_SIZE = MAX_DEPTH + MAX_QUIESCENCE_DEPTH + 1;
        std::array<std::array<MilliSquare, MAX_DEPTH_ARRAY_SIZE>, NumberOfColors> alphaBetaAtDepth;
        std::array<int64_t, MAX_DEPTH_ARRAY_SIZE> numberOfNodesAtDepth;
        std::array<HashEntry, MAX_DEPTH_ARRAY_SIZE> hashEntryAtDepth;

        static int constexpr PRINCIPAL_VARIATION_ARRAY_SIZE = (MAX_DEPTH_ARRAY_SIZE * (MAX_DEPTH_ARRAY_SIZE + 1)) / 2;
        std::array<HashEntry, PRINCIPAL_VARIATION_ARRAY_SIZE> principalVariation;
        bool suppressFaultyPv {false};

        static int constexpr MB = 1 << 20;
        HashTable transpositionTable {MB * 1};
        PrincipalVariationTable principalVariationTable {1024, 8};

        static int constexpr HISTORY_SIZE = 1024;
        std::array<ZKey, HISTORY_SIZE> history;

        EvaluationParameters evaluationParameters;

        TimePoint evaluationTargetTimePoint;

        static MilliSeconds constexpr INTERRUPT_INTERVAL {10}; 
      
        enum InterruptState
        {
            Idle,
            Busy,
            Interrupted
        };

        std::atomic<InterruptState> interruptState {Idle};

        static MilliSeconds constexpr INFO_INTERVAL {1000};
        TimePoint lastInfoSentTimePoint;

        std::function<void(std::string)> engineToGuiOutputFunction;
    };
}