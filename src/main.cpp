#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <string>
#include <fcntl.h>
#include <sys/wait.h>
#include <vector>
#include <algorithm>

#include "PShowdownParser.h"
#include <fstream>
#include <nlohmann/json.hpp>

using json=nlohmann::json;

void get_output(int outpipe, std::string& output_str)
{
    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(outpipe, &read_set);

    output_str = "";

    while (true) {
        ssize_t len;
        char output[2048];

        // Use select() to check if data is available to be read from out_pipe[0]
        timeval timeout = {0, 1500000};  // Wait up to 1.5 seconds
        int num_ready = select(outpipe + 1, &read_set, NULL, NULL, &timeout);

        // std::cout << "Num Ready : " << num_ready << std::endl;

        if (num_ready > 0) {
            // Data is available to be read
            len = read(outpipe, output, sizeof(output));
            //std::cout << len << std::endl;
            if (len > 0) {
                output[len] = '\0';
                output_str += output;
                if (len < 2048)
                    break;
            }
            else break;
        } else if (num_ready == 0) {
            // No data is available to be read
            //std::cout << "No data" << std::endl;
            break;
        } else {
            // Error occurred
            std::cerr << "Error: select() failed!\n";
            break;
        }
    }
}

void LoadTrainerData(std::string& battleFilename, json& playerdata, std::string& packed_team)
{
    std::ifstream battleFile(battleFilename.c_str());
    if (!battleFile.is_open())
    {
        std::cout << "Could not load battle file at " << battleFilename << std::endl;
        return;
    }
    
    std::string jsonText, pokepaste;
    std::string inLine;
    while (std::getline(battleFile, inLine))
    {
        jsonText += inLine;
    }

    battleFile.close();

    // first, populate a json object so we know what we're dealing with
    playerdata = json::parse(jsonText);

    std::string ppaste_loc = playerdata["teamloc"];

    
    int child_outpipe[2];
    int child_inpipe[2];
    //std::cout << "About to pipe" << std::endl;

    if (pipe(child_inpipe) == -1 || pipe(child_outpipe) == -1)
    {
        std::cerr << "Piping failed in setupAITrainerBattle()" << std::endl;
    }

    //std::cout << "Pipe success" << std::endl;
    
    pid_t child_pid;
    if ((child_pid = fork()) == -1)
    {
        std::cerr << "Fork Failed in setupAITrainerBattle()" << std::endl;
        return;
    }

    if (child_pid == 0)
    {
        // we are the child :)

        // we don't need to read from the output pipe, or
        // write to the input pipe
        close(child_outpipe[0]);
        close(child_inpipe[1]);

        dup2(child_inpipe[0], STDIN_FILENO);
        dup2(child_outpipe[1], STDOUT_FILENO);

        std::string command = "/bin/cat /home/roc/GitHub/pshowdownwrapper/" + ppaste_loc + " | /home/roc/GitHub/pokemon-showdown/pokemon-showdown pack-team";

        int retval = execl("/bin/sh", "sh", "-c", command.c_str(), NULL);
        std::cout << "EXECL RETURNED : " << retval << std::endl;

        close(child_inpipe[0]);
        close(child_outpipe[1]);
    }
    else
    {
        // parent process

        close(child_outpipe[1]);
        close(child_inpipe[0]);

        get_output(child_outpipe[0], packed_team);
        packed_team.erase(std::remove(packed_team.begin(), packed_team.end(), '\n'), packed_team.cend());

        kill(child_pid, SIGINT);
        waitpid(child_pid, NULL, 0);
    }
}

void SetupAITrainerBattle(std::string& ai_filename, int inputstream, int outstream,
    std::string& output, PShowdownParser& parser)
{
    json aitrainer, player;
    std::string aitrainer_team, player_team;
    std::string player_filename = "data/player/playertrainer.pkteam";
    LoadTrainerData(player_filename, player, player_team);
    LoadTrainerData(ai_filename, aitrainer, aitrainer_team);

    std::string ret;

    //std::cout << "Sending start" << std::endl;
    std::string command = ">start {\"formatid\":\"";
    command += aitrainer["format"];
    command += "\"}\n";
    write(inputstream, command.c_str(), command.size());
    //std::cout << "Getting output" << std::endl;
    get_output(outstream, ret);
    //std::cout << ret << std::endl;
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
    write(inputstream, command.c_str(), command.size());
    //std::cout << "Getting output" << std::endl;
    get_output(outstream, ret);
    //std::cout << ret << std::endl;
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
    write(inputstream, command.c_str(), command.size());
    //std::cout << "Getting output" << std::endl;
    get_output(outstream, ret);
    //std::cout << ret << std::endl;
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
    const char* command = "../pokemon-showdown/pokemon-showdown";
    int in_pipe[2];
    int out_pipe[2];
    pid_t pid;

    if (pipe(in_pipe) == -1 || pipe(out_pipe) == -1) {
        std::cerr << "Error: pipe() failed!\n";
        return 1;
    }

    if ((pid = fork()) == -1) {
        std::cerr << "Error: fork() failed!\n";
        return 1;
    } else if (pid == 0) {
        // Child process: execute the command and send output to parent
        close(in_pipe[1]);
        close(out_pipe[0]);

        dup2(in_pipe[0], STDIN_FILENO);
        dup2(out_pipe[1], STDOUT_FILENO);

        // Redirect stderr to /dev/null to suppress error messages
        int fd = open("/dev/null", O_WRONLY);
        dup2(fd, STDERR_FILENO);

        execl(command, command, "simulate-battle", NULL);

        close(in_pipe[0]);
        close(out_pipe[1]);
    } else {
        // Parent process: read user input and send it to child
        close(in_pipe[0]);
        close(out_pipe[1]);

        PShowdownParser parser(in_pipe[1], out_pipe[0]);

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
                write(in_pipe[1], outcmd.c_str(), outcmd.size());
            }
            else if (cmd[0] == "setplayer")
            {
                // format: "setplayer [playernum] [playername]"
                std::string outcmd = ">player p";
                outcmd += cmd[1];
                outcmd += " {\"name\":\"";
                outcmd += cmd[2];
                outcmd += "\"}\n";
                write(in_pipe[1], outcmd.c_str(), outcmd.size());
            }
            else if (cmd[0] == "moves")
            {
                std::cout << parser.QueryMoves() << std::endl;
                continue;
            }
            else if (cmd[0] == "loadbattle")
            {
                std::string out;
                SetupAITrainerBattle(cmd[1], in_pipe[1], out_pipe[0],
                    out, parser);
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
                std::cout << "command : " << outcmd;
                write(in_pipe[1], outcmd.c_str(), outcmd.size());
            }
            else if (cmd[0] == "switch")
            {
                std::cout << "Switching isn't quite ready yet :)" << std::endl;
            }

            std::string output_str;
            get_output(out_pipe[0], output_str);
            std::string parsed_string = parser.parsePShowdownOutput(output_str);
            std::cout << output_str << std::endl;
            std::cout << std::endl << parsed_string << std::endl;
        }

        // Close pipes and terminate child process
        close(in_pipe[1]);
        close(out_pipe[0]);
        waitpid(pid, NULL, 0);
    }

    return 0;
}
