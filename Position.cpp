#include "Position.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <iostream>

#define PERFT

namespace spezi
{
    namespace
    {
        auto constexpr NO_PIECE = NumberOfPieceTypes * 2;
        auto constexpr PIECE_ERROR = NumberOfPieceTypes * 2 + 1;
        
        auto constexpr pieceBoard(
            BitBoard const empty,
            BitBoard const (&allPieces)[NumberOfColors],
            BitBoard const (&individualPieces)[NumberOfPieceTypes])
        {
            std::array<int, NumberOfSquares> retval {};
            
            for(auto const square : SQUARES)
            {
                if(!square)
                {
                    continue;
                }

                auto & result = retval[ffs(square)];
                result = NO_PIECE;    
                int pieces = 0;

                for(auto const piece : {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING })
                {
                    if(individualPieces[piece] & square)
                    {
                        if(allPieces[WHITE] & square)
                        {
                            result = piece;
                        }
                        else
                        {
                            result = NumberOfPieceTypes + piece;
                        }
                        
                        ++ pieces;
                    }
                }

                switch(pieces)
                {
                case 0:
                    if((allPieces[WHITE]
                        | allPieces[BLACK]
                        | ~empty) & square)
                        {
                            result = PIECE_ERROR;
                        }
                    break;
                case 1:
                    if(!((allPieces[WHITE]
                        ^ allPieces[BLACK])
                        & ~empty & square))
                        {
                            result = PIECE_ERROR;
                        }
                    break;
                default:
                    result = PIECE_ERROR;
                    break;
                }
            }
            return retval;
        }   

        ZKey constexpr zKeyFromPieceBoard(std::array<int, NumberOfSquares> pieceBoard)
        {
            ZKey result {0};
            for(Square square = 0; square < NumberOfSquares; ++square)
            {
                if(pieceBoard[square] == NO_PIECE)
                {
                    continue;
                }
                auto const color = pieceBoard[square] / NumberOfPieceTypes;
                auto const piece = pieceBoard[square] % NumberOfPieceTypes;
                result ^= PieceKeys[color][piece][square];
            }
            return result;
        }

        template<Color color, Piece piece>
        static inline MilliSquare staticPieceEvaluation(BitBoard pieces, int const p)
        {
            MilliSquare value = 0;
            while(pieces)
            {   
                value += StaticMobilities<color, piece>[ffs(pieces)][p];
                pieces &= pieces - 1;
            }
            return value;
        }

        static inline char castlingCaptureUpdateFlags(BitBoard const from, BitBoard const to)
        {
            auto const wKing = popcount(E1 & from);
            auto const bKing = popcount(E8 & from);
            auto const K = popcount(H1 & (from | to)) | wKing;
            auto const Q = popcount(A1 & (from | to)) | wKing;
            auto const k = popcount(H8 & (from | to)) | bKing;
            auto const q = popcount(A8 & (from | to)) | bKing;
            return 0xF & ~(K | (Q << 1) | (k << 2) | (q << 3));
        }
    } 

    Position::Position(std::string fen)
    {
        std::istringstream sectionStream(fen);
        std::array<std::string, 6> sections;
        for(auto & section : sections)
        {
            if(!(sectionStream>>section))
            {
                throw std::runtime_error("Invalid FEN notation: '" + fen + "'");
            }
        }

        individualPieces[PAWN]=individualPieces[KNIGHT]=
            individualPieces[BISHOP]=individualPieces[ROOK]=
            individualPieces[QUEEN]=individualPieces[KING]=
            allPieces[WHITE] = allPieces[BLACK] = EMPTY;
        
        std::replace(sections[0].begin(), sections[0].end(), '/', ' ');
        std::istringstream rankStream(sections[0]);
        std::array<std::string, SquaresPerFile> ranks;
        int rankIndex = SquaresPerFile; 
        for(auto & rank : ranks)
        {
            --rankIndex;
            if(!(rankStream>>rank))
            {
                throw std::runtime_error("Invalid piece placement in FEN: '" + fen + "'");
            }
            int file = 0;
            char const * next = rank.data();
            while(file < 8)
            {
                BitBoard s = A1 << (rankIndex * SquaresPerRank + file);
                if(*next > 0x30 && *next < 0x39) { file += *next - 0x31; } 
                else if(*next == 'P') { individualPieces[PAWN] ^= s; allPieces[WHITE] ^= s; }
                else if(*next == 'N') { individualPieces[KNIGHT] ^= s; allPieces[WHITE] ^= s; }
                else if(*next == 'B') { individualPieces[BISHOP] ^= s; allPieces[WHITE] ^= s; }
                else if(*next == 'R') { individualPieces[ROOK] ^= s; allPieces[WHITE] ^= s; }
                else if(*next == 'Q') { individualPieces[QUEEN] ^= s; allPieces[WHITE] ^= s; }
                else if(*next == 'K') { individualPieces[KING] ^= s; allPieces[WHITE] ^= s; }
                else if(*next == 'p') { individualPieces[PAWN] ^= s; allPieces[BLACK] ^= s; }
                else if(*next == 'n') { individualPieces[KNIGHT] ^= s; allPieces[BLACK] ^= s; }
                else if(*next == 'b') { individualPieces[BISHOP] ^= s; allPieces[BLACK] ^= s; }
                else if(*next == 'r') { individualPieces[ROOK] ^= s; allPieces[BLACK] ^= s; }
                else if(*next == 'q') { individualPieces[QUEEN] ^= s; allPieces[BLACK] ^= s; }
                else if(*next == 'k') { individualPieces[KING] ^= s; allPieces[BLACK] ^= s; }
                else { throw std::runtime_error("Invalid rank in FEN: '" + rank + "'");}
                ++next;
                ++file;
            }
        }

        empty = ~(allPieces[WHITE] | allPieces[BLACK]);

        if(sections[1] == "w")
        {
            sideToMove = WHITE;
        }
        else if(sections[1] == "b")
        {
            sideToMove = BLACK;
        }
        else
        {
            throw std::runtime_error("Invalid side-to-move character: " + sections[1]);
        }

        std::string allowedCastling[] =
        { 
            "-", "K", "Q", "k", "q", "KQ", "Kk", "Kq", "Qk", "Qq", "kq",
            "KQk", "KQq", "Kkq", "Qkq", "KQkq"
        }; 

        bool castlingError = true;
        for(auto const s : allowedCastling)
        {
            if(s == sections[2])
            {
                castlingError = false;
                break;
            }
        }
        if(castlingError)
        { 
            throw std::runtime_error("Invalid castling rights: " + sections[2]);
        }

        if(sections[2][0] != 'K')
        {
            castlingRights &= ~1;
        }
        if(sections[2][0] != 'Q' && sections[2][1] != 'Q')
        {
            castlingRights &= ~(1 << 1);            
        }
        if(sections[2][0] != 'k' && sections[2][1] != 'k' && sections[2][2] != 'k')
        {
            castlingRights &= ~(1 << 2);
        }
        if(sections[2][0] != 'q' && sections[2][1] != 'q' && sections[2][2] != 'q' && sections[2][3] != 'q')
        {
            castlingRights &= ~(1 << 3);
        }
        if(sections[2] == "-")
        {
            castlingRights = 0;
        }

        halfMoves = std::stoi(sections[4]);
        fullMoves = std::stoi(sections[5]);       

        if(halfMoves < 0)
        {
            throw std::runtime_error("Illegal number of half moves since last pawn move or capture: " + sections[4]);
        }

        if(fullMoves < 0)
        {
            throw std::runtime_error("Illegal number of moves: " + sections[5]);
        }

        zKey = sideToMove == WHITE ? 0 : BlackToMoveKey;
        zKey ^= CastlingKeys[castlingRights];
        zKey ^= zKeyFromPieceBoard(pieceBoard(empty, allPieces, individualPieces));

        history.resize(1024);
    }

    std::string Position::getZKey() const
    {
        std::ostringstream hexRepresentation;
        hexRepresentation
            <<std::uppercase
            <<std::hex
            <<std::setw(16)
            <<std::setfill('0')
            <<zKey;
        return "0x" + hexRepresentation.str();
    }

    std::string Position::getBoardDisplay() const
    {
        auto const board = pieceBoard(empty, allPieces, individualPieces);
        
        char const p[] = {'*', 'N', 'B', 'R', 'Q', 'K','+', 'n', 'b', 'r', 'q', 'k', '.', 'E'};         
        char const v = '|'; char const h = '-';
        char const ul = '/'; char const ur = '\\';
        char const ll = '\\'; char const lr = '/';
        
        auto const bar =  std::string(17, h);
        std::string space = " ";
        std::string newline = "\n";

        std::string blackToMove = sideToMove == BLACK ? "  <<" : "";
        std::string whiteToMove = sideToMove == WHITE ? "  <<" : "";
        std::string boardDisplay = "  a b c d e f g h" + blackToMove + newline;
        boardDisplay += ul + bar + ur;
        for(int rank = 7; rank >= 0; --rank)
        {
            boardDisplay += newline + v;
            for(int file = 0; file < 8; ++file)
            {
                boardDisplay += space + p[board[rank*8+file]];
            } 
            boardDisplay += space + v + space + std::to_string(rank+1);
        }
        boardDisplay += newline + ll + bar + lr + newline;
        boardDisplay += "  a b c d e f g h" + whiteToMove + newline;
       
        return boardDisplay;
    }

    EvaluationStatistics Position::evaluateRecursively(int const depth)
    {
        if(depth > MAX_DEPTH)
        {
            throw std::runtime_error("depth " + std::to_string(depth) + "exceeds maximum depth");
        }

        EvaluationStatistics result;
        for(auto currentMaxDepth = depth; currentMaxDepth != depth+1; ++currentMaxDepth)
        {
            auto const start = std::chrono::steady_clock::now();
            maxDepth = currentMaxDepth;

            evaluationAtDepth = {};
            numberOfNodesAtDepth = {};

            sideToMove = static_cast<Color>(sideToMove ^ BLACK);
            evaluate(0);
            sideToMove = static_cast<Color>(sideToMove ^ BLACK);
    
            int64_t numberOfNodes = 0;
            int64_t numberOfQuiescenceNodes = 0;
            auto maximumReachedDepth = 0;

            for(auto d = 0; d < MAX_DEPTH + MAX_QUIESCENCE_DEPTH; ++d)
            {
                if(!numberOfNodesAtDepth[d])
                {
                    break;
                }
                
                if(d >= currentMaxDepth)
                {
                    numberOfQuiescenceNodes += numberOfNodesAtDepth[d];
                }
                    
                maximumReachedDepth = d;
                numberOfNodes += numberOfNodesAtDepth[d];
            }

            auto const stop = std::chrono::steady_clock::now();
            auto const duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start); 

            result = 
            {
                evaluationAtDepth[0],
                currentMaxDepth,
                maximumReachedDepth,
                numberOfNodes,
                numberOfQuiescenceNodes,
                static_cast<float>(duration.count())/1e6f
            };
        }
        return result;
    }   
}

#include <iostream>

namespace spezi
{
    void Position::evaluate(int const depth)
    {   
        if(isAttacked(ffs(allPieces[sideToMove] & individualPieces[KING])))
        {
            // last move by opponent left opponent's king in check
            // sideToMove here still refers to the opponent
            evaluationAtDepth[depth] = LOSS[sideToMove];
            return;
        }

#ifndef PERFT
        if(repetition(depth))
        {
            evaluationAtDepth[depth] = DRAW;
            return;
        }
#endif
        history[depth] = HistoryNode{zKey, halfMoves};

        auto const other = sideToMove;
        sideToMove = static_cast<Color>(sideToMove ^ BLACK);

        evaluationAtDepth[depth] = LOSS[sideToMove];
        ++numberOfNodesAtDepth[depth];
        auto const numberOfNodesAtEntry = numberOfNodesAtDepth[depth + 1];
        auto const quiescence = depth >= maxDepth;

#ifdef PERFT
        if(quiescence)
        {
            sideToMove = other;
            return;
        }
#endif
        if(quiescence)
        {   
            evaluationAtDepth[depth] = evaluateStatically();
            evaluateCaptures(depth);
            if(numberOfNodesAtDepth[depth + 1] == numberOfNodesAtEntry
                && isAttacked(ffs(allPieces[sideToMove] & individualPieces[KING])))
            {
                // no captures found any more but we are in check
                // try king moves first (more promising for escaping checks)
                // isAttacked may be left out if a null move search is
                // performed even in quiescence mode
                evaluateNonCaptures(depth);

                if(numberOfNodesAtDepth[depth + 1] == numberOfNodesAtEntry)
                {
                    // still no legal moves
                    evaluationAtDepth[depth] = LOSS[sideToMove];
                }
            }
        }
        else
        {
            evaluateCaptures(depth);
            evaluateNonCaptures(depth);

            if(numberOfNodesAtDepth[depth + 1] == numberOfNodesAtEntry)
            {
                // if we are here, there were no legal moves 
                // if null move is evaluated, we would probably not need to check isAttacked
                // anymore (null move would only result in zero additional nodes if in check)
                if(isAttacked(ffs(allPieces[sideToMove] & individualPieces[KING])))
                {
                    // mate
                    evaluationAtDepth[depth] = LOSS[sideToMove];
                }
                else
                {
                    // stalemate
                    evaluationAtDepth[depth] = DRAW;
                }
            }
        }

        sideToMove = other;
    }

    bool Position::repetition(int const depth)
    {
        auto constexpr minimalFullMovesToLookBack = 2;
        auto maximalFullMovesToLookBack = halfMoves / 2;
        for(auto fullMovesToLookBack = minimalFullMovesToLookBack; 
            fullMovesToLookBack <= maximalFullMovesToLookBack;
            ++fullMovesToLookBack)
        {
            if(history[depth - fullMovesToLookBack * 2].zKey == zKey)
            {
                return true;
            }
        }
        return false;
    }

    bool Position::isAttacked(Square const square)
    {
        auto const other = sideToMove ^ BLACK;

        return (PawnAttacks[sideToMove][square] & allPieces[other] & individualPieces[PAWN])
            || (KnightAttacks[square] & allPieces[other] & individualPieces[KNIGHT])
            || (DiagonalAttacks[square][pext(~empty,DiagonalMasks[square])] & allPieces[other]
                & (individualPieces[BISHOP] | individualPieces[QUEEN]))
            || (RankAttacks[square][pext(~empty, RankMasks[square])] & allPieces[other]
                & (individualPieces[ROOK] | individualPieces[QUEEN]))
            || (FileAttacks[square][pext(~empty, FileMasks[square])] & allPieces[other]
                & (individualPieces[ROOK] | individualPieces[QUEEN]))
            || (KingAttacks[square] & allPieces[other] & individualPieces[KING]);
    }   

    MilliSquare Position::evaluateStatically()
    {
        auto const p = populationIndex(popcount(~empty));
        auto value = StaticMobilities<WHITE, KING>[ffs(allPieces[WHITE] & individualPieces[KING])][p];
        value -= StaticMobilities<BLACK, KING>[ffs(allPieces[BLACK] & individualPieces[KING])][p];

        value += staticPieceEvaluation<WHITE, QUEEN>(allPieces[WHITE] & individualPieces[QUEEN], p);
        value -= staticPieceEvaluation<BLACK, QUEEN>(allPieces[BLACK] & individualPieces[QUEEN], p);
        
        value += staticPieceEvaluation<WHITE, ROOK>(allPieces[WHITE] & individualPieces[ROOK], p);
        value -= staticPieceEvaluation<BLACK, ROOK>(allPieces[BLACK] & individualPieces[ROOK], p);
        
        value += staticPieceEvaluation<WHITE, BISHOP>(allPieces[WHITE] & individualPieces[BISHOP], p);
        value -= staticPieceEvaluation<BLACK, BISHOP>(allPieces[BLACK] & individualPieces[BISHOP], p);
        
        value += staticPieceEvaluation<WHITE, KNIGHT>(allPieces[WHITE] & individualPieces[KNIGHT], p);
        value -= staticPieceEvaluation<BLACK, KNIGHT>(allPieces[BLACK] & individualPieces[KNIGHT], p);
       
        value += staticPieceEvaluation<WHITE, PAWN>(allPieces[WHITE] & individualPieces[PAWN], p);
        value -= staticPieceEvaluation<BLACK, PAWN>(allPieces[BLACK] & individualPieces[PAWN], p);
       
        return value;
    }

    void Position::evaluateCaptures(int const depth)
    {
        auto const other = sideToMove ^ BLACK;
        auto const promotionRank = (sideToMove == WHITE ? RANKS[SquaresPerFile-2] : RANKS[1]);

        auto const castlingRightsAtEntry = castlingRights;
        zKey ^= enPassant ? EnPassantKeys[ffs(enPassant) % SquaresPerRank] : ZKey {0};
        auto const enPassantAtEntry = enPassant;
        enPassant = EMPTY;
        auto const halfMovesAtEntry = halfMoves;
        halfMoves = 0;

        // MVV-LVA: queens first
        for(auto attackedPiece = static_cast<int>(QUEEN); attackedPiece >= static_cast<int>(PAWN); --attackedPiece)
        {
            auto targets = allPieces[other] & individualPieces[attackedPiece];
            while(targets)
            {
                auto const target = ffs(targets);

                auto const diagonalAttacks = DiagonalAttacks[target][pext(~empty, DiagonalMasks[target])];
                auto const rankAttacks = RankAttacks[target][pext(~empty, RankMasks[target])];
                auto const fileAttacks = FileAttacks[target][pext(~empty, FileMasks[target])];

                auto const to = A1 << target;
                allPieces[sideToMove] ^= to;
                allPieces[other] ^= to;
                individualPieces[attackedPiece] ^= to;

                // generate attackers by finding reverse color attacks from target square
                BitBoard attackers[NumberOfPieceTypes];
                attackers[PAWN] = PawnAttacks[other][target] & allPieces[sideToMove] & individualPieces[PAWN];
                attackers[KNIGHT] = KnightAttacks[target] & allPieces[sideToMove] & individualPieces[KNIGHT]; 
                attackers[BISHOP] = diagonalAttacks & allPieces[sideToMove] & individualPieces[BISHOP];
                attackers[ROOK] = (rankAttacks | fileAttacks) & allPieces[sideToMove] & individualPieces[ROOK];
                attackers[QUEEN] = (diagonalAttacks | rankAttacks | fileAttacks) & allPieces[sideToMove] & individualPieces[QUEEN];
                attackers[KING] = KingAttacks[target] & allPieces[sideToMove] & individualPieces[KING];

                // MVV-LVA: pawns first
                for(auto attackingPiece = static_cast<int>(PAWN); attackingPiece != NumberOfPieceTypes; ++attackingPiece)
                {
                    individualPieces[attackingPiece] ^= to;
                    zKey ^= PieceKeys[sideToMove][attackingPiece][target];

                    while(attackers[attackingPiece])
                    {
                        auto const attacker = ffs(attackers[attackingPiece]);
                        auto const from = A1 << attacker;
                        
                        allPieces[sideToMove] ^= from;
                        individualPieces[attackingPiece] ^= from;
                        empty ^= from;           
                        zKey ^= PieceKeys[sideToMove][attackingPiece][attacker];
                        zKey ^= CastlingKeys[castlingRights];             
                        castlingRights &= castlingCaptureUpdateFlags(from, to);
                        zKey ^= CastlingKeys[castlingRights];             

                        if(attackingPiece == PAWN && from & promotionRank)
                        {
                            individualPieces[attackingPiece] ^= to;
                            zKey ^= PieceKeys[sideToMove][attackingPiece][target];
                            for(int promotedPiece = static_cast<int>(QUEEN); 
                                promotedPiece != static_cast<int>(PAWN);
                                --promotedPiece)
                            {
                                individualPieces[promotedPiece] ^= to;
                                zKey ^= PieceKeys[sideToMove][promotedPiece][target];
                                evaluate(depth + 1);
                                updateWindowIfInsideWindow(depth);
                                --halfMoves;                            
                                individualPieces[promotedPiece] ^= to;
                                zKey ^= PieceKeys[sideToMove][promotedPiece][target];
                            }            
                            individualPieces[attackingPiece] ^= to;
                            zKey ^= PieceKeys[sideToMove][attackingPiece][target];
                        }
                        else
                        { 
                            evaluate(depth + 1);
                            updateWindowIfInsideWindow(depth);                            
                        }

                        allPieces[sideToMove] ^= from;
                        individualPieces[attackingPiece] ^= from;
                        empty ^= from;
                        zKey ^= PieceKeys[sideToMove][attackingPiece][attacker];
                        zKey ^= CastlingKeys[castlingRights];             
                        castlingRights = castlingRightsAtEntry;
                        zKey ^= CastlingKeys[castlingRights];             

                        attackers[attackingPiece] &= attackers[attackingPiece] - 1;
                    }

                    individualPieces[attackingPiece] ^= to;
                    zKey ^= PieceKeys[sideToMove][attackingPiece][target];
                }

                allPieces[sideToMove] ^= to;
                allPieces[other] ^= to;
                individualPieces[attackedPiece] ^= to;
                targets &= targets - 1;
            }
        }

        if(enPassantAtEntry)
        {
            auto const target = ffs(enPassantAtEntry);
            auto const pawn = A1 << (target + (SquaresPerRank * ((sideToMove << 1) - 1)));
            auto attackers = PawnAttacks[other][target] & allPieces[sideToMove] & individualPieces[PAWN];
            individualPieces[PAWN] ^= enPassantAtEntry;
            individualPieces[PAWN] ^= pawn;
            allPieces[sideToMove] ^= enPassantAtEntry;
            allPieces[other] ^= pawn;
            empty ^= (enPassantAtEntry ^ pawn);
            while(attackers)
            {
                auto const attacker = ffs(attackers);
                auto const from = A1 << attacker;
                allPieces[sideToMove] ^= from;
                individualPieces[PAWN] ^= from;
                empty ^= from;
                evaluate(depth + 1);
                updateWindowIfInsideWindow(depth);                            
                allPieces[sideToMove] ^= from;
                individualPieces[PAWN] ^= from;
                empty ^= from;
                attackers &= attackers - 1;
            }
            individualPieces[PAWN] ^= enPassantAtEntry;
            individualPieces[PAWN] ^= pawn;
            allPieces[sideToMove] ^= enPassantAtEntry;
            allPieces[other] ^= pawn;
            empty ^= (enPassantAtEntry ^ pawn);
        }

        enPassant = enPassantAtEntry;
        zKey ^= enPassant ? EnPassantKeys[ffs(enPassant) % SquaresPerRank] : ZKey {0};
        halfMoves = halfMovesAtEntry;
    }

    void Position::evaluateNonCaptures(int const depth)
    {
        auto const promotionRank = ((sideToMove == WHITE) ? RANKS[SquaresPerFile-2] : RANKS[1]);
        auto const castlingRightsAtEntry = castlingRights;
        zKey ^= enPassant ? EnPassantKeys[ffs(enPassant) % SquaresPerRank]: ZKey{0};
        auto const enPassantAtEntry = enPassant;
        enPassant = EMPTY;
        auto const halfMovesAtEntry = halfMoves;
        ++halfMoves;

        for(auto movingPiece = static_cast<int>(PAWN); movingPiece != NumberOfPieceTypes; ++movingPiece)
        {   
            auto movers = allPieces[sideToMove] & individualPieces[movingPiece];
            
            while(movers)
            {
                auto const mover = ffs(movers);
                auto const from = A1 << mover;
                allPieces[sideToMove] ^= from;
                individualPieces[movingPiece] ^= from;
                empty ^= from;
                zKey ^= PieceKeys[sideToMove][movingPiece][mover];
                if(movingPiece == KING || movingPiece == ROOK)
                {
                    zKey ^= CastlingKeys[castlingRights];  
                    castlingRights &= castlingCaptureUpdateFlags(from, from);
                    zKey ^= CastlingKeys[castlingRights];
                }

                auto targets = generateNonCaptureSquares(static_cast<Piece>(movingPiece), mover);

                while(targets)
                {
                    auto const target = ffs(targets);
                    auto const to = A1 << target;

                    allPieces[sideToMove] ^= to;    
                    empty ^= to;    

                    if(movingPiece == PAWN)
                    {
                        halfMoves = 0;

                        if(from & promotionRank)
                        {
                            for(int promotedPiece = static_cast<int>(QUEEN); 
                                promotedPiece != static_cast<int>(PAWN);
                                --promotedPiece)
                            {
                                individualPieces[promotedPiece] ^= to;
                                zKey ^= PieceKeys[sideToMove][promotedPiece][target];
                                evaluate(depth + 1);
                                updateWindowIfInsideWindow(depth);                            
                                individualPieces[promotedPiece] ^= to;
                                zKey ^= PieceKeys[sideToMove][promotedPiece][target];
                            }
                        }
                        else
                        {
                            individualPieces[PAWN] ^= to;
                            zKey ^= PieceKeys[sideToMove][PAWN][target];
                            enPassant = (A1 << ((ffs(from) + ffs(to)) >> 1)) & Files[mover];
                            zKey ^= enPassant ? EnPassantKeys[ffs(enPassant) % SquaresPerRank] : ZKey {0};
                            evaluate(depth + 1);
                            updateWindowIfInsideWindow(depth);                            
                            individualPieces[PAWN] ^= to;
                            zKey ^= PieceKeys[sideToMove][PAWN][target];
                            zKey ^= enPassant ? EnPassantKeys[ffs(enPassant) % SquaresPerRank] : ZKey {0};
                            enPassant = EMPTY;
                        }

                        halfMoves = halfMovesAtEntry + 1;
                    }                     
                    else 
                    {    
                        individualPieces[movingPiece] ^= to;
                        zKey ^= PieceKeys[sideToMove][movingPiece][target];
                        evaluate(depth + 1);
                        updateWindowIfInsideWindow(depth);                            
                        individualPieces[movingPiece] ^= to;
                        zKey ^= PieceKeys[sideToMove][movingPiece][target];
                    }
                    
                    allPieces[sideToMove] ^= to;
                    empty ^= to;

                    targets &= targets - 1;            
                }
            
                allPieces[sideToMove] ^= from;
                individualPieces[movingPiece] ^= from;
                empty ^= from;
                zKey ^= PieceKeys[sideToMove][movingPiece][mover];
                if(movingPiece == KING || movingPiece == ROOK)
                {
                    zKey ^= CastlingKeys[castlingRights];  
                    castlingRights = castlingRightsAtEntry;
                    zKey ^= CastlingKeys[castlingRights];
                }
                
                movers &= movers - 1;
            }
        }

        auto const shift = sideToMove * 56;

        if((castlingRights & (1 << (sideToMove << 1)))
            && (((empty >> shift) & (F1|G1)) == (F1|G1))
            && !isAttacked(ffs(F1 << shift))
            && !isAttacked(ffs(E1 << shift)))
        {
            auto const affectedKingSquares = (E1 | G1) << shift;
            auto const affectedRookSquares = (H1 | F1) << shift;
            auto const affectedSquares = affectedKingSquares | affectedRookSquares;
            allPieces[sideToMove] ^= affectedSquares;
            empty ^= affectedSquares;
            individualPieces[KING] ^= affectedKingSquares;
            individualPieces[ROOK] ^= affectedRookSquares;
            zKey ^= PieceKeys[sideToMove][KING][e1 + shift];
            zKey ^= PieceKeys[sideToMove][KING][g1 + shift];
            zKey ^= PieceKeys[sideToMove][ROOK][h1 + shift];
            zKey ^= PieceKeys[sideToMove][ROOK][f1 + shift];
            zKey ^= CastlingKeys[castlingRights];
            castlingRights &= ~(3 << (sideToMove << 1));
            zKey ^= CastlingKeys[castlingRights];
            evaluate(depth + 1);
            updateWindowIfInsideWindow(depth);                            
            allPieces[sideToMove] ^= affectedSquares;
            empty ^= affectedSquares;
            individualPieces[KING] ^= affectedKingSquares;
            individualPieces[ROOK] ^= affectedRookSquares;
            zKey ^= PieceKeys[sideToMove][KING][e1 + shift];
            zKey ^= PieceKeys[sideToMove][KING][g1 + shift];
            zKey ^= PieceKeys[sideToMove][ROOK][h1 + shift];
            zKey ^= PieceKeys[sideToMove][ROOK][f1 + shift];
            zKey ^= CastlingKeys[castlingRights];
            castlingRights = castlingRightsAtEntry;
            zKey ^= CastlingKeys[castlingRights];
        }
        if((castlingRights & (2 << (sideToMove << 1)))
            && (((empty >> shift) & (B1|C1|D1)) == (B1|C1|D1))
            && !isAttacked(ffs(D1 << shift))
            && !isAttacked(ffs(E1 << shift)))
        {
            auto const affectedKingSquares = (E1 | C1) << shift;
            auto const affectedRookSquares = (A1 | D1) << shift;
            auto const affectedSquares = affectedKingSquares | affectedRookSquares;
            allPieces[sideToMove] ^= affectedSquares;
            empty ^= affectedSquares;
            individualPieces[KING] ^= affectedKingSquares;
            individualPieces[ROOK] ^= affectedRookSquares;
            zKey ^= PieceKeys[sideToMove][KING][e1 + shift];
            zKey ^= PieceKeys[sideToMove][KING][c1 + shift];
            zKey ^= PieceKeys[sideToMove][ROOK][a1 + shift];
            zKey ^= PieceKeys[sideToMove][ROOK][d1 + shift];
            zKey ^= CastlingKeys[castlingRights];
            castlingRights &= ~(3 << (sideToMove << 1));
            zKey ^= CastlingKeys[castlingRights];
            evaluate(depth + 1);
            updateWindowIfInsideWindow(depth);                            
            allPieces[sideToMove] ^= affectedSquares;
            empty ^= affectedSquares;
            individualPieces[KING] ^= affectedKingSquares;
            individualPieces[ROOK] ^= affectedRookSquares;
            zKey ^= PieceKeys[sideToMove][KING][e1 + shift];
            zKey ^= PieceKeys[sideToMove][KING][c1 + shift];
            zKey ^= PieceKeys[sideToMove][ROOK][a1 + shift];
            zKey ^= PieceKeys[sideToMove][ROOK][d1 + shift];
            zKey ^= CastlingKeys[castlingRights];
            castlingRights = castlingRightsAtEntry;
            zKey ^= CastlingKeys[castlingRights];
        }

        enPassant = enPassantAtEntry;
        zKey ^= enPassant ? EnPassantKeys[ffs(enPassant) % SquaresPerRank]: ZKey{0};
        halfMoves = halfMovesAtEntry;
    }

    bool Position::updateWindowIfInsideWindow(int const depth)
    {
#ifndef PERFT
        int const sign = sideToMove == WHITE ? 1 : -1;
        evaluationAtDepth[depth] = 
            sign * evaluationAtDepth[depth + 1] > sign * evaluationAtDepth[depth] ?
            evaluationAtDepth[depth + 1] : evaluationAtDepth[depth];
        return true;
#else
        return true;
#endif         
    }
    
    BitBoard Position::generateNonCaptureSquares(Piece const piece, Square const origin) const
    {
        switch(piece)
        {
            case PAWN:
            {
                auto reachable = PawnPushes[sideToMove][origin] & empty;        
                // double pushes from starting position
                if(sideToMove == WHITE)
                {
                    reachable |= (reachable << SquaresPerRank) & RANKS[3];
                }
                else
                {
                    reachable |= (reachable >> SquaresPerRank) & RANKS[4];
                }
                return reachable & empty;
            }
            case KNIGHT:
                return KnightAttacks[origin] & empty;
            case BISHOP:
                return DiagonalAttacks[origin][pext(~empty, DiagonalMasks[origin])] & empty;
            case ROOK:        
                return (RankAttacks[origin][pext(~empty, RankMasks[origin])]
                        | FileAttacks[origin][pext(~empty, FileMasks[origin])])
                        & empty;        
            case QUEEN:
                return (DiagonalAttacks[origin][pext(~empty, DiagonalMasks[origin])]
                        | RankAttacks[origin][pext(~empty, RankMasks[origin])]
                        | FileAttacks[origin][pext(~empty, FileMasks[origin])])
                        & empty;  
            case KING:
                return KingAttacks[origin] & empty;
            default:
                throw std::runtime_error("unknown piece type: " + std::to_string(piece));      
        }
    }
}