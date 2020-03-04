#include "Position.hpp"

#include <algorithm>
#include <array>
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

        halfMoves = std::stoi(sections[4]);
        fullMoves = std::stoi(sections[5]);           
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

        std::string boardDisplay = "  a b c d e f g h" + newline;
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
        boardDisplay += "  a b c d e f g h" + newline;
       
        return boardDisplay;
    }

    MilliSquare Position::evaluate(int depth)
    {
        switch(depth)
        {
            case 0:
                evaluate<WHITE, 0>();
                break;
            case 1:
                evaluate<WHITE, 1>();
                break;
            case 2:
                evaluate<WHITE, 2>();
                break;
            case 3:
                evaluate<WHITE, 3>();
                break;
            case 4:
                evaluate<WHITE, 4>();
                break;
            case 5:
                evaluate<WHITE, 5>();
                break;
            case 6:
                evaluate<WHITE, 6>();
                break;
            case 7:
                evaluate<WHITE, 7>();
                break;
            default:
                break;
        }
        return 0;
    }   

    template<Color color, int depth>
    void Position::evaluate()
    {
        if constexpr (depth == 0)
        {
            quiescence<color, depth>();
            return;
        }
        else
        {
            // illegal if side to move is in check
            if(inCheck<color>())
            {
                return;
            }
            
            auto constexpr initial = color == WHITE ? BLACK_WIN : WHITE_WIN;
            evaluationAtDepth[MAX_DEPTH - 1 - depth] = initial;
    
            evaluateCaptures<color, depth>();
            evaluateNonCaptures<color, PAWN, depth>();
            evaluateNonCaptures<color, KNIGHT, depth>();
            evaluateNonCaptures<color, BISHOP, depth>();
            evaluateNonCaptures<color, ROOK, depth>();
            evaluateNonCaptures<color, QUEEN, depth>();
            evaluateNonCaptures<color, KING, depth>();
            ++halfMoves;    
        }
    }

    template<Color color, int depth>
    void Position::quiescence()
    {    
        if(!inCheck<color>())
        {
            ++halfMoves;
           /* auto const value = staticEvaluation();
            if constexpr (color == WHITE)
            {
                if(value > evaluationAtDepth[MAX_DEPTH - 1 - depth])
                {
                    evaluationAtDepth[MAX_DEPTH - 1 - depth] = value;
                }
            }
            else
            {
                if(value > evaluationAtDepth[MAX_DEPTH - 1 - depth])
                {
                    evaluationAtDepth[MAX_DEPTH - 1 - depth] = value;
                }
            }*/
            //std::cout<<getBoardDisplay();
            //std::cout<<"static value (at 0): "<<milliToPawnUnit(value)<<std::endl;
        }
    }

    MilliSquare Position::staticEvaluation()
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

    template<Color color>
    bool Position::inCheck() const
    {
        auto constexpr other = (color == WHITE ? BLACK : WHITE);
        
        auto const king = ffs(allPieces[color] & individualPieces[KING]);
        
        auto const diagonalAttacks = DiagonalAttacks[king][pext(~empty, DiagonalMasks[king])];
        auto const pawnAttacks = PawnAttacks<color>[king] & allPieces[other] & individualPieces[PAWN];
        auto const rankAttacks = RankAttacks[king][pext(~empty, RankMasks[king])];
        auto const knightAttacks = KnightAttacks[king] & allPieces[other] & individualPieces[KNIGHT];
        auto const fileAttacks = FileAttacks[king][pext(~empty, FileMasks[king])];
        auto const bishopAttacks = diagonalAttacks & allPieces[other] & individualPieces[BISHOP];
        auto const rookAttacks = (rankAttacks | fileAttacks) & allPieces[other] & individualPieces[ROOK]; 
        auto const queenAttacks = (diagonalAttacks | rankAttacks | fileAttacks) & allPieces[other] & individualPieces[QUEEN];
        
        return pawnAttacks | knightAttacks | bishopAttacks | rookAttacks | queenAttacks; 
    }

    template<Color color, int depth>
    void Position::evaluateCaptures()
    {
        auto constexpr other = (color == WHITE ? BLACK : WHITE);
        auto constexpr promotionRank = ((color == WHITE) ? RANKS[SquaresPerFile-2] : RANKS[1]);

        // MVV-LVA: queens first
        for(auto attackedPiece = static_cast<int>(QUEEN); attackedPiece >= static_cast<int>(PAWN); --attackedPiece)
        {
            auto targets = allPieces[other] & individualPieces[attackedPiece];
            while(targets)
            {
                auto const target = ffs(targets);
                auto const to = A1 << target;
                allPieces[color] ^= to;
                allPieces[other] ^= to;
                individualPieces[attackedPiece] ^= to;

                // compute here, use later (hopefully this will speed up things)
                auto const diagonalAttacks = DiagonalAttacks[target][pext(~empty, DiagonalMasks[target])];

                // generate attackers by finding reverse color attacks from target square
                // MVV-LVA: pawns first
                auto attackers = PawnAttacks<other>[target] & allPieces[color] & individualPieces[PAWN];
                while(attackers)
                {
                    auto const attacker = ffs(attackers);
                    auto const from = A1 << attacker;

                    allPieces[color] ^= from;
                    individualPieces[PAWN] ^= from;
                    empty ^= from;

                    if(from & promotionRank)
                    {
                        for(int promotedPiece = static_cast<int>(QUEEN); 
                            promotedPiece != static_cast<int>(PAWN);
                            --promotedPiece)
                        {
                            individualPieces[promotedPiece] ^= to;
                            evaluate<other, depth-1>();
                            updateEval<color, depth>();                            
                            individualPieces[promotedPiece] ^= to;
                        }
                    }
                    else
                    { 
                        individualPieces[PAWN] ^= to;
                        evaluate<other, depth-1>();
                        updateEval<color, depth>();  
                        individualPieces[PAWN] ^= to;                          
                    }
                
                    allPieces[color] ^= from;
                    individualPieces[PAWN] ^= from;
                    empty ^= from;
                    attackers &= attackers - 1;
                }

                // compute here, use later (hopefully this will speed up things)
                auto const rankAttacks = RankAttacks[target][pext(~empty, RankMasks[target])];

                attackers = KnightAttacks[target] & allPieces[color] & individualPieces[KNIGHT];
                individualPieces[KNIGHT] ^= to;
                while(attackers)
                {
                    auto const attacker = ffs(attackers);
                    auto const from = A1 << attacker;
                    allPieces[color] ^= from;
                    individualPieces[KNIGHT] ^= from;
                    empty ^= from;
                    evaluate<other, depth-1>();
                    updateEval<color, depth>();  
                    allPieces[color] ^= from;
                    individualPieces[KNIGHT] ^= from;
                    empty ^= from;
                    attackers &= attackers - 1;
                }
                individualPieces[KNIGHT] ^= to;

                // compute here, use later (hopefully this will speed up things)
                auto const fileAttacks = FileAttacks[target][pext(~empty, RankMasks[target])];

                //  use it
                attackers = diagonalAttacks & allPieces[color] & individualPieces[BISHOP];
                individualPieces[BISHOP] ^= to;
                while(attackers)
                {
                    auto const attacker = ffs(attackers);
                    auto const from = A1 << attacker;
                    allPieces[color] ^= from;
                    individualPieces[BISHOP] ^= from;
                    empty ^= from;
                    evaluate<other, depth-1>();
                    updateEval<color, depth>();  
                    allPieces[color] ^= from;
                    individualPieces[BISHOP] ^= from;
                    empty ^= from;
                    attackers &= attackers - 1;
                }
                individualPieces[BISHOP] ^= to;

                //  use it
                attackers = (rankAttacks | fileAttacks) & allPieces[color] & individualPieces[ROOK];
                individualPieces[ROOK] ^= to;
                while(attackers)
                {
                    auto const attacker = ffs(attackers);
                    auto const from = A1 << attacker;
                    allPieces[color] ^= from;
                    individualPieces[ROOK] ^= from;
                    empty ^= from;
                    evaluate<other, depth-1>();
                    updateEval<color, depth>();  
                    allPieces[color] ^= from;
                    individualPieces[ROOK] ^= from;
                    empty ^= from;
                    attackers &= attackers - 1;
                }
                individualPieces[ROOK] ^= to;

                //  use it
                attackers = (diagonalAttacks | rankAttacks | fileAttacks) & allPieces[color] & individualPieces[QUEEN];
                individualPieces[QUEEN] ^= to;
                while(attackers)
                {
                    auto const attacker = ffs(attackers);
                    auto const from = A1 << attacker;
                    allPieces[color] ^= from;
                    individualPieces[QUEEN] ^= from;
                    empty ^= from;
                    evaluate<other, depth-1>();
                    updateEval<color, depth>();  
                    allPieces[color] ^= from;
                    individualPieces[QUEEN] ^= from;
                    empty ^= from;
                    attackers &= attackers - 1;
                }
                individualPieces[QUEEN] ^= to;

                
                auto const attacker = ffs(KingAttacks[target] & allPieces[color] & individualPieces[KING]);
                if(attacker != NULL_SQUARE)
                {
                    individualPieces[KING] ^= to;
                    auto const from = A1 << attacker;
                    allPieces[color] ^= from;
                    individualPieces[KING] ^= from;
                    empty ^= from;
                    evaluate<other, depth-1>();
                    updateEval<color, depth>();  
                    allPieces[color] ^= from;
                    individualPieces[KING] ^= from;
                    empty ^= from;
                    attackers &= attackers - 1;
                    individualPieces[KING] ^= to;
                }

                allPieces[color] ^= to;
                allPieces[other] ^= to;
                individualPieces[attackedPiece] ^= to;
                targets &= targets - 1;
            }
        }
    }

    template<Color color, Piece piece, int depth>
    void Position::evaluateNonCaptures()
    {
        auto constexpr other = (color == WHITE ? BLACK : WHITE); 
        auto constexpr promotionRank = ((color == WHITE) ? RANKS[SquaresPerFile-2] : RANKS[1]);

        auto movers = allPieces[color] & individualPieces[piece];
        
        while(movers)
        {
            auto const mover = ffs(movers);
            auto const from = A1 << mover;
            allPieces[color] ^= from;
            individualPieces[piece] ^= from;
            empty ^= from;

            auto targets = generateNonCaptureSquares<color, piece>(mover);

            while(targets)
            {
                auto const target = ffs(targets);
                auto const to = A1 << target;

                allPieces[color] ^= to;    
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
                            evaluate<other, depth-1>();
                            updateEval<color, depth>();                            
                            individualPieces[promotedPiece] ^= to;
                        }
                    }
                    else
                    {
                        individualPieces[PAWN] ^= to;
                        evaluate<other, depth-1>();
                        updateEval<color, depth>();                            
                        individualPieces[PAWN] ^= to;
                    }                     
                }
                else
                {
                    individualPieces[piece] ^= to;
                    evaluate<other, depth-1>();
                    updateEval<color, depth>();                            
                    individualPieces[piece] ^= to;
                }

                allPieces[color] ^= to;
                empty ^= to;
                targets &= targets - 1;            
            }
            
            allPieces[color] ^= from;
            individualPieces[piece] ^= from;
            empty ^= from;
            movers &= movers - 1;
        }

        // castling, en passant?
    }

    template<Color color, int depth>
    void Position::updateEval()
    {
        /*if constexpr(depth==6)
        {   
            std::cout<<getBoardDisplay()<<std::endl;
            std::cout<<"evaluation old:"<<milliToPawnUnit(evaluationAtDepth[MAX_DEPTH - depth -1])<<std::endl;
        }*/
        if constexpr(color == WHITE)
        {
            if(evaluationAtDepth[MAX_DEPTH - depth - 1] < evaluationAtDepth[MAX_DEPTH - depth])
            {
                evaluationAtDepth[MAX_DEPTH - depth - 1] = evaluationAtDepth[MAX_DEPTH - depth];
       /*         if(depth>4)
                {
                    std::cout<<getBoardDisplay();
                    std::cout<<"new high white: "<< milliToPawnUnit(evaluationAtDepth[MAX_DEPTH-depth-1])<<std::endl;
                }*/
            }
        }
        else
        {
            if(evaluationAtDepth[MAX_DEPTH - depth - 1] > evaluationAtDepth[MAX_DEPTH - depth])
            {
                evaluationAtDepth[MAX_DEPTH - depth - 1] = evaluationAtDepth[MAX_DEPTH - depth];
             /*   if(depth > 4)
                {
                    std::cout<<getBoardDisplay();
                    std::cout<<"new low black: "<< milliToPawnUnit(evaluationAtDepth[MAX_DEPTH-depth-1])<<std::endl;
                }*/
            }
        }
    }
    
    template<Color color, Piece piece>
    BitBoard Position::generateNonCaptureSquares(Square const origin) const
    {
        BitBoard reachable { EMPTY };

        if constexpr(piece == PAWN)
        {
            reachable = PawnPushes<color>[origin] & empty;        
            // double pushes from starting position
            if constexpr(color == WHITE)
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