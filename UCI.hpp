#pragma once

#include "Position.hpp"

#include <array>

namespace spezi
{
    struct GoParameters
    {
        int wtime = -1;
        int btime = -1;
        int winc = -1;
        int binc = -1;
        int movestogo = -1;
        int depth = -1;
        int nodes = -1;
        int mate = -1;
        int movetime = -1;

        std::array<HashEntry, 256> movesFrom = {};
    };

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
                Interrupted,
                Ended
            };

            enum CommandState
            {
                Started,
                Finished,
                Aborted
            };

            std::string readCommandFromGui();
            void writeCommandToGui(std::string line);

            void processCommandFromGui(std::string line);

            // commands
            void uci();
            void debug(bool on);
            void isready();
            void setoption(std::string name, std::string value);
            void ucinewgame();
            void position(CommandState state);
            void go(CommandState state);
            void stop();
            void ponderhit();
            void quit();

            // position sub commands 
            bool startpos();
            bool fen(std::vector<std::string>::const_iterator & firstFenSection);
            bool moves(std::vector<std::string>::const_iterator begin,
                        std::vector<std::string>::const_iterator end);

            // go sub commands
            bool searchmoves(std::vector<std::string>::const_iterator & begin,
                                std::vector<std::string>::const_iterator end);
            bool ponder();
            bool wtime(int milliseconds);
            bool btime(int milliseconds);
            bool winc(int milliseconds);
            bool binc(int milliseconds);
            bool movestogo(int moves);           
            bool depth(int plies);
            bool nodes(int nodes);
            bool mate(int moves);
            bool movetime(int milliseconds);
            bool infinite();

            // other
            void interrupt();

            // private fields
            bool debugging {false};
            
            Position p;
            CommandState pState {Finished};
            
            GoParameters g;
            CommandState gState {Finished};
            
            UCIState uciState {Initial};
    };
}