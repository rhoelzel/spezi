#include "Position.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <sstream>
#include <iostream>

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

        static inline char castlingUpdateFlags(BitBoard const from, BitBoard const to)
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
    }

    std::string Position::getFen() const
    {
        return std::to_string(halfMoves);
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
            auto maximumQuiescenceDepth = 0;

            for(auto d = 0; d < MAX_DEPTH + MAX_QUIESCENCE_DEPTH; ++d)
            {
                if(!numberOfNodesAtDepth[d])
                {
                    break;
                }
                
                if(d >= currentMaxDepth)
                {
                    numberOfQuiescenceNodes += numberOfNodesAtDepth[d];
                    maximumQuiescenceDepth = d;
                }
                numberOfNodes += numberOfNodesAtDepth[d];
            }

            auto const stop = std::chrono::steady_clock::now();
            auto const duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start); 

            result = 
            {
                evaluationAtDepth[0],
                currentMaxDepth,
                maximumQuiescenceDepth,
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

        auto const other = sideToMove;
        sideToMove = static_cast<Color>(sideToMove ^ BLACK);

        evaluationAtDepth[depth] = LOSS[sideToMove];
        ++numberOfNodesAtDepth[depth];
        auto const numberOfNodesAtEntry = numberOfNodesAtDepth[depth + 1];
                
//        std::cout<<"evaluation at depth: "<<depth<<std::endl;
//        std::cout<<getBoardDisplay()<<std::endl;

        auto const quiescence = depth >= maxDepth;

        //comment this in for perft checks 
        if(quiescence)
        {
            //std::cout << getBoardDisplay();
            sideToMove = other;
            return;
        }/**/

        if(quiescence)
        {   
            evaluationAtDepth[depth] = evaluateStatically();
//            std::cout<<"static: "<<evaluationAtDepth[depth]<<std::endl;
            evaluateCaptures(depth);
            if(numberOfNodesAtDepth[depth + 1] == numberOfNodesAtEntry
                && isAttacked(ffs(allPieces[sideToMove] & individualPieces[KING])))
            {
                // no captures found any more but we are in check
                // try king moves first (more promising for escaping checks)
                // isAttacked may be left out if a null move search is
                // performed even in quiescence mode
                evaluateNonCaptures<KING>(depth);
                evaluateNonCaptures<QUEEN>(depth);
                evaluateNonCaptures<ROOK>(depth);
                evaluateNonCaptures<BISHOP>(depth);
                evaluateNonCaptures<KNIGHT>(depth);
                evaluateNonCaptures<PAWN>(depth);

                if(numberOfNodesAtDepth[depth + 1] == numberOfNodesAtEntry)
                {
                    // still no legal moves
                    evaluationAtDepth[depth] = LOSS[sideToMove];
//                    std::cout<<"final q: "<<evaluationAtDepth[depth]<<std::endl;
                }
            }
//            std::cout<<"exit q"<<std::endl;
        }
        else
        {
            evaluateCaptures(depth);
            evaluateNonCaptures<PAWN>(depth);
            evaluateNonCaptures<KNIGHT>(depth);
            evaluateNonCaptures<BISHOP>(depth);
            evaluateNonCaptures<ROOK>(depth);
            evaluateNonCaptures<QUEEN>(depth);
            evaluateNonCaptures<KING>(depth);

            if(numberOfNodesAtDepth[depth + 1] == numberOfNodesAtEntry)
            {
                // if we are here, there were no legal moves 
                // if null move is evaluated, we would probably not need to check isAttacked
                // anymore (null move would only result in zero additional nodes if in check)
                if(isAttacked(ffs(allPieces[sideToMove] & individualPieces[KING])))
                {
                    // mate
                    evaluationAtDepth[depth] = LOSS[sideToMove];
//                    std::cout<<"final r: "<<evaluationAtDepth[depth]<<std::endl;
                }
                else
                {
                    // stalemate
                    evaluationAtDepth[depth] = DRAW;
//                    std::cout<<"final r: "<<evaluationAtDepth[depth]<<std::endl;
                }
            }
//            std::cout<<"exit r"<<std::endl;
        }

        sideToMove = other;
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
        auto const enPassantAtEntry = enPassant;
        enPassant = EMPTY;

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

                    while(attackers[attackingPiece])
                    {
                        auto const attacker = ffs(attackers[attackingPiece]);
                        auto const from = A1 << attacker;
                        
                        allPieces[sideToMove] ^= from;
                        individualPieces[attackingPiece] ^= from;
                        empty ^= from;                        
                        castlingRights &= castlingUpdateFlags(from, to);

                        if(attackingPiece == PAWN && from & promotionRank)
                        {
                            individualPieces[attackingPiece] ^= to;
                            for(int promotedPiece = static_cast<int>(QUEEN); 
                                promotedPiece != static_cast<int>(PAWN);
                                --promotedPiece)
                            {
                                individualPieces[promotedPiece] ^= to;
                                evaluate(depth + 1);
                                updateEval(depth);                            
                                individualPieces[promotedPiece] ^= to;
                            }            
                            individualPieces[attackingPiece] ^= to;
                        }
                        else
                        { 
                            evaluate(depth + 1);
                            updateEval(depth);                            
                        }

                        allPieces[sideToMove] ^= from;
                        individualPieces[attackingPiece] ^= from;
                        empty ^= from;
                        castlingRights = castlingRightsAtEntry;

                        attackers[attackingPiece] &= attackers[attackingPiece] - 1;
                    }

                    individualPieces[attackingPiece] ^= to;
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
                updateEval(depth);                            
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
    }

    template<Piece piece>
    void Position::evaluateNonCaptures(int const depth)
    {
        auto const promotionRank = ((sideToMove == WHITE) ? RANKS[SquaresPerFile-2] : RANKS[1]);
        auto const castlingRightsAtEntry = castlingRights;
        auto const enPassantAtEntry = enPassant;
        enPassant = EMPTY;

        auto movers = allPieces[sideToMove] & individualPieces[piece];
        
        while(movers)
        {
            auto const mover = ffs(movers);
            auto const from = A1 << mover;
            allPieces[sideToMove] ^= from;
            individualPieces[piece] ^= from;
            empty ^= from;

            auto targets = generateNonCaptureSquares<piece>(mover);

            while(targets)
            {
                auto const target = ffs(targets);
                auto const to = A1 << target;

                allPieces[sideToMove] ^= to;    
                empty ^= to;    
                
                if constexpr(piece == PAWN)
                {
                    if(from & promotionRank)
                    {
                        for(int promotedPiece = static_cast<int>(QUEEN); 
                            promotedPiece != static_cast<int>(PAWN);
                            --promotedPiece)
                        {
                            individualPieces[promotedPiece] ^= to;
                            evaluate(depth + 1);
                            updateEval(depth);                            
                            individualPieces[promotedPiece] ^= to;
                        }
                    }
                    else
                    {
                        individualPieces[PAWN] ^= to;
                        enPassant = (A1 << ((ffs(from) + ffs(to)) >> 1)) & Files[mover];
                        evaluate(depth + 1);
                        updateEval(depth);                            
                        individualPieces[PAWN] ^= to;
                        enPassant = EMPTY;
                    }                     
                }
                else
                {                    
                    if constexpr(piece == ROOK || piece == KING)
                    {
                        castlingRights &= castlingUpdateFlags(from, to);   
                    }
                    individualPieces[piece] ^= to;
                    evaluate(depth + 1);
                    updateEval(depth);                            
                    individualPieces[piece] ^= to;
                    if constexpr(piece == ROOK || piece == KING)
                    {
                        castlingRights = castlingRightsAtEntry;
                    }
                }

                allPieces[sideToMove] ^= to;
                empty ^= to;
                targets &= targets - 1;            
            }
            
            allPieces[sideToMove] ^= from;
            individualPieces[piece] ^= from;
            empty ^= from;
            movers &= movers - 1;
        }

        // castling
        if constexpr(piece == KING)
        {
            auto const shift = sideToMove * 56;
            auto const K = E1 << shift; 

            if(!isAttacked(ffs(K))) // pretty inefficient, fix later
            {        
                if((castlingRights & (1 << (sideToMove << 1)))
                    && (((empty >> shift) & (F1|G1)) == (F1|G1))
                    && !isAttacked(ffs(F1 << shift)))
                {
                    auto const affectedKingSquares = (E1 | G1) << shift;
                    auto const affectedRookSquares = (H1 | F1) << shift;
                    auto const affectedSquares = affectedKingSquares | affectedRookSquares;
                    castlingRights &= ~(3 << (sideToMove << 1));
                    allPieces[sideToMove] ^= affectedSquares;
                    empty ^= affectedSquares;
                    individualPieces[KING] ^= affectedKingSquares;
                    individualPieces[ROOK] ^= affectedRookSquares;
                    evaluate(depth + 1);
                    updateEval(depth);                            
                    allPieces[sideToMove] ^= affectedSquares;
                    empty ^= affectedSquares;
                    individualPieces[KING] ^= affectedKingSquares;
                    individualPieces[ROOK] ^= affectedRookSquares;
                    castlingRights = castlingRightsAtEntry;
                }
                if((castlingRights & (2 << (sideToMove << 1)))
                    && (((empty >> shift) & (B1|C1|D1)) == (B1|C1|D1))
                    && !isAttacked(ffs(D1 << shift)))
                {
                    auto const affectedKingSquares = (E1 | C1) << shift;
                    auto const affectedRookSquares = (A1 | D1) << shift;
                    auto const affectedSquares = affectedKingSquares | affectedRookSquares;
                    castlingRights &= ~(3 << (sideToMove << 1));
                    allPieces[sideToMove] ^= affectedSquares;
                    empty ^= affectedSquares;
                    individualPieces[KING] ^= affectedKingSquares;
                    individualPieces[ROOK] ^= affectedRookSquares;
                    evaluate(depth + 1);
                    updateEval(depth);                            
                    allPieces[sideToMove] ^= affectedSquares;
                    empty ^= affectedSquares;
                    individualPieces[KING] ^= affectedKingSquares;
                    individualPieces[ROOK] ^= affectedRookSquares;
                    castlingRights = castlingRightsAtEntry;
                }
            }
        }
        enPassant = enPassantAtEntry;
    }

    void Position::updateEval(int const depth)
    {
//        std::cout<<"updateEval: side "<<sideToMove<<", previous "<<evaluationAtDepth[depth]
//            <<" one above "<<evaluationAtDepth[depth+1];
        int const sign = sideToMove == WHITE ? 1 : -1;
        evaluationAtDepth[depth] = 
            sign * evaluationAtDepth[depth + 1] > sign * evaluationAtDepth[depth] ?
            evaluationAtDepth[depth + 1] : evaluationAtDepth[depth]; 
//        std::cout<<" new "<<evaluationAtDepth[depth];
    }
    
    template<Piece piece>
    BitBoard Position::generateNonCaptureSquares(Square const origin) const
    {
        BitBoard reachable { EMPTY };

        if constexpr(piece == PAWN)
        {
            reachable = PawnPushes[sideToMove][origin] & empty;        
            // double pushes from starting position
            if(sideToMove == WHITE)
            {
                reachable |= (reachable << SquaresPerRank) & RANKS[3];
            }
            else
            {
                reachable |= (reachable >> SquaresPerRank) & RANKS[4];
            }
        }
        
        if constexpr(piece == KNIGHT)
        {
            reachable = KnightAttacks[origin];
        }
        
        if constexpr(piece == BISHOP || piece == QUEEN)
        {
            reachable = DiagonalAttacks[origin][pext(~empty, DiagonalMasks[origin])];
        }
        
        if constexpr(piece == ROOK || piece == QUEEN)
        {
            reachable |= (RankAttacks[origin][pext(~empty, RankMasks[origin])]
                        | FileAttacks[origin][pext(~empty, FileMasks[origin])]);        
        }

        if constexpr(piece == KING)
        {
            reachable = KingAttacks[origin];
        }

        return reachable & empty;
    }
}