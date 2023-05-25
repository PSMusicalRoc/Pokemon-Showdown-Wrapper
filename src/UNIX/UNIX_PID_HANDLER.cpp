#include "UNIX/UNIX_PID_HANDLER.h"
#include <iostream>

UNIX_PID_HANDLER* UNIX_PID_HANDLER::ptr = nullptr;

bool UNIX_PID_HANDLER::CreateFork(const std::string& forkid, const std::string& command)
{
    pid_t pid;
    pid_t retval = 0;
    int inpipe[2];
    int outpipe[2];

    if (pipe(inpipe) == -1 || pipe(outpipe) == -1) {
        std::cerr << "Error: pipe() failed!\n";
        return false;
    }

    if ((pid = fork()) == -1) {
        std::cerr << "Error: fork() failed!\n";
        retval = -1;
        kill(pid, SIGTERM);
    } else if (pid == 0) {
        // Child process: execute the command and send output to parent
        close(inpipe[1]);
        close(outpipe[0]);

        int val = dup2(inpipe[0], STDIN_FILENO);
        val = dup2(outpipe[1], STDOUT_FILENO);
        //dup2(outpipe[1], STDERR_FILENO);

        close(inpipe[0]);
        close(outpipe[1]);

        // Redirect stderr to /dev/null to suppress error messages
        int fd = open("/dev/null", O_WRONLY);
        dup2(fd, STDERR_FILENO);

        std::vector<std::string> inter; // an intermediate vec for strings
        std::vector<const char*> args;
        size_t index = 0;
        while (index < command.size())
        {
            size_t next_space = command.find(' ', index);
            size_t next_quote = command.find('"', index);
            std::string substr;
            if (next_quote < next_space)
            {
                // group the stuff into one command :)
                size_t ending_quote = command.find('"', next_quote+1);
                if (ending_quote == command.npos)
                {
                    // no ending quote, just lump together
                    // and hope it works
                    substr = command.substr(next_quote + 1);
                    inter.push_back(substr);
                    index = command.size();
                    continue;
                }
                else
                {
                    substr = command.substr(next_quote + 1, ending_quote - next_quote - 1);
                    inter.push_back(substr);
                    index = ending_quote + 1;
                    continue;
                }
            }

            if (next_space == command.npos)
            {
                substr = command.substr(index);
                inter.push_back(substr);
                index = command.size();
            }
            else
            {
                substr = command.substr(index, next_space - index);
                inter.push_back(substr);
                index = next_space + 1;
            }
        }

        for (int i = 0; i < inter.size(); i++)
        {
            args.push_back(inter.at(i).c_str());
        }
        args.push_back(NULL);

        /*for (const char* a : args)
        {
            std::cout << a << std::endl;
        }*/

        retval = getpid();

        execv(args[0], const_cast<char* const*>(args.data()));
        std::cout << "Errno: " << errno << std::endl;

        std::cout << "The program has failed." << std::endl;
    } else {
        // Parent process: read user input and send it to child
        close(inpipe[0]);
        close(outpipe[1]);

        // pid is now for sure the pid of the child
        // process
        UnixPID pidobj(pid, inpipe[1], outpipe[0]);
        _processes.emplace(forkid, pidobj);

        return true;
    }
}

bool UNIX_PID_HANDLER::EndFork(const std::string& forkid)
{
    typename std::map<std::string, UnixPID>::iterator it;
    if ((it = _processes.find(forkid)) == _processes.end())
    {
        return true;
    }

    // it is an iterator pointing at the key-pair
    close(it->second.inpipe);
    close(it->second.outpipe);
    int status;
    if (waitpid(it->second.processID, &status, 1) == -1)
        return false;

    if (_processes.erase(forkid))
        return true;
    return false;
}

int UNIX_PID_HANDLER::WriteToFork(const std::string& forkid, const std::string& input)
{
    UnixPID& pid = _processes.at(forkid);
    ssize_t t = write(pid.inpipe, input.c_str(), input.size());
    return t;
}

int UNIX_PID_HANDLER::ReadFromFork(const std::string& forkid, std::string& output)
{
    output = "";
    UnixPID& pid = _processes.at(forkid);
    nix_get_output(pid.outpipe, output);
    return output.size();
}

int UNIX_PID_HANDLER::ReadFromForkTimed(const std::string& forkid, std::string& output, float seconds)
{
    output = "";
    UnixPID& pid = _processes.at(forkid);
    nix_get_output_timed(pid.outpipe, output, seconds);
    return output.size();
}


const UnixPID& UNIX_PID_HANDLER::GetForkData(const std::string& forkid) const
{
    return _processes.at(forkid);
}


PID_Handler<UnixPID>* UNIX_PID_HANDLER::Get()
{
    if (!ptr)
    {
        ptr = new UNIX_PID_HANDLER();
    }
    return ptr;
}

void UNIX_PID_HANDLER::DeleteHandler()
{
    delete ptr;
    ptr = 0;
}

void UNIX_PID_HANDLER::nix_get_output(int outpipe, std::string& output_str)
{
    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(outpipe, &read_set);

    output_str = "";

    while (true) {
        ssize_t len;
        char output[2048];
        len = read(outpipe, output, sizeof(output));
        //std::cout << len << std::endl;
        if (len > 0) {
            output[len] = '\0';
            output_str += output;
            if (len < 2048)
                break;
        }
        else break;
    }
}

void UNIX_PID_HANDLER::nix_get_output_timed(int outpipe, std::string& output_str, float num_secs)
{
    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(outpipe, &read_set);

    output_str = "";

    while (true) {
        ssize_t len;
        char output[2048];

        // Use select() to check if data is available to be read from out_pipe[0]
        timeval timeout = {0, 1000000 * num_secs};
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
