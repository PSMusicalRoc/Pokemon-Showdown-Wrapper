#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <string>
#include <fcntl.h>
#include <sys/wait.h>
#include <vector>

#include "PShowdownParser.h"

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
        timeval timeout = {0, 1000000};  // Wait up to 1 second
        int num_ready = select(outpipe + 1, &read_set, NULL, NULL, &timeout);

        if (num_ready > 0) {
            // Data is available to be read
            len = read(outpipe, output, sizeof(output));
            if (len > 0) {
                output[len] = '\0';
                output_str += output;
            }
        } else if (num_ready == 0) {
            // No data is available to be read
            break;
        } else {
            // Error occurred
            std::cerr << "Error: select() failed!\n";
            break;
        }
    }
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

        PShowdownParser parser;

        std::string incmd;
        while (true) {
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
