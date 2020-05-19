#include "UCI.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
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
            processCommandFromGui(
                readCommandFromGui());
        }
    }
   
    std::string UCI::readCommandFromGui()
    {
        std::string line;
        std::getline(std::cin, line);
        return line;
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

        if(args[numberOfToken] == "uci")
        {   
            uci();
        }
        
        if(uciState != Ready)
        {
            return;
        }
        
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
        }
        else if(args[numberOfToken] == "isready")
        {
            isready();
        }
        else if(args[numberOfToken] == "setoption" && args.size() >= numberOfToken + 5)
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
            position(Started);
            auto iter = args.cbegin() + numberOfToken + 1;
            bool success = false;
            if(*iter == "startpos")
            {
                success = startpos();
            }            
            else if(*iter == "fen" && args.size() >= numberOfToken + 8)
            {
                success = fen(++iter);
            }

            if(success)
            {
                if(*iter == "moves" && args.size() >= numberOfToken + 3)
                {
                    success = moves(++iter, args.cend());
                }
            }

            position(success ? Finished : Aborted);
        }
        else if(args[numberOfToken] == "go" && args.size() >= numberOfToken + 2)
        {
            go(Started);
            auto iter = args.cbegin() + numberOfToken + 1; 
            while(iter!=args.cend())
            {
                bool success = false;
                if(*iter == "searchmoves")
                {
                    success = searchmoves(++iter, --args.cend());
                }
                else if(*iter == "ponder")
                {
                    success = ponder();
                }
                else if(*iter == "wtime" && ++iter != args.cend())
                {
                    success = wtime(std::stoi(*iter));
                }
                else if(*iter == "btime" && ++iter != args.cend())
                {
                    success = btime(std::stoi(*iter));
                }
                else if(*iter == "winc" && ++iter != args.cend())
                {
                    success = winc(std::stoi(*iter));
                }
                else if(*iter == "binc" && ++iter != args.cend())
                {
                    success = btime(std::stoi(*iter));
                }
                else if(*iter == "movestogo" && ++iter != args.cend())
                {
                    success = movestogo(std::stoi(*iter));
                }
                else if(*iter == "depth" && ++iter != args.cend())
                {
                    success = depth(std::stoi(*iter));
                }
                else if(*iter == "nodes" && ++iter != args.cend())
                {
                    success = nodes(std::stoi(*iter));
                }
                else if(*iter == "mate" && ++iter != args.cend())
                {
                    success = mate(std::stoi(*iter));
                }
                else if(*iter == "movetime" && ++iter != args.cend())
                {
                    success = movetime(std::stoi(*iter));
                }
                else if(*iter == "infinite")
                {
                    success = infinite();
                }
                
                if(!success)
                {
                    go(Aborted);
                    return;
                }
                ++iter;
            };  

            go(Finished);
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
        interrupt();

        if(uciState == Initial)
        {
            uciState = Ready;
        }
        
        writeCommandToGui("id name Spezi");
        writeCommandToGui("id name Roberto");
        writeCommandToGui("option name Hash type spin default 1 min 1 max 4096");
        writeCommandToGui("option name #Null type spin default 2 min 0 max 2");
        writeCommandToGui("uciok");
    }

    void UCI::debug(bool on)
    {
        debugging = on;
    }
    
    void UCI::isready()
    {
        if(uciState != Initial)
        {
            writeCommandToGui("readyok");
        }
    }
    
    void UCI::setoption(std::string name, std::string value)
    {
        if(name == "Hash")
        {
        }
        else if(name == "#Null")
        {
        }
    }
    
    void UCI::ucinewgame()
    {

    }
    
    void UCI::position(CommandState state)
    {
        if(pState != Started)
        {
            pState = Started;
        }
        else if(state == Finished)
        {
            pState = Finished;
        }
        else
        {
            pState = Aborted;
        }
    }
    
    void UCI::go(CommandState state)
    {
        if(gState != Started)
        {
            gState = Started;
        }
        else if(state == Finished)
        {
            gState = Finished;
        }
        else
        {
            gState = Aborted;
        }
    }
    
    void UCI::stop()
    {

    }
    
    void UCI::ponderhit()
    {

    }
    
    void UCI::quit()
    {
        uciState = Ended; 
    }

    void UCI::interrupt()
    {
        if(uciState == Initial 
            || uciState == Ready
            || uciState == Ended)
        {
            return;
        }

        uciState = Interrupted;
    }
}