#pragma once

#include "Position.hpp"

#include <array>

namespace spezi
{
    class UCI
    {
        public:
            void run();

        private:
            enum UCIState
            {
                Initial,
                Ready,
                Busy,
                Ended
            };

            static std::string readCommandFromGui();
            static void writeCommandToGui(std::string line);

            void processCommandFromGui(std::string line);

            // commands
            void uci();
            void debug(bool on);
            void isready();
            void setoption(std::string name, std::string value);
            void ucinewgame();
            void position(std::string fen,
                            std::vector<std::string>::const_iterator movesBegin,
                            std::vector<std::string>::const_iterator movesEnd);

            void go();
            void stop();
            void ponderhit();
            void quit();

            // go sub commands
            void searchmoves(std::vector<std::string>::const_iterator & begin,
                                std::vector<std::string>::const_iterator end);
            void ponder();
            void wtime(int milliseconds);
            void btime(int milliseconds);
            void winc(int milliseconds);
            void binc(int milliseconds);
            void movestogo(int moves);           
            void depth(int plies);
            void nodes(int nodes);
            void mate(int moves);
            void movetime(int milliseconds);
            void infinite();

            // other
            void interrupt();
            // position sub commands 
            std::string fen(std::vector<std::string>::const_iterator & firstFenSection);
     
            // private fields
            bool debugging {false};
            
            Position p {startingFen, writeCommandToGui};
           
            EvaluationParameters goParameters;
            
            UCIState uciState {Initial};
    };
}