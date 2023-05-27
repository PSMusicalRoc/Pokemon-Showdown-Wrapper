#include <iostream>
#include "ProcessID_Handler.h"
#include <vector>
#include <algorithm>

#include "PShowdownParser.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <filesystem>

#include "PK_Rand.h"

/**
 * Gonna have to do some major porting to use windows unfortunately.
*/

using json=nlohmann::json;

// Important things for Linux and Windows differentiation

void LoadTrainerData(std::string& battleFilename, json& playerdata, PIDTYPE handler, std::string& packed_team)
{
    std::ifstream battleFile(battleFilename.c_str());
    if (!battleFile.is_open())
    {
        std::cout << "Could not load battle file at " << battleFilename << std::endl;
        return;
    }
    
    std::string jsonText;
    std::string inLine;
    while (std::getline(battleFile, inLine))
    {
        jsonText += inLine;
    }

    battleFile.close();

    // first, populate a json object so we know what we're dealing with
    playerdata = json::parse(jsonText);

    std::string ppaste_loc = playerdata["teamloc"];

    std::ifstream teamfile(ppaste_loc);
    if (!teamfile.is_open())
    {
        std::cout << "Could not load battle file at " << ppaste_loc << std::endl;
        return;
    }

    std::string team;
    while (std::getline(teamfile, inLine))
    {
        team += inLine + "\n";
    }
    teamfile.close();

    std::filesystem::path base = std::filesystem::current_path();
    std::string innercommand = "/bin/cat " + base.string() + "/" + ppaste_loc + " | vendor/pokemon-showdown/pokemon-showdown pack-team";
    std::string command = "/bin/sh -c \"" + innercommand + "\"";
    if (!handler->CreateFork("packteam", command))
    {
        std::cerr << "Could not create a pack-team thread." << std::endl;
        return;
    }
    
    handler->ReadFromForkTimed("packteam", packed_team, 10.0f);
#ifdef ROC_DEBUG
    std::cout << packed_team << std::endl;
#endif
    packed_team.erase(std::remove(packed_team.begin(), packed_team.end(), '\n'), packed_team.cend());
    handler->EndFork("packteam");
}

void SetupAITrainerBattle(std::string& ai_filename, PIDTYPE handler,
    std::string& output, PShowdownParser& parser)
{
    json aitrainer, player;
    std::string aitrainer_team, player_team;
    std::string player_filename = "data/player/playertrainer.pkteam";
    LoadTrainerData(player_filename, player, handler, player_team);
    LoadTrainerData(ai_filename, aitrainer, handler, aitrainer_team);

    std::string ret;

    //std::cout << "Sending start" << std::endl;
    std::string command = ">start {\"formatid\":\"";
    command += aitrainer["format"];
    command += "\"}\n";
    handler->WriteToFork("showdown", command);
    //std::cout << "Getting output" << std::endl;
    handler->ReadFromFork("showdown", ret);
#ifdef ROC_DEBUG
    std::cout << ret << std::endl;
#endif
    //std::cout << "Parsing" << std::endl;
    output += parser.parsePShowdownOutput(ret);

    //std::cout << "Sending player p1" << std::endl;
    command = ">player p1 {\"name\":\"";
    command += player["class"];
    command += " ";
    command += player["name"];
    command += "\",\"team\":\"";
    command += player_team;
    command += "\"}\n";
    //std::cout << command << std::endl;
    handler->WriteToFork("showdown", command);
    //std::cout << "Getting output" << std::endl;
    handler->ReadFromFork("showdown", ret);
#ifdef ROC_DEBUG
    std::cout << ret << std::endl;
#endif
    //std::cout << "Parsing" << std::endl;
    output += parser.parsePShowdownOutput(ret);

    //std::cout << "Sending player p2" << std::endl;
    command = ">player p2 {\"name\":\"";
    command += aitrainer["class"];
    command += " ";
    command += aitrainer["name"];
    command += "\",\"team\":\"";
    command += aitrainer_team;
    command += "\"}\n";
    handler->WriteToFork("showdown", command);
    //std::cout << "Getting output" << std::endl;
    handler->ReadFromFork("showdown", ret);
#ifdef ROC_DEBUG
    std::cout << ret << std::endl;
#endif
    //std::cout << "Parsing" << std::endl;
    output += parser.parsePShowdownOutput(ret);

    //std::cout << "Returning" << std::endl;
}

void parse_cmd_to_vec(const std::string& cmd, std::vector<std::string>& outvec)
{
    int index = 0;
    while (index < cmd.size())
    {
        std::size_t newpos = cmd.find(' ', index);
        std::string substr;
        if (newpos == cmd.npos)
        {
            substr = cmd.substr(index);
        }
        else {
            substr = cmd.substr(index, newpos - index);
        }
        outvec.push_back(substr);
        if (newpos != cmd.npos)
            index = newpos + 1;
        else
            index = cmd.size();
    }
}

int main() {
    PIDTYPE handler = PIDHANDLER::Get();
    PShowdownParser parser("showdown", handler);

    if (!handler->CreateFork("showdown", "vendor/pokemon-showdown/pokemon-showdown simulate-battle"))
    {
        std::cerr << "Could not create Pokemon Showdown thread!" << std::endl;
        handler->DeleteHandler();
        return -1;
    }

    std::string incmd;
    while (true) {
        if (parser.OppSelectMove())
        {
            // logic here for opp to select move
            parser.OppSelectedMove();                
        }

        std::cout << "Enter input: ";
        std::getline(std::cin, incmd);
        if (incmd.empty()) continue;

        if (incmd == "exit") {break;}

        std::vector<std::string> cmd;
        parse_cmd_to_vec(incmd, cmd);

        if (cmd[0] == "start")
        {
            // format == "start [format]"
            std::string outcmd = ">start {\"formatid\":\"";
            outcmd += cmd[1];
            outcmd += "\"}\n";
            handler->WriteToFork("showdown", outcmd);
        }
        else if (cmd[0] == "setplayer")
        {
            // format: "setplayer [playernum] [playername]"
            std::string outcmd = ">player p";
            outcmd += cmd[1];
            outcmd += " {\"name\":\"";
            outcmd += cmd[2];
            outcmd += "\"}\n";
            handler->WriteToFork("showdown", outcmd);
        }
        else if (cmd[0] == "moves")
        {
            std::cout << parser.QueryMoves() << std::endl;
            continue;
        }
        else if (cmd[0] == "loadbattle")
        {
            std::string out;
            SetupAITrainerBattle(cmd[1], handler, out, parser);
            std::cout << out << std::endl;
        }
        else if (cmd[0] == "use")
        {
            //std::cout << "Using moves isn't quite ready yet :)" << std::endl;
            std::string outcmd = ">p1 move";
            for (int i = 1; i < cmd.size(); i++)
            {
                outcmd += " " + cmd[i];
            }
            outcmd += "\n";
            handler->WriteToFork("showdown", outcmd);
        }
        else if (cmd[0] == "switch")
        {
            std::string outcmd = ">p1 switch";
            for (int i = 1; i < cmd.size(); i++)
            {
                outcmd += " " + cmd[i];
            }
            outcmd += "\n";
            handler->WriteToFork("showdown", outcmd);
        }
        else continue;

        std::string output_str, full_out;
        handler->ReadFromForkTimed("showdown", output_str, 5.0f);
        full_out = output_str;
        //std::string parsed_string = parser.parsePShowdownOutput(output_str);
        while (output_str != "")
        {
            handler->ReadFromForkTimed("showdown", output_str, 1.5);
            full_out += output_str;
        }
        std::string parsed_string = parser.parsePShowdownOutput(full_out);
#ifdef ROC_DEBUG
        std::cout << full_out << std::endl;
#endif
        std::cout << std::endl << parsed_string << std::endl;

        /**
         * This part of the program deals with closing and
         * making sure that the child process closes correctly.
        */

        // Close pipes and terminate child process
        // @todo include close macro here
    }

    return 0;
}
