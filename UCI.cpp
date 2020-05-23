#include "UCI.hpp"

#include <algorithm>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace spezi
{
    namespace
    {
        std::vector<std::string> const guiCommands
        {            
            "uci", "debug", "isready", "setoption", "ucinewgame",
            "position", "go", "stop", "ponderhit", "quit"
        };

        std::vector<std::string> splitLine(std::string line)
        { 
            std::istringstream lineStream(std::move(line));
            std::vector<std::string> tokens;
            while(true)
            {
                std::string token;
                if(!(lineStream>>token))
                {
                    break;
                }
                tokens.push_back(token);
            }
            return tokens;      
        }
    }

    void UCI::run()
    {
        while(uciState != Ended)
        {
            try
            {
                auto guiCommand = readCommandFromGui();
                processCommandFromGui(std::move(guiCommand));
            }
            catch(const std::exception& e)
            {
                writeCommandToGui(e.what());
            }
        }
    }
   
    std::string UCI::readCommandFromGui()
    {
        std::string line;
        std::getline(std::cin, line);
        return line;
    }

    void UCI::writeCommandToGui(std::string line)
    {
        // engine to gui commands will not come from the main thread,
        // => protect this to avoid garbling the output
        static std::mutex mtx;
        std::unique_lock<std::mutex> lock(mtx);
        std::cout<<line<<std::endl;
    }

    void UCI::processCommandFromGui(std::string commandLine)
    {
        auto args = splitLine(std::move(commandLine));
        size_t numberOfToken = 0;
        
        while(true)
        {
            if(std::find(guiCommands.begin(), guiCommands.end(),
                args[numberOfToken]) != guiCommands.end())
            {
                break;
            }
            else
            {
                if(++numberOfToken == args.size())
                {
                    // do nothing if we cannot decipher command
                    return;
                }
            }
        }

        // process uci command first (total reset)
        if(args[numberOfToken] == "uci")
        {   
            interrupt();
            uci();
            return;
        }

        // do not accept any command other than "uci" when not initialized
        if(uciState == Initial)
        {
            return;
        }

        // process commands that do not need to interrupt calculations

        if(args[numberOfToken] == "debug" && args.size() >= numberOfToken + 2)
        {
            if(args[numberOfToken + 1] == "on")
            {
                debug(true);
            }
            else if(args[numberOfToken + 1] == "off")
            {
                debug(false);
            }
            return;
        }

        if(args[numberOfToken] == "isready")
        {
            isready();
            return;
        }
       
        // any other command will have to interrupt running calculations, so do it once here

        interrupt();

        // do not accept any commands when not ready (although this should not happen at this point)
        if(uciState != Ready)
        {
            return;
        }
        
        if(args[numberOfToken] == "setoption" && args.size() >= numberOfToken + 5)
        {
            if(args[numberOfToken + 1] == "name"
                &&args[numberOfToken + 3] == "value")
            { 
                setoption(args[numberOfToken + 2], args[numberOfToken + 4]);
            }
        }
        else if(args[numberOfToken] == "ucinewgame")
        {
            ucinewgame();
        }
        else if(args[numberOfToken] == "position" && args.size() >= numberOfToken + 2)
        {
            auto iter = args.cbegin() + numberOfToken + 1;

            std::string positionAsFen;

            if(*iter == "startpos")
            {
                positionAsFen = startingFen;                
            }            
            else if(*iter == "fen" && args.size() >= numberOfToken + 8)
            {
                positionAsFen = fen(++iter);
            }
            else
            {
                throw std::runtime_error("error parsing position command: 'startpos' or 'fen' missing");
            }

            if(++iter != args.end() && *iter == "moves")
            {
                position(std::move(positionAsFen), ++iter, args.cend());
            }
            else
            {
                position(std::move(positionAsFen), args.cend(), args.cend());
            }
        }
        else if(args[numberOfToken] == "go" && args.size() >= numberOfToken + 2)
        {
            goParameters = EvaluationParameters{};
            auto iter = args.cbegin() + numberOfToken + 1; 
            while(iter!=args.cend())
            {
                if(*iter == "searchmoves")
                {
                    searchmoves(++iter, args.cend());
                }                        
                if(*iter == "ponder")
                {
                    ponder();
                }
                else if(*iter == "wtime" && ++iter != args.cend())
                {
                    wtime(std::stoul(*iter));
                }
                else if(*iter == "btime" && ++iter != args.cend())
                {
                    btime(std::stoul(*iter));
                }
                else if(*iter == "winc" && ++iter != args.cend())
                {
                    winc(std::stoul(*iter));
                }
                else if(*iter == "binc" && ++iter != args.cend())
                {
                    binc(std::stoul(*iter));
                }
                else if(*iter == "movestogo" && ++iter != args.cend())
                {
                    movestogo(std::stoul(*iter));
                }
                else if(*iter == "depth" && ++iter != args.cend())
                {
                    depth(std::stoul(*iter));
                }
                else if(*iter == "nodes" && ++iter != args.cend())
                {
                    nodes(std::stoul(*iter));
                }
                else if(*iter == "mate" && ++iter != args.cend())
                {
                    mate(std::stoul(*iter));
                }
                else if(*iter == "movetime" && ++iter != args.cend())
                {
                    movetime(std::stoul(*iter));
                }
                else if(*iter == "infinite")
                {
                    infinite();
                }
                else
                {
                    throw std::runtime_error("could not process go command");
                }
    
                ++iter;
            };  

            go();
        }
        else if(args[numberOfToken] == "stop")
        {
            stop();
        }
        else if(args[numberOfToken] == "ponderhit")
        {
            ponderhit();
        }
        else if(args[numberOfToken] == "quit")
        {
            quit();
        }
    } 

    void UCI::uci()
    {
        p.clearHashTable();

        writeCommandToGui("id name Spezi");
        writeCommandToGui("id name Roberto");
        writeCommandToGui("option name Hash type spin default 1 min 1 max 4096");
        writeCommandToGui("option name NullMoves type spin default 2 min 0 max 2");
        writeCommandToGui("option name QDepth type spin default 8 min 0 max 64");
        // Suppress faulty PV 
        writeCommandToGui("option name SuppressPV type check default false");
        writeCommandToGui("uciok");
    
        uciState = Ready;
    }

    void UCI::debug(bool on)
    {
        debugging = on;
    }
    
    void UCI::isready()
    {
        writeCommandToGui("readyok");
    }
    
    void UCI::setoption(std::string name, std::string value)
    {
        if(name == "Hash")
        {
            p.setHashTableSize(std::stoul(value));
        }
        else if(name == "NullMoves")
        {
            p.setMaxNumberOfNullMoves(std::stoul(value));
        }
        else if(name == "QDepth")
        {
            p.setMaxQuiescenceDepth(std::stoul(value));
        }
        else if(name == "SuppressPV")
        {
            p.setSuppressPv(value != "false");
        }
    }
    
    void UCI::ucinewgame()
    {
        p.clearHashTable();
    }
    
    void UCI::position(std::string fen,
                            std::vector<std::string>::const_iterator movesBegin,
                            std::vector<std::string>::const_iterator movesEnd)
    {
        p.setFen(std::move(fen));
        p.makeMoves(movesBegin, movesEnd);
    }
    
    void UCI::go()
    {
        uciState = Busy;

        auto evaluateAndCatchExceptions = [this](EvaluationParameters const & params)
        {
            try
            {
                p.evaluateRecursively(params); 
            }
            catch(std::exception const & e)
            {
                writeCommandToGui(e.what());
            }
        };         

        auto evaluationThread = std::thread(evaluateAndCatchExceptions, goParameters);
        evaluationThread.detach();
    }
    
    void UCI::stop()
    {
        /* nothing to do here, interrupt is already handled */
    }
    
    void UCI::ponderhit()
    {

    }
    
    void UCI::quit()
    {
        uciState = Ended; 
    }

    void UCI::searchmoves(std::vector<std::string>::const_iterator & /*begin*/,
                                std::vector<std::string>::const_iterator /*end*/)
    {

    }

    void UCI::ponder()
    {

    }

    void UCI::wtime(int const milliseconds)
    {
        goParameters.wtime = milliseconds;
    }

    void UCI::btime(int const milliseconds)
    {
        goParameters.btime = milliseconds;
    }

    void UCI::winc(int const milliseconds)
    {   
        goParameters.wtime = milliseconds;
    }

    void UCI::binc(int const milliseconds)
    {
        goParameters.binc = milliseconds;
    }

    void UCI::movestogo(int const moves)
    {
        goParameters.movestogo = moves;
    }

    void UCI::depth(int const plies)
    {
        goParameters.depth = plies;
    }

    void UCI::nodes(int const nodes)
    {
        goParameters.nodes = nodes;
    }

    void UCI::mate(int const moves)
    {
        goParameters.mate = moves;
    }

    void UCI::movetime(int const milliseconds)
    {
        goParameters.movetime = milliseconds;
    }

    void UCI::infinite()
    {
        /* nothing to do here, infinite search is the default */
    }

    void UCI::interrupt()
    {
        if(uciState == Busy) 
        {
            p.interrupt();
            uciState = Ready;
        }
    }

    std::string UCI::fen(std::vector<std::string>::const_iterator & firstFenSection)
    {
        return *firstFenSection + " " +
               *(++firstFenSection) + " " +
               *(++firstFenSection) + " " +
               *(++firstFenSection) + " " +
               *(++firstFenSection) + " " +
               *(++firstFenSection);
    }
}