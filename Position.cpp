#include "Position.hpp"

#include <algorithm>
#include <array>
#include <sstream>

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
    bool Position::evaluate()
    {
        if constexpr (depth == 0)
        {
            return quiescence<color>();
        }
        else
        {
            // illegal if side to move is in check
            if(inCheck<color>())
            {
                return true;
            }

            return evaluateCaptures<color, PAWN, QUEEN, depth>()
                && evaluateCaptures<color, KNIGHT, QUEEN, depth>()
                && evaluateCaptures<color, BISHOP, QUEEN, depth>()
                && evaluateCaptures<color, ROOK, QUEEN, depth>()
                && evaluateCaptures<color, QUEEN, QUEEN, depth>()
                && evaluateCaptures<color, KING, QUEEN, depth>()
                && evaluateCaptures<color, PAWN, ROOK, depth>()
                && evaluateCaptures<color, KNIGHT, ROOK, depth>()
                && evaluateCaptures<color, BISHOP, ROOK, depth>()
                && evaluateCaptures<color, ROOK, ROOK, depth>()
                && evaluateCaptures<color, QUEEN, ROOK, depth>()
                && evaluateCaptures<color, KING, ROOK, depth>()
                && evaluateCaptures<color, PAWN, BISHOP, depth>()
                && evaluateCaptures<color, KNIGHT, BISHOP, depth>()
                && evaluateCaptures<color, BISHOP, BISHOP, depth>()
                && evaluateCaptures<color, ROOK, BISHOP, depth>()
                && evaluateCaptures<color, QUEEN, BISHOP, depth>()
                && evaluateCaptures<color, KING, BISHOP, depth>()
                && evaluateCaptures<color, PAWN, KNIGHT, depth>()
                && evaluateCaptures<color, KNIGHT, KNIGHT, depth>()
                && evaluateCaptures<color, BISHOP, KNIGHT, depth>()
                && evaluateCaptures<color, ROOK, KNIGHT, depth>()
                && evaluateCaptures<color, QUEEN, KNIGHT, depth>()
                && evaluateCaptures<color, KING, KNIGHT, depth>()
                && evaluateCaptures<color, PAWN, PAWN, depth>()
                && evaluateCaptures<color, KNIGHT, PAWN, depth>()
                && evaluateCaptures<color, BISHOP, PAWN, depth>()
                && evaluateCaptures<color, ROOK, PAWN, depth>()
                && evaluateCaptures<color, QUEEN, PAWN, depth>()
                && evaluateCaptures<color, KING, PAWN, depth>()
                && evaluateNonCaptures<color, PAWN, depth>()
                && evaluateNonCaptures<color, KNIGHT, depth>()
                && evaluateNonCaptures<color, BISHOP, depth>()
                && evaluateNonCaptures<color, ROOK, depth>()
                && evaluateNonCaptures<color, QUEEN, depth>()
                && evaluateNonCaptures<color, KING, depth>()
                && ++halfMoves;    
        }
    }

    template<Color color>
    bool Position::quiescence()
    {
        auto result = inCheck<color>();
        if(!result)
        {
            ++halfMoves;
        }
        return true;
    }

    template<Color color>
    bool Position::inCheck() const
    {
        auto const king = ffs(allPieces[color] & individualPieces[KING]);
        // generate attackers by finding own color attacks from king square
        return (generateCaptureSquares<color, PAWN>(king) & individualPieces[PAWN])
                || (generateCaptureSquares<color, KNIGHT>(king) & individualPieces[KNIGHT])
                || (generateCaptureSquares<color, BISHOP>(king) & individualPieces[BISHOP])
                || (generateCaptureSquares<color, ROOK>(king) & individualPieces[ROOK])
                || (generateCaptureSquares<color, QUEEN>(king) & individualPieces[QUEEN])
                || (generateCaptureSquares<color, KING>(king) & individualPieces[KING]);
    }

    template<Color color, Piece attackingPiece, Piece attackedPiece, int depth>
    bool Position::evaluateCaptures()
    {
        auto constexpr other = (color == WHITE ? BLACK : WHITE);
        auto constexpr promotionRank = ((color == WHITE) ? RANKS[SquaresPerFile-2] : RANKS[1]);

        auto targets = allPieces[other] & individualPieces[attackedPiece];
        bool inWindow = true;

        while(targets && inWindow)
        {
            auto const target = ffs(targets);
            auto const to = A1 << target;
            allPieces[color] ^= to;
            allPieces[other] ^= to;
            individualPieces[attackingPiece] ^= to;
            individualPieces[attackedPiece] ^= to;

            // generate attackers by finding reverse color attacks from target square
            auto attackers = generateCaptureSquares<other, attackingPiece>(target) 
                            & individualPieces[attackingPiece];

            while(attackers && inWindow)
            {
                auto const attacker = ffs(attackers);
                auto const from = A1 << attacker;
                allPieces[color] ^= from;
                individualPieces[attackingPiece] ^= from;
                empty ^= from;

                if constexpr(attackingPiece == PAWN)
                {
                    if(from & promotionRank)
                    {
                        individualPieces[PAWN] ^= to;
                        for(int promotedPiece = static_cast<int>(QUEEN); 
                            promotedPiece != static_cast<int>(PAWN) && inWindow;
                            --promotedPiece)
                        {
                            individualPieces[promotedPiece] ^= to;
                            inWindow = evaluate<other, depth-1>();
                            individualPieces[promotedPiece] ^= to;
                        }
                        individualPieces[PAWN] ^= to;
                    }
                    else
                    {
                        inWindow = evaluate<other, depth-1>();
                    }                     
                }
                else
                {
                    inWindow = evaluate<other, depth-1>();
                }
                
                allPieces[color] ^= from;
                individualPieces[attackingPiece] ^= from;
                empty ^= from;
                attackers &= attackers - 1;
            }

            allPieces[color] ^= to;
            allPieces[other] ^= to;
            individualPieces[attackingPiece] ^= to;
            individualPieces[attackedPiece] ^= to;
            targets &= targets - 1;
        }

        return true;
    }

    template<Color color, Piece piece, int depth>
    bool Position::evaluateNonCaptures()
    {
        auto constexpr other = (color == WHITE ? BLACK : WHITE); 
        auto constexpr promotionRank = ((color == WHITE) ? RANKS[SquaresPerFile-2] : RANKS[1]);

        auto movers = allPieces[color] & individualPieces[piece];
        bool inWindow = true;

        while(movers && inWindow)
        {
            auto const mover = ffs(movers);
            auto const from = A1 << mover;
            allPieces[color] ^= from;
            individualPieces[piece] ^= from;
            empty ^= from;

            auto targets = generateNonCaptureSquares<color, piece>(mover);

            while(targets && inWindow)
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
                            promotedPiece != static_cast<int>(PAWN) && inWindow;
                            --promotedPiece)
                        {
                            individualPieces[promotedPiece] ^= to;
                            inWindow = evaluate<other, depth-1>();
                            individualPieces[promotedPiece] ^= to;
                        }
                    }
                    else
                    {
                        individualPieces[PAWN] ^= to;
                        inWindow = evaluate<other, depth-1>();
                        individualPieces[PAWN] ^= to;
                    }                     
                }
                else
                {
                    individualPieces[piece] ^= to;
                    inWindow = evaluate<other, depth-1>();
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
        
        return true;
    }

    template<Color color, Piece piece>
    BitBoard Position::generateCaptureSquares(Square const origin) const
    {
        auto constexpr other = (color == WHITE ? BLACK : WHITE);

        BitBoard reachable { EMPTY };

        if constexpr(piece == PAWN)
        {
            reachable = PawnAttacks<color>[origin];        
        }
        
        if constexpr(piece == KNIGHT)
        {
            reachable = KnightAttacks[origin];
        }
        
        if constexpr(piece == BISHOP || piece == QUEEN)
        {  
            // pext is somewhat expensive: check diagonals first
            if(Diagonals[origin] & allPieces[other] & individualPieces[piece])
            {
                reachable = DiagonalAttacks[origin][pext(~empty, DiagonalMasks[origin])];
            }
        }
        
        if constexpr(piece == ROOK || piece == QUEEN)
        {
            // pext is somewhat expensive: check diagonals first
            if(RanksAndFiles[origin] & allPieces[other] & individualPieces[piece])
            {
                reachable |= (RankAttacks[origin][pext(~empty, RankMasks[origin])]
                            | FileAttacks[origin][pext(~empty, FileMasks[origin])]);
            }        
        }

        if constexpr(piece == KING)
        {
            reachable = KingAttacks[origin];
        }

        return reachable & allPieces[other];
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