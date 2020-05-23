#include "Position.hpp"

#include <algorithm>
#include <array>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

//#define PERFT

namespace spezi
{
    namespace
    {
        // returns the position in the history table where the move 
        // sideToMove is about to make should be stored
        auto constexpr historyIndex(int const fullMoves, Color const sideToMove)
        {
            return (fullMoves - 1) * 2 + sideToMove;
        }

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

        static inline unsigned char castlingCaptureUpdateFlags(BitBoard const from, BitBoard const to)
        {
            auto const wKing = popcount(E1 & from);
            auto const bKing = popcount(E8 & from);
            auto const K = popcount(H1 & (from | to)) | wKing;
            auto const Q = popcount(A1 & (from | to)) | wKing;
            auto const k = popcount(H8 & (from | to)) | bKing;
            auto const q = popcount(A8 & (from | to)) | bKing;
            return 0xF & ~(K | (Q << 1) | (k << 2) | (q << 3));
        }

        template<typename IntegerType>
        static inline IntegerType absInc(IntegerType const i)
        {
            return i + (i > 0) - (i < 0);
        }
        
        template<typename IntegerType>
        static inline IntegerType absDec(IntegerType const i)
        {
            return i - (i > 0) + (i < 0);
        }

        template<typename IntegerType>
        static inline IntegerType absAdd(IntegerType const i, IntegerType const j)
        {
            return i + ((i > 0) - (i < 0)) * j;
        }

        std::string scoreString(MilliSquare const evaluation, Color const sideToMove)
        {
            if(evaluation > -MaxExpectedMobility && evaluation < MaxExpectedMobility)
            {
                return " score cp " + std::to_string(evaluation * 100 / PawnUnit * (sideToMove==WHITE ? 1 : -1));
            }

            auto const movesToMate = evaluation > 0 ? 
                std::to_string((MateValue - evaluation) / 2)
                : std::to_string((evaluation + MateValue) / 2);
            
            if((sideToMove == WHITE && evaluation > 0) || (sideToMove == BLACK && evaluation < 0))
            {
                return " score mate " + movesToMate;
            }     

            return " score mate -" + movesToMate;
        }
    } 

    Position::Position(std::string fen, std::function<void(std::string)> outputFunction)
     :  lastInfoSentTimePoint(std::chrono::steady_clock::now()),
        engineToGuiOutputFunction(std::move(outputFunction))
    {
        setFen(std::move(fen));   
    }

    void Position::setFen(std::string fen)
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

        std::string const allowedCastling[] =
        { 
            "-", "K", "Q", "k", "q", "KQ", "Kk", "Kq", "Qk", "Qq", "kq",
            "KQk", "KQq", "Kkq", "Qkq", "KQkq"
        }; 

        bool castlingError = true;
        for(auto const & s : allowedCastling)
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

        if(sections[3] != "-")
        {
            if(sections[3].size() != 2
                || sections[3][0] < 'a'
                || sections[3][0] > 'h'
                || sections[3][1] < '1' 
                || sections[3][1] > '8')
            {
                throw std::runtime_error("Illegal en passant square: " + sections[3]);
            }

            enPassant = FILES[sections[3][0]-'a'] & RANKS[sections[3][1]-'1'];
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

        auto const historySize = historyIndex(fullMoves, sideToMove);

        for(auto index = 0; index != historySize; ++index)
        {
            // empty history up until the played number of moves
            history[index] = ZKey{};
        }

        zKey = sideToMove == WHITE ? 0 : BlackToMoveKey;
        zKey ^= CastlingKeys[castlingRights];
        zKey ^= zKeyFromPieceBoard(pieceBoard(empty, allPieces, individualPieces));
    }

    void Position::makeMoves(std::vector<std::string>::const_iterator begin, std::vector<std::string>::const_iterator const end)
    {
        while(begin != end)
        {
            makeMove(*begin);
            ++begin;
        }
    }

    void Position::setHashTableSize(unsigned int const megaByte)
    {
        if(megaByte == 0)
        {
            throw std::runtime_error("hash size 0 is not allowed");
        }
        transpositionTable = HashTable{MB * megaByte};
    }
    
    void Position::setMaxNumberOfNullMoves(unsigned int const maxNumberOfConsecutiveNullMoves)
    {
        if(maxNumberOfConsecutiveNullMoves > 2)
        {
            throw std::runtime_error("cannot have more than two consecutive null moves");
        }
        maxConsecutiveNullMoves = maxNumberOfConsecutiveNullMoves;
    }

    void Position::setMaxQuiescenceDepth(unsigned int quiescenceDepth)
    {
        if(quiescenceDepth > MAX_QUIESCENCE_DEPTH)
        {
            throw std::runtime_error("cannot have quiescence depth > 64");
        }
        maxQuiescenceDepth = quiescenceDepth;
    }

    void Position::clearHashTable()
    {
        transpositionTable.clear();
    }

    void Position::interrupt()
    {
        if(interruptState == Busy)
        {
            interruptState = Interrupted;
        }

        while(interruptState!=Idle)
        {
            std::this_thread::sleep_for(INTERRUPT_INTERVAL);
        };
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

    std::string Position::getBoardDisplay(int const indent) const
    {
        auto const board = pieceBoard(empty, allPieces, individualPieces);
        
        char const p[] = {'*', 'N', 'B', 'R', 'Q', 'K','+', 'n', 'b', 'r', 'q', 'k', '.', 'E'};         
        char const v = '|'; char const h = '-';
        char const ul = '/'; char const ur = '\\';
        char const ll = '\\'; char const lr = '/';
        
        auto const offset = std::string(indent, ' ');
        auto const bar =  std::string(17, h);
        std::string space = " ";
        std::string newline = "\n";

        std::string blackToMove = sideToMove == BLACK ? "  <<" : "";
        std::string whiteToMove = sideToMove == WHITE ? "  <<" : "";
        std::string boardDisplay = offset + "  a b c d e f g h" + blackToMove + newline;
        boardDisplay += offset + ul + bar + ur;
        for(int rank = 7; rank >= 0; --rank)
        {
            boardDisplay += newline + offset + v;
            for(int file = 0; file < 8; ++file)
            {
                boardDisplay += space + p[board[rank*8+file]];
            } 
            boardDisplay += space + v + space + std::to_string(rank+1);
        }
        boardDisplay += newline + offset + ll + bar + lr + newline;
        boardDisplay += offset + "  a b c d e f g h" + whiteToMove + newline;
       
        return boardDisplay;
    }

    std::string Position::getPrincipalVariationII() const
    {
        int depth = 0;
        std::string result = "";

        while(true)
        {
            auto const entry = principalVariation[depth];
        
            auto const score = entry.value<MilliSquare, HashEntry::SCORE_MASK>();
            auto const draft = entry.value<int, HashEntry::DRAFT_MASK>();

            if((score / MateValue == 1) ||
                (depth == MAX_DEPTH_ARRAY_SIZE) || (score / MaxExpectedMobility == 0 && draft <= 0))
            {
                break;
            }
 
            auto const move = entry.getUciNotation();
            if(move == "a1a1 ")
            {
                // needs work :)
                break;
            }

            result += entry.getUciNotation() + " "; 
            ++depth;
        }
        return result;
    }

    std::string Position::getPrincipalVariation() const
    {
        auto entry = principalVariationTable.get(zKey);
        auto key = zKey;
        auto side = sideToMove;
        std::string result = "";

        auto counter = 0;

        while(entry.zKey == key)
        {
            auto const score = entry.value<MilliSquare, HashEntry::SCORE_MASK>();
            auto const draft = entry.value<int, HashEntry::DRAFT_MASK>();

            if(score / MateValue == 1   // do not show terminal (illegal) position   
                || (score / MaxExpectedMobility == 0 && draft < 0)
                || ++counter > 100      // do not show quiescence moves if not mating
            )
            {
                break;
            }

            auto const tentativeMove = entry.getLongAlgebraicNotation() + "at depth " + 
                std::to_string(draft) + " with score " + std::to_string(score) + "\n";
                result += tentativeMove;
       
            auto const movedPiece = entry.value<Piece, HashEntry::MOVED_PIECE_MASK>();
            auto const capturedPiece = entry.value<Piece, HashEntry::CAPTURED_PIECE_MASK>();
            auto const promotedPiece = entry.value<Piece, HashEntry::PROMOTED_PIECE_MASK>();
            auto const origin = entry.value<Square, HashEntry::ORIGIN_SQUARE_MASK>();
            auto const target = entry.value<Square, HashEntry::TARGET_SQUARE_MASK>();
            auto const epBefore = entry.value<BitBoard, HashEntry::EN_PASSANT_BEFORE_MASK>();
            auto const epAfter = entry.value<BitBoard, HashEntry::EN_PASSANT_AFTER_MASK>();
            auto const castlingBefore = entry.value<unsigned char, HashEntry::CASTLING_BEFORE_MASK>();
            auto const castlingUpdate = entry.value<unsigned char, HashEntry::CASTLING_UPDATE_MASK>();

            key ^= BlackToMoveKey;
            key ^= PieceKeys[side][movedPiece][origin];

            if(promotedPiece != KING)
            {
                key ^= PieceKeys[side][promotedPiece][target];
            }
            else
            {
                key ^= PieceKeys[side][movedPiece][target];
            }

            if(capturedPiece != KING)
            {
                if(epBefore && target == ffs(epBefore))
                {
                    auto const pawn = target + ((side << 1) - 1) * SquaresPerRank;
                    key ^= PieceKeys[(side + 1) % 2][capturedPiece][pawn];
                }
                else
                {
                    key ^= PieceKeys[(side + 1) % 2][capturedPiece][target];
                }
            }
            
            if(movedPiece == KING)
            {
                if(origin == e1 && target == g1)
                {
                    key ^= PieceKeys[side][ROOK][h1];
                    key ^= PieceKeys[side][ROOK][f1];
                }
                if(origin == e1 && target == c1)
                {
                    key ^= PieceKeys[side][ROOK][a1];
                    key ^= PieceKeys[side][ROOK][d1];
                }
                if(origin == e8 && target == g8)
                {
                    key ^= PieceKeys[side][ROOK][h8];
                    key ^= PieceKeys[side][ROOK][f8];
                }
                if(origin == e8 && target == c8)
                {
                    key ^= PieceKeys[side][ROOK][a8];
                    key ^= PieceKeys[side][ROOK][d8];
                }
            }

            key ^= epBefore ? EnPassantKeys[ffs(epBefore) % SquaresPerRank] : ZKey {0};
            key ^= epAfter ? EnPassantKeys[ffs(epAfter) % SquaresPerRank] : ZKey {0};
            key ^= CastlingKeys[castlingBefore];
            key ^= CastlingKeys[castlingBefore & castlingUpdate];

            side = static_cast<Color>((side + 1) % 2);

            entry = principalVariationTable.get(key);
        }

        return result;
    }

    EvaluationStatistics Position::evaluateRecursively(EvaluationParameters const & parameters)
    {
        if(parameters.depth > MAX_DEPTH)
        {
            throw std::runtime_error("depth " + std::to_string(parameters.depth) + " exceeds maximum depth");
        }

        evaluationParameters = parameters;
        if(parameters.depth == -1)
        {
            evaluationParameters.depth = MAX_DEPTH;
        }

        evaluationTargetTimePoint = getTargetTime( 
            sideToMove == WHITE ? MilliSeconds{evaluationParameters.wtime} : MilliSeconds{evaluationParameters.btime}, 
            sideToMove == WHITE ? MilliSeconds{evaluationParameters.winc} : MilliSeconds{evaluationParameters.binc},
            fullMoves - 1);

        interruptState = Busy;

        EvaluationStatistics result;
        std::string bestMovePonderString;
        std::string infoString;

        for(auto currentMaxDepth = 1; currentMaxDepth <= evaluationParameters.depth; currentMaxDepth +=1)
        {
            alphaRaises = 0;
            betaCutoffs = 0;
            cutEntries = 0;
            allEntries = 0;
            exactEntries = 0;
            cutHashes = 0;
            allHashes = 0;            
            exactHashes = 0;
            hashCutoffs = 0;
            nullMoveCutoffs = 0;
            pvEntries = 0;

            principalVariationTable.clear();

            auto const start = std::chrono::steady_clock::now();
            maxDepth = currentMaxDepth;

            alphaBetaAtDepth = {};
            alphaBetaAtDepth[WHITE][0] = LOSS[WHITE];   // initial alpha
            alphaBetaAtDepth[BLACK][0] = LOSS[BLACK];   // initial beta
            numberOfNodesAtDepth = {};

            // simulate starting from an earlier position to accomodate color switching in evaluate()
            sideToMove = static_cast<Color>(sideToMove ^ BLACK);
            fullMoves -= sideToMove;
            zKey^= BlackToMoveKey;
            evaluate(0);
            // revert simulation of earlier position
            zKey^= BlackToMoveKey;
            fullMoves += sideToMove;
            sideToMove = static_cast<Color>(sideToMove ^ BLACK);

            int64_t numberOfNodes = 0;
            int64_t numberOfQuiescenceNodes = 0;
            auto maximumReachedDepth = 0;

            if(interruptState == Interrupted)
            {
                break;
            }

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
            auto const duration = std::chrono::duration_cast<MilliSeconds>(stop - start); 

            result = 
            {
                alphaBetaAtDepth[sideToMove][0],
                currentMaxDepth,
                maximumReachedDepth,
                numberOfNodes,
                numberOfQuiescenceNodes,
                static_cast<float>(duration.count())/1e3f
            };

            infoString = "info depth "+std::to_string(result.maximumRegularDepth)
                            + " seldepth " + std::to_string(result.maximumReachedDepth)
                            + scoreString(result.evaluation, sideToMove)
                            + " nodes " + std::to_string(result.numberOfNodes)
                            + " nps " + std::to_string(static_cast<int>(result.numberOfNodes / result.seconds))
                            + " time " + std::to_string(static_cast<int>(result.seconds * 1000))
                            + " pv " + getPrincipalVariationII();
            engineToGuiOutputFunction(infoString);                
            bestMovePonderString = "bestmove " + principalVariation[0].getUciNotation() + " ponder " + principalVariation[1].getUciNotation();

            if(result.evaluation > MaxExpectedMobility || result.evaluation < -MaxExpectedMobility)
            {
                break;
            }

            if(!enoughTimeForDeeperSearch(evaluationTargetTimePoint, duration))
            {
                break;
            }
        }

        // send last info string for full search again,
        // send bestmove ponder string   
        engineToGuiOutputFunction(infoString);
        engineToGuiOutputFunction(bestMovePonderString);

       /* 
        std::cout<<"alpha raises:       "<<alphaRaises<<std::endl;
        std::cout<<"beta cutoffs:       "<<betaCutoffs<<std::endl;
        std::cout<<"cut entries:        "<<cutEntries<<std::endl;
        std::cout<<"all entries:        "<<allEntries<<std::endl;
        std::cout<<"exact entries:      "<<exactEntries<<std::endl;
        std::cout<<"cut hashes:         "<<cutHashes<<std::endl;
        std::cout<<"all hashes:         "<<allHashes<<std::endl;
        std::cout<<"exact hashes:       "<<exactHashes<<std::endl;
        std::cout<<"hash cutoffs:       "<<hashCutoffs<<std::endl;
        std::cout<<"null move cutoffs:  "<<nullMoveCutoffs<<std::endl;
        std::cout<<"pv entries:         "<<pvEntries<<std::endl;
        std::cout<<"pv misses:          "<<pvMisses<<std::endl;*/

        interruptState = Idle;

        return result;
    }   

    void Position::evaluate(int const depth)
    {  
        if(checkAbortingConditions())
        {
            return;
        }

        auto const nodesAtEntry = numberOfNodesAtDepth[depth + 1];   
        ++numberOfNodesAtDepth[depth];

        auto const quiescence = depth >= maxDepth;
        auto const other = sideToMove;

        fullMoves += sideToMove;
        sideToMove = static_cast<Color>(sideToMove ^ BLACK);
        zKey ^= BlackToMoveKey;

#ifndef PERFT
        // make early exit checks (repetition, transposition, legality) from least to most expensive
        if(repetition())
        {
            alphaBetaAtDepth[sideToMove][depth] = DRAW;
            goto exit;
        }

        // TODO: check 50 move rule here!

        if(depth > 0)
        {
            alphaBetaAtDepth[WHITE][depth] = absInc(alphaBetaAtDepth[WHITE][depth-1]);
            alphaBetaAtDepth[BLACK][depth] = absInc(alphaBetaAtDepth[BLACK][depth-1]);
        }

        history[historyIndex(fullMoves, sideToMove)] = zKey;

        hashEntryAtDepth[depth] = {}; 
        if(!evaluateHashMove(depth))
        {
            goto exit;
        }
#endif
        if(isAttacked(sideToMove, ffs(allPieces[other] & individualPieces[KING])))
        {
            alphaBetaAtDepth[sideToMove][depth] = LOSS[other];
            --numberOfNodesAtDepth[depth];     // do not count illegal positions
            goto exit;
        }        

#ifdef PERFT
        if(quiescence)
        {
            goto exit;
        }
#endif

        if(quiescence)
        {
            auto const inCheck = isAttacked(other, ffs(allPieces[sideToMove] & individualPieces[KING]));
            auto const score = evaluateStatically();//*/pawnUnitsOnBoard();
           
            alphaBetaAtDepth[sideToMove][depth] = score;

            if(!inCheck)
            {
                // never cut or terminate quiescence search if we are in check
                auto const sign = (other << 1) - 1;
                if(sign * score >= sign * alphaBetaAtDepth[other][depth])
                {
                    // beta cutoff;
                    alphaBetaAtDepth[sideToMove][depth] = alphaBetaAtDepth[other][depth];
                    goto exit;
                }
                if(depth - maxDepth == maxQuiescenceDepth)
                {
                    goto exit;
                }
            }

            if(evaluateCaptures(depth)  // captures did not produce beta cutoff and
                && numberOfNodesAtDepth[depth + 1] == nodesAtEntry // no legal captures
                && inCheck)
            {
                // proceed with evaluation of non-captures to evade checks
                // first, reset current alpha to MateValue (otherwise program will believe
                // there already is a move with the static evaluation, which is not true) 
                alphaBetaAtDepth[sideToMove][depth] = LOSS[sideToMove];
                evaluateNonCaptures(depth);
            }
        }
        else
        {
            auto const noNullMoveCutoff = evaluateNullMove(depth);
            auto const originalNullMoveDepth = nullMoveDepth;
            nullMoveDepth = 0;

            if(noNullMoveCutoff  // null move did not produce beta cutoff
                && evaluateCaptures(depth) // captures did not produce beta cutoff and              
                && evaluateNonCaptures(depth) // normal moves did not produce beta cutoff
                && numberOfNodesAtDepth[depth + 1] == nodesAtEntry) // no legal moves / captures
            {
                if(!isAttacked(other, ffs(allPieces[sideToMove] & individualPieces[KING])))
                {
                    alphaBetaAtDepth[sideToMove][depth] = DRAW;
                    hashEntryAtDepth[depth] = HashEntry(PV_NODE, zKey, maxDepth - depth, DRAW); 
                }
                else
                {
                    // TODO: brauch ich das wirklich hier (wg. null move?) vielleicht nur die zweite Zeile?
                    alphaBetaAtDepth[sideToMove][depth] = LOSS[sideToMove];
                    hashEntryAtDepth[depth] = HashEntry(PV_NODE, zKey, maxDepth - depth, LOSS[sideToMove]); 
                }
            }

            nullMoveDepth = originalNullMoveDepth;
        }

    switch(hashEntryAtDepth[depth].value<HashEntryType, HashEntry::TYPE_MASK>())
    {
        case CUT_NODE:
            transpositionTable.insert(hashEntryAtDepth[depth]);
            ++cutEntries;
            break;
        case ALL_NODE:
            ++allEntries;
            break;
        case PV_NODE:
            transpositionTable.insert(hashEntryAtDepth[depth]);
            ++exactEntries;
         /*   if(!quiescence)
            {
                if(principalVariationTable.insert(hashEntryAtDepth[depth]))
                {
                    ++pvEntries;
                }
                else
                {
                    ++pvMisses;
                }
            }*/
            if(!nullMovesOnBranch)
            {
                storePrincipalVariation(hashEntryAtDepth[depth], depth);
            }
            break;
    }

    exit:
        zKey ^= BlackToMoveKey;
        sideToMove = other;
        fullMoves -= sideToMove;


    }

    bool Position::repetition()
    {
        auto constexpr minimalFullMovesToLookBack = 2;
        auto const thisMoveIndex = historyIndex(fullMoves, sideToMove);
        auto maximalFullMovesToLookBack = halfMoves / 2;
        for(auto fullMovesToLookBack = minimalFullMovesToLookBack; 
            fullMovesToLookBack <= maximalFullMovesToLookBack;
            ++fullMovesToLookBack)
        {
            if(history[thisMoveIndex - fullMovesToLookBack * 2] == zKey)
            {
                return true;
            }
        }
        return false;
    }

    bool Position::isAttacked(Color const attacking, Square const square)
    {
        return (PawnAttacks[attacking ^ BLACK][square] & allPieces[attacking] & individualPieces[PAWN])
            || (KnightAttacks[square] & allPieces[attacking] & individualPieces[KNIGHT])
            || (DiagonalAttacks[square][pext(~empty,DiagonalMasks[square])] & allPieces[attacking]
                & (individualPieces[BISHOP] | individualPieces[QUEEN]))
            || (RankAttacks[square][pext(~empty, RankMasks[square])] & allPieces[attacking]
                & (individualPieces[ROOK] | individualPieces[QUEEN]))
            || (FileAttacks[square][pext(~empty, FileMasks[square])] & allPieces[attacking]
                & (individualPieces[ROOK] | individualPieces[QUEEN]))
            || (KingAttacks[square] & allPieces[attacking] & individualPieces[KING]);
    }   

    MilliSquare Position::pawnUnitsOnBoard() const
    {
        auto const whitePieces =
            popcount(allPieces[WHITE] & individualPieces[QUEEN]) * 9 +
            popcount(allPieces[WHITE] & individualPieces[ROOK]) * 5 +
            popcount(allPieces[WHITE] & (individualPieces[BISHOP] | individualPieces[KNIGHT])) * 3 +
            popcount(allPieces[WHITE] & individualPieces[PAWN]);

        auto const blackPieces =
            popcount(allPieces[BLACK] & individualPieces[QUEEN]) * 9 +
            popcount(allPieces[BLACK] & individualPieces[ROOK]) * 5 +
            popcount(allPieces[BLACK] & (individualPieces[BISHOP] | individualPieces[KNIGHT])) * 3 +
            popcount(allPieces[BLACK] & individualPieces[PAWN]);

        auto constexpr center = D4|E4|D5|E5;
        auto constexpr extendedCenter = C3|D3|E3|F3|F4|F5|F6|E6|D6|C6|C5|C4;

        auto const whiteCenter = popcount(allPieces[WHITE] & center); 
        auto const whiteExtendedCenter = popcount(allPieces[WHITE] & extendedCenter); 
        
        auto const blackCenter = popcount(allPieces[BLACK] & center); 
        auto const blackExtendedCenter = popcount(allPieces[BLACK] & extendedCenter); 
        
        return (whitePieces - blackPieces) * PawnUnit + (whiteCenter - blackCenter) * PawnUnit / 3 + (whiteExtendedCenter - blackExtendedCenter) * PawnUnit / 9;
    }

    MilliSquare Position::evaluateStatically() const
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

    bool Position::evaluateHashMove(int const depth)
    {
        auto entry = transpositionTable.get(zKey);
        if(zKey == entry.zKey) 
        {
            if(entry.value<int, HashEntry::DRAFT_MASK>() >= maxDepth - depth)
            {
                auto const score = entry.value<MilliSquare, HashEntry::SCORE_MASK>();
                if(entry.value<HashEntryType, HashEntry::TYPE_MASK>() == PV_NODE)
                {              
                    // exact score => record score and return immediately  
                    ++exactHashes;
                    alphaBetaAtDepth[sideToMove][depth] = score;
                    // important: because of the early exit in evaluate(...), 
                    // pv table is not updated there for exact hits in the hash table 
                    storePrincipalVariation(entry, depth);
                    return false;
                }
                else if(entry.value<HashEntryType, HashEntry::TYPE_MASK>() == CUT_NODE)
                {
                    ++cutHashes;
                    // cut node, score is a lower/upper bound if white/black to move 
                    // => check if cutoff still stands and return immediately if true
                    auto const other = sideToMove ^ BLACK;
                    auto const sign = (other << 1) - 1;     
                    if(score >= sign * alphaBetaAtDepth[other][depth])
                    {
                        ++hashCutoffs;
                        alphaBetaAtDepth[sideToMove][depth] = alphaBetaAtDepth[other][depth];
                        return false;
                    }
                }
                else
                {
                    ++allHashes;
                }
            }

            auto const movedPiece = entry.value<Piece, HashEntry::MOVED_PIECE_MASK>();
            auto const capturedPiece = entry.value<Piece, HashEntry::CAPTURED_PIECE_MASK>();
            auto const promotedPiece = entry.value<Piece, HashEntry::PROMOTED_PIECE_MASK>();
            auto const origin = entry.value<Square, HashEntry::ORIGIN_SQUARE_MASK>();
            auto const target = entry.value<Square, HashEntry::TARGET_SQUARE_MASK>();
            auto const epBefore = entry.value<BitBoard, HashEntry::EN_PASSANT_BEFORE_MASK>();
            auto const epAfter = entry.value<BitBoard, HashEntry::EN_PASSANT_AFTER_MASK>();
            auto const castlingBefore = entry.value<unsigned char, HashEntry::CASTLING_BEFORE_MASK>();
            auto const castlingUpdate = entry.value<unsigned char, HashEntry::CASTLING_UPDATE_MASK>();

            auto const halfMovesAtEntry = halfMoves;

            auto const from = A1 << origin;
            auto const to = A1 << target;

            zKey ^= PieceKeys[sideToMove][movedPiece][origin];
            allPieces[sideToMove] ^= from;
            allPieces[sideToMove] ^= to;
            individualPieces[movedPiece] ^=from;
            empty ^= from;

            zKey ^= PieceKeys[sideToMove]
                [(promotedPiece == KING) * movedPiece + (promotedPiece!=KING) * promotedPiece][target];
            individualPieces[(promotedPiece == KING) * movedPiece + (promotedPiece!=KING) * promotedPiece] ^= to;

            if(capturedPiece != KING)
            {
                halfMoves = 0;

                if(epBefore && target == ffs(epBefore))
                {
                    auto const pawn = target + ((sideToMove << 1) - 1) * SquaresPerRank;
                    zKey ^= PieceKeys[(sideToMove + 1) % 2][capturedPiece][pawn];
                    allPieces[(sideToMove + 1) % 2] ^= (A1 << pawn); 
                    individualPieces[capturedPiece] ^= (A1 << pawn);
                    empty ^= to;
                    empty ^= (A1 << pawn);
                }
                else
                {
                    zKey ^= PieceKeys[(sideToMove + 1) % 2][capturedPiece][target];
                    allPieces[(sideToMove + 1) % 2] ^= to;
                    individualPieces[capturedPiece] ^= to; 
                }
            }
            else
            {
                empty ^= to;
                halfMoves += movedPiece == PAWN ? -halfMoves : 1; 

                if(movedPiece == KING)
                {
                    if(origin == e1)
                    {
                        if(target == g1)
                        {
                            auto const rookSquares = F1 | H1;
                            allPieces[WHITE] ^= rookSquares;
                            individualPieces[ROOK] ^= rookSquares;
                            empty ^= rookSquares;
                            zKey ^= PieceKeys[WHITE][ROOK][f1];
                            zKey ^= PieceKeys[WHITE][ROOK][h1];
                        }
                        else if (target == c1)
                        {
                            auto const rookSquares = A1 | D1;
                            allPieces[WHITE] ^= rookSquares;
                            individualPieces[ROOK] ^= rookSquares;
                            empty ^= rookSquares;
                            zKey ^= PieceKeys[WHITE][ROOK][a1];
                            zKey ^= PieceKeys[WHITE][ROOK][d1];
                        }
                    }
                    else if(origin == e8)
                    {
                        if(target == g8)
                        {
                            auto const rookSquares = F8 | H8;
                            allPieces[BLACK] ^= rookSquares;
                            individualPieces[ROOK] ^= rookSquares;
                            empty ^= rookSquares;
                            zKey ^= PieceKeys[BLACK][ROOK][f8];
                            zKey ^= PieceKeys[BLACK][ROOK][h8];
                        }
                        else if(target == c8)
                        {
                            auto const rookSquares = A8 | D8;
                            allPieces[BLACK] ^= rookSquares;
                            individualPieces[ROOK] ^= rookSquares;
                            empty ^= rookSquares;
                            zKey ^= PieceKeys[BLACK][ROOK][A8];
                            zKey ^= PieceKeys[BLACK][ROOK][D8];
                        }
                    }
                }
            }

            zKey ^= epBefore ? EnPassantKeys[ffs(epBefore) % SquaresPerRank] : ZKey {0};
            zKey ^= epAfter ? EnPassantKeys[ffs(epAfter) % SquaresPerRank] : ZKey {0};
            zKey ^= CastlingKeys[castlingBefore];
            zKey ^= CastlingKeys[castlingBefore & castlingUpdate];

            castlingRights = castlingBefore & castlingUpdate;
            enPassant = epAfter;

            evaluate(depth + 1);
            auto const inWindow = updateWindowOrCutoff(entry.zKey, depth, castlingBefore, epBefore,
                                origin, target, movedPiece, capturedPiece, promotedPiece, castlingUpdate);

            // rollback
            zKey = entry.zKey;        
            allPieces[sideToMove] ^= from;
            allPieces[sideToMove] ^= to;
            individualPieces[movedPiece] ^=from;
            empty ^= from;

            individualPieces[(promotedPiece == KING) * movedPiece + (promotedPiece!=KING) * promotedPiece] ^= to;

            if(capturedPiece != KING)
            {
                if(epBefore && target == ffs(epBefore))
                {
                    auto const pawn = target + ((sideToMove << 1) - 1) * SquaresPerRank;
                    allPieces[(sideToMove + 1) % 2] ^= (A1 << pawn); 
                    individualPieces[capturedPiece] ^= (A1 << pawn);
                    empty ^= to;
                    empty ^= (A1 << pawn);
                }
                else
                {
                    allPieces[(sideToMove + 1) % 2] ^= to;
                    individualPieces[capturedPiece] ^= to; 
                }
            }
            else
            {
                empty ^= to;
               
                if(movedPiece == KING)
                {
                    if(origin == e1)
                    {
                        if(target == g1)
                        {
                            auto const rookSquares = F1 | H1;
                            allPieces[WHITE] ^= rookSquares;
                            individualPieces[ROOK] ^= rookSquares;
                            empty ^= rookSquares;
                        }
                        else if (target == c1)
                        {
                            auto const rookSquares = A1 | D1;
                            allPieces[WHITE] ^= rookSquares;
                            individualPieces[ROOK] ^= rookSquares;
                            empty ^= rookSquares;
                        }
                    }
                    else if(origin == e8)
                    {
                        if(target == g8)
                        {
                            auto const rookSquares = F8 | H8;
                            allPieces[BLACK] ^= rookSquares;
                            individualPieces[ROOK] ^= rookSquares;
                            empty ^= rookSquares;
                        }
                        else if(target == c8)
                        {
                            auto const rookSquares = A8 | D8;
                            allPieces[BLACK] ^= rookSquares;
                            individualPieces[ROOK] ^= rookSquares;
                            empty ^= rookSquares;
                        }
                    }
                }
            }

            castlingRights = castlingBefore;
            enPassant = epBefore;
            halfMoves = halfMovesAtEntry;

            return inWindow;
        }
        return true;
    }

    bool Position::evaluateNullMove(int const depth)
    {
#ifdef PERFT
        return true;
#endif
        auto constexpr R = 3;   // standard depth decrease R = 3 for null move heuristic
        
        if(nullMoveDepth == maxConsecutiveNullMoves)
        {
            return true;
        }            

        auto const other = sideToMove ^ BLACK;
        auto const sign = (other << 1) - 1;     
        auto const beta = absAdd(alphaBetaAtDepth[other][depth - nullMoveDepth * R], (nullMoveDepth + 1) * R);

        // - call evaluation with depth + 3 (instead of + 1)  
        // - with null window, we are only interested in beta cutoffs, not in alpha increases
        // - report beta cutoff with return value false, raise alpha to beta as with normal cutoff
        // - otherwise return true, move is searched normally after this call 

        auto const betaDec = absDec(beta);
        auto const alphaDec = betaDec - sign;   

        alphaBetaAtDepth[sideToMove][depth + R - 1] = alphaDec;
        alphaBetaAtDepth[other][depth + R - 1] = betaDec;

        auto const enPassantAtEntry = enPassant;
        enPassant = EMPTY;
        zKey ^= enPassantAtEntry ? EnPassantKeys[ffs(enPassantAtEntry) % SquaresPerRank] : ZKey {0};
        ++nullMoveDepth;
        ++nullMovesOnBranch;
        evaluate(depth + R);
        --nullMovesOnBranch;
        --nullMoveDepth;
        zKey ^= enPassantAtEntry ? EnPassantKeys[ffs(enPassantAtEntry) % SquaresPerRank] : ZKey {0};
        enPassant = enPassantAtEntry;

        auto const score = alphaBetaAtDepth[other][depth + R];
        if(sign * score >= sign * beta)
        {
            ++nullMoveCutoffs;
            alphaBetaAtDepth[sideToMove][depth] = absAdd(beta, -R);
            return false;
        }

        return true;
    }

    bool Position::evaluateCaptures(int const depth)
    {
        auto const other = sideToMove ^ BLACK;
        auto const promotionRank = (sideToMove == WHITE ? RANKS[SquaresPerFile-2] : RANKS[1]);

        auto const castlingRightsAtEntry = castlingRights;
        auto const zKeyAtEntry = zKey;
        zKey ^= enPassant ? EnPassantKeys[ffs(enPassant) % SquaresPerRank] : ZKey {0};
        auto const enPassantAtEntry = enPassant;
        enPassant = EMPTY;
        auto const halfMovesAtEntry = halfMoves;
        halfMoves = 0;

        bool inWindow = true;

        // MVV-LVA: queens first
        for(auto attackedPiece = static_cast<int>(QUEEN); 
            (attackedPiece >= static_cast<int>(PAWN)) && inWindow;
            --attackedPiece)
        {
            auto targets = allPieces[other] & individualPieces[attackedPiece];
            while(targets && inWindow)
            {
                auto const target = ffs(targets);

                auto const diagonalAttacks = 
                    Diagonals[target] & allPieces[sideToMove] & (individualPieces[BISHOP] | individualPieces[QUEEN]) ?
                    DiagonalAttacks[target][pext(~empty, DiagonalMasks[target])] : EMPTY;
                
                auto const rankAttacks = 
                    Ranks[target] & allPieces[sideToMove] & (individualPieces[ROOK] | individualPieces[QUEEN]) ?
                RankAttacks[target][pext(~empty, RankMasks[target])] : EMPTY;

                auto const fileAttacks = 
                    Files[target] & allPieces[sideToMove] & (individualPieces[ROOK] | individualPieces[QUEEN]) ?
                    FileAttacks[target][pext(~empty, FileMasks[target])] : EMPTY;

                auto const to = A1 << target;
                allPieces[sideToMove] ^= to;
                allPieces[other] ^= to;
                individualPieces[attackedPiece] ^= to;
		        zKey ^= PieceKeys[other][attackedPiece][target];
		
                // generate attackers by finding reverse color attacks from target square
                BitBoard attackers[NumberOfPieceTypes];
                attackers[PAWN] = PawnAttacks[other][target] & allPieces[sideToMove] & individualPieces[PAWN];
                attackers[KNIGHT] = KnightAttacks[target] & allPieces[sideToMove] & individualPieces[KNIGHT]; 
                attackers[BISHOP] = diagonalAttacks & allPieces[sideToMove] & individualPieces[BISHOP];
                attackers[ROOK] = (rankAttacks | fileAttacks) & allPieces[sideToMove] & individualPieces[ROOK];
                attackers[QUEEN] = (diagonalAttacks | rankAttacks | fileAttacks) & allPieces[sideToMove] & individualPieces[QUEEN];
                attackers[KING] = KingAttacks[target] & allPieces[sideToMove] & individualPieces[KING];

                // MVV-LVA: pawns first
                for(auto attackingPiece = static_cast<int>(PAWN); 
                    (attackingPiece != NumberOfPieceTypes) && inWindow;
                    ++attackingPiece)
                {
                    individualPieces[attackingPiece] ^= to;
                    zKey ^= PieceKeys[sideToMove][attackingPiece][target];

                    while(attackers[attackingPiece] && inWindow)
                    {
                        auto const attacker = ffs(attackers[attackingPiece]);
                        auto const from = A1 << attacker;
                        
                        allPieces[sideToMove] ^= from;
                        individualPieces[attackingPiece] ^= from;
                        empty ^= from;           
                        zKey ^= PieceKeys[sideToMove][attackingPiece][attacker];
                        zKey ^= CastlingKeys[castlingRights];     
                        auto const castlingUpdate = castlingCaptureUpdateFlags(from, to);         
                        castlingRights &= castlingUpdate;
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
                                inWindow = updateWindowOrCutoff(
                                    zKeyAtEntry, depth, castlingRightsAtEntry, enPassantAtEntry,
                                    attacker, target, static_cast<Piece>(attackingPiece),
                                    static_cast<Piece>(attackedPiece), 
                                    static_cast<Piece>(promotedPiece), castlingUpdate);
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
                            inWindow = updateWindowOrCutoff(
                                    zKeyAtEntry, depth, castlingRightsAtEntry, enPassantAtEntry,
                                    attacker, target, static_cast<Piece>(attackingPiece),
                                    static_cast<Piece>(attackedPiece));
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
		        zKey ^= PieceKeys[other][attackedPiece][target];
				
                targets &= targets - 1;
            }
        }

        if(enPassantAtEntry && inWindow)
        {
            auto const target = ffs(enPassantAtEntry);
            auto const pawn = A1 << (target + (SquaresPerRank * ((sideToMove << 1) - 1)));
            auto attackers = PawnAttacks[other][target] & allPieces[sideToMove] & individualPieces[PAWN];
            individualPieces[PAWN] ^= enPassantAtEntry;
            individualPieces[PAWN] ^= pawn;
            allPieces[sideToMove] ^= enPassantAtEntry;
            allPieces[other] ^= pawn;
            empty ^= (enPassantAtEntry ^ pawn);
	        zKey ^= PieceKeys[other][PAWN][ffs(pawn)];
	        zKey ^= PieceKeys[sideToMove][PAWN][target];
	        while(attackers && inWindow)
            {
                auto const attacker = ffs(attackers);
                auto const from = A1 << attacker;
                allPieces[sideToMove] ^= from;
                individualPieces[PAWN] ^= from;
                empty ^= from;
		        zKey ^= PieceKeys[sideToMove][PAWN][attacker];
                evaluate(depth + 1);
                inWindow = updateWindowOrCutoff(
                    zKeyAtEntry, depth, castlingRightsAtEntry, enPassantAtEntry,
                    attacker, target, PAWN, PAWN);
                allPieces[sideToMove] ^= from;
                individualPieces[PAWN] ^= from;
                empty ^= from;
		        zKey ^= PieceKeys[sideToMove][PAWN][attacker];
				
                attackers &= attackers - 1;
            }
            individualPieces[PAWN] ^= enPassantAtEntry;
            individualPieces[PAWN] ^= pawn;
            allPieces[sideToMove] ^= enPassantAtEntry;
            allPieces[other] ^= pawn;
            empty ^= (enPassantAtEntry ^ pawn);
	        zKey ^= PieceKeys[other][PAWN][ffs(pawn)];
	        zKey ^= PieceKeys[sideToMove][PAWN][target];	    
        }

        enPassant = enPassantAtEntry;
        zKey ^= enPassant ? EnPassantKeys[ffs(enPassant) % SquaresPerRank] : ZKey {0};
        halfMoves = halfMovesAtEntry;

        return inWindow;
    }

    bool Position::evaluateNonCaptures(int const depth)
    {
        auto const promotionRank = ((sideToMove == WHITE) ? RANKS[SquaresPerFile-2] : RANKS[1]);
      
        auto const castlingRightsAtEntry = castlingRights;
        unsigned char castlingUpdate = 0xF;
        auto const zKeyAtEntry = zKey;
        zKey ^= enPassant ? EnPassantKeys[ffs(enPassant) % SquaresPerRank] : ZKey{0};
        auto const enPassantAtEntry = enPassant;
        enPassant = EMPTY;
        auto const halfMovesAtEntry = halfMoves;
        ++halfMoves;

        bool inWindow = true;

        for(auto movingPiece = static_cast<int>(PAWN);
            (movingPiece != NumberOfPieceTypes) && inWindow;
            ++movingPiece)
        {   
            auto movers = allPieces[sideToMove] & individualPieces[movingPiece];
            
            while(movers && inWindow)
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
                    castlingUpdate = castlingCaptureUpdateFlags(from, from);
                    castlingRights &= castlingUpdate;
                    zKey ^= CastlingKeys[castlingRights];
                }

                auto targets = generateNonCaptureSquares(static_cast<Piece>(movingPiece), mover);

                while(targets && inWindow)
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
                                inWindow = updateWindowOrCutoff(
                                    zKeyAtEntry, depth, castlingRightsAtEntry, enPassantAtEntry,
                                    mover, target, static_cast<Piece>(movingPiece),
                                    KING, static_cast<Piece>(promotedPiece));
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
                            inWindow = updateWindowOrCutoff(
                                    zKeyAtEntry, depth, castlingRightsAtEntry, enPassantAtEntry,
                                    mover, target, static_cast<Piece>(movingPiece));
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
                        inWindow = updateWindowOrCutoff(
                                    zKeyAtEntry, depth, castlingRightsAtEntry, enPassantAtEntry,
                                    mover, target, static_cast<Piece>(movingPiece), KING, KING, castlingUpdate);
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
                    castlingUpdate = 0xF;
                }
                
                movers &= movers - 1;
            }
        }

        auto const shift = sideToMove * 56;
        auto const other = static_cast<Color>(sideToMove ^ BLACK); 

        if(inWindow
            && (castlingRights & (1 << (sideToMove << 1)))
            && (((empty >> shift) & (F1|G1)) == (F1|G1))
            && !isAttacked(other, ffs(F1 << shift))
            && !isAttacked(other, ffs(E1 << shift)))
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
            inWindow = updateWindowOrCutoff(
                        zKeyAtEntry, depth, castlingRightsAtEntry, enPassantAtEntry,
                        e1 + shift, g1 + shift, KING, KING, KING, 0xC);
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
        if(inWindow &&
            (castlingRights & (2 << (sideToMove << 1)))
            && (((empty >> shift) & (B1|C1|D1)) == (B1|C1|D1))
            && !isAttacked(other, ffs(D1 << shift))
            && !isAttacked(other, ffs(E1 << shift)))
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
            inWindow = updateWindowOrCutoff(
                        zKeyAtEntry, depth, castlingRightsAtEntry, enPassantAtEntry,
                        e1 + shift, c1 + shift, KING, KING, KING, 0x3);
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

        return inWindow;
    }

    bool Position::updateWindowOrCutoff(
        ZKey originalZKey,
        int depth,
        unsigned char originalCastling,
        BitBoard originalEnPassant,
        Square origin, 
        Square target,
        Piece moved,
        Piece captured,
        Piece promoted,            
        unsigned char castlingUpdate)
    {
#ifdef PERFT
        return true;
#endif 
        auto const other = sideToMove ^ BLACK;
        auto const score = absDec(alphaBetaAtDepth[other][depth + 1]);
        auto const sign = (other << 1) - 1;
 
        if(sign * score >= sign * alphaBetaAtDepth[other][depth])   
        {
            // cutoff
            alphaBetaAtDepth[sideToMove][depth] = alphaBetaAtDepth[other][depth];

            //if(depth<maxDepth)
            //{
                hashEntryAtDepth[depth] = HashEntry(CUT_NODE, originalZKey, maxDepth - depth, 
                    score, originalCastling, originalEnPassant, origin, target, 
                    moved, captured, promoted, castlingUpdate);
            //}
            ++betaCutoffs;

            return false;
        }

        if(sign * score > sign * alphaBetaAtDepth[sideToMove][depth])
        {
            if(depth == 0)
            {
                hashEntryAtDepth[depth] = {};
            }

            // raise alpha / lower beta
            alphaBetaAtDepth[sideToMove][depth] = score;

            ++alphaRaises;

//            if(depth < maxDepth)
//            {
                hashEntryAtDepth[depth] = HashEntry(PV_NODE, originalZKey, maxDepth - depth, 
                    score, originalCastling, originalEnPassant, origin, target, 
                    moved, captured, promoted, castlingUpdate);
//            }
        }
        return true;
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

    void Position::storePrincipalVariation(HashEntry const hashEntry, int const depth)
    {
        auto firstMovePointer = &principalVariation[depth * MAX_DEPTH_ARRAY_SIZE - (depth * (depth - 1)) / 2];
        auto const lengthOfPrincipalVariation = MAX_DEPTH_ARRAY_SIZE - depth;
        *firstMovePointer = hashEntry;          
        std::copy(
            firstMovePointer + lengthOfPrincipalVariation,
            firstMovePointer + lengthOfPrincipalVariation * 2 - 1, 
            firstMovePointer + 1);            
    }

    bool Position::checkAbortingConditions()
    {
        if(interruptState == Interrupted)
        {
            return true;
        } 

        auto const now = std::chrono::steady_clock::now();
        if(now > evaluationTargetTimePoint)
        {
            interruptState = Interrupted;
            return true;
        }
        return false;
    }
   
    void Position::sendInfo()
    {
        auto const now = std::chrono::steady_clock::now();
        auto const timeSinceLastInfoSent = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastInfoSentTimePoint);

        if(timeSinceLastInfoSent < INFO_INTERVAL)
        {
            return;
        }
    }

    void Position::makeMove(std::string const & uciNotation)
    {
        auto const origin = uciNotation[0] - 'a' + (uciNotation[1] - '1') * SquaresPerRank;
        auto const target = uciNotation[2] - 'a' + (uciNotation[3] - '1') * SquaresPerRank; 

        auto const from = A1 << origin;
        auto const to = A1 << target;

        auto const movedPiece = individualPieces[PAWN] & from ? PAWN
                                : individualPieces[KNIGHT] & from ? KNIGHT
                                : individualPieces[BISHOP] & from ? BISHOP
                                : individualPieces[ROOK] & from ? ROOK
                                : individualPieces[QUEEN] & from ? QUEEN
                                : KING;
        auto const capturedPiece = (individualPieces[PAWN] & to) || (enPassant && target == ffs(enPassant))? PAWN
                                : individualPieces[KNIGHT] & to ? KNIGHT
                                : individualPieces[BISHOP] & to ? BISHOP
                                : individualPieces[ROOK] & to ? ROOK
                                : individualPieces[QUEEN] & to ? QUEEN
                                : KING;
        auto const promotedPiece = uciNotation.size() == 4 ? KING
                                : uciNotation[4] == 'n' ? KNIGHT
                                : uciNotation[4] == 'b' ? BISHOP
                                : uciNotation[4] == 'r' ? ROOK
                                : uciNotation[4] == 'q' ? QUEEN
                                : KING;

        zKey ^= PieceKeys[sideToMove][movedPiece][origin];
        allPieces[sideToMove] ^= from;
        allPieces[sideToMove] ^= to;
        individualPieces[movedPiece] ^=from;
        empty ^= from;

        zKey ^= PieceKeys[sideToMove]
            [(promotedPiece == KING) * movedPiece + (promotedPiece!=KING) * promotedPiece][target];
        individualPieces[(promotedPiece == KING) * movedPiece + (promotedPiece!=KING) * promotedPiece] ^= to;

        if(capturedPiece != KING)
        {
            halfMoves = 0;

            if(enPassant && target == ffs(enPassant))
            {
                auto const pawn = target + ((sideToMove << 1) - 1) * SquaresPerRank;
                zKey ^= PieceKeys[(sideToMove + 1) % 2][capturedPiece][pawn];
                allPieces[(sideToMove + 1) % 2] ^= (A1 << pawn); 
                individualPieces[capturedPiece] ^= (A1 << pawn);
                empty ^= to;
                empty ^= (A1 << pawn);
            }
            else
            {
                zKey ^= PieceKeys[(sideToMove + 1) % 2][capturedPiece][target];
                allPieces[(sideToMove + 1) % 2] ^= to;
                individualPieces[capturedPiece] ^= to; 
            }
        }
        else
        {
            empty ^= to;
            halfMoves += movedPiece == PAWN ? -halfMoves : 1; 

            if(movedPiece == KING)
            {
                if(origin == e1)
                {
                    if(target == g1)
                    {
                        auto const rookSquares = F1 | H1;
                        allPieces[WHITE] ^= rookSquares;
                        individualPieces[ROOK] ^= rookSquares;
                        empty ^= rookSquares;
                        zKey ^= PieceKeys[WHITE][ROOK][f1];
                        zKey ^= PieceKeys[WHITE][ROOK][h1];
                    }
                    else if (target == c1)
                    {
                        auto const rookSquares = A1 | D1;
                        allPieces[WHITE] ^= rookSquares;
                        individualPieces[ROOK] ^= rookSquares;
                        empty ^= rookSquares;
                        zKey ^= PieceKeys[WHITE][ROOK][a1];
                        zKey ^= PieceKeys[WHITE][ROOK][d1];
                    }
                }
                else if(origin == e8)
                {
                    if(target == g8)
                    {
                        auto const rookSquares = F8 | H8;
                        allPieces[BLACK] ^= rookSquares;
                        individualPieces[ROOK] ^= rookSquares;
                        empty ^= rookSquares;
                        zKey ^= PieceKeys[BLACK][ROOK][f8];
                        zKey ^= PieceKeys[BLACK][ROOK][h8];
                    }
                    else if(target == c8)
                    {
                        auto const rookSquares = A8 | D8;
                        allPieces[BLACK] ^= rookSquares;
                        individualPieces[ROOK] ^= rookSquares;
                        empty ^= rookSquares;
                        zKey ^= PieceKeys[BLACK][ROOK][A8];
                        zKey ^= PieceKeys[BLACK][ROOK][D8];
                    }
                }
            }
        }

        zKey ^= CastlingKeys[castlingRights];
        castlingRights &= castlingCaptureUpdateFlags(from, to);
        zKey ^= CastlingKeys[castlingRights];
        
        zKey ^= enPassant ? EnPassantKeys[ffs(enPassant) % SquaresPerRank] : ZKey {0};
      
        enPassant = (movedPiece == PAWN) ? (A1 << ((ffs(from) + ffs(to)) >> 1)) & Files[origin] : EMPTY;

        zKey ^= enPassant ? EnPassantKeys[ffs(enPassant) % SquaresPerRank] : ZKey {0};

        fullMoves += sideToMove;
        sideToMove = static_cast<Color>(sideToMove ^ BLACK);
        zKey ^= BlackToMoveKey;
    }
}
