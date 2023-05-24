#include "UNIX/UNIX_PID_HANDLER.h"

bool UNIX_PID_HANDLER::CreateFork(std::string& forkid, std::string& command) override
{
    pid_t pid;
    pid_t retval = 0;
    int[2] inpipe;
    int[2] outpipe;

    if (pipe(inpipe) == -1 || pipe(outpipe) == -1) {
        std::cerr << "Error: pipe() failed!\n";
        retval = -1;
        kill(pid, SIGTERM);
    }

    if ((pid = fork()) == -1) {
        std::cerr << "Error: fork() failed!\n";
        retval = -1;
        kill(pid, SIGTERM);
    } else if (pid == 0) {
        // Child process: execute the command and send output to parent
        close(inpipe[1]);
        close(outpipe[0]);

        dup2(inpipe[0], STDIN_FILENO);
        dup2(outpipe[1], STDOUT_FILENO);

        close(inpipe[0]);
        close(outpipe[1]);

        // Redirect stderr to /dev/null to suppress error messages
        int fd = open("/dev/null", O_WRONLY);
        dup2(fd, STDERR_FILENO);

        std::vector<char* const> args;
        size_t index = 0;
        while (index < command.size())
        {
            size_t next_space = command.find(' ', index);
            std::string substr;
            if (next_space == command.npos)
            {
                substr = command.substr(index);
                args.push_back(substr.c_str());
                index = command.size();
            }
            else
            {
                substr = command.substr(index, next_space - index);
                args.push_back(substr.c_str());
                index = next_space + 1;
            }
        }
        args.push_back(NULL);

        retval = getpid();

        execv(args[0], args.data());

        std::cout << "The program has failed." << std::endl;
    } else {
        // Parent process: read user input and send it to child
        close(inpipe[0]);
        close(outpipe[1]);

        while (retval == 0) continue;

        if (retval == -1)
        {
            return false;
        }

        // retval is now for sure the pid of the child
        // process
        _processes.

        return retval;
    }
}

bool UNIX_PID_HANDLER::EndFork(std::string& forkid) override
{

}

int UNIX_PID_HANDLER::WriteToFork(std::string& forkid, std::string& input) override
{

}

int UNIX_PID_HANDLER::ReadFromFork(std::string& forkid, std::string& output) override
{

}

int UNIX_PID_HANDLER::ReadFromForkTimed(std::string& forkid, std::string& output, float seconds) override
{

}

PID_Handler* UNIX_PID_HANDLER::Get() override
{
    if (!ptr)
    {
        ptr = new UNIX_PID_HANDLER();
    }
    return ptr;
}

void UNIX_PID_HANDLER::DeleteHandler() override
{
    delete ptr;
    ptr = 0;
}


pid_t UNIX_PID_HANDLER::CreateFork(int* inpipe, int* outpipe, std::string& command)
{
   
}

void UNIX_PID_HANDLER::CloseFork(pid_t child_pid)
{
    close(in_pipe[1]);
    close(out_pipe[0]);
    waitpid(child_pid, NULL, 0);
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
