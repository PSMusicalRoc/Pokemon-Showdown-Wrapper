#ifndef _UNIX_PID_HANDLER_H_
#define _UNIX_PID_HANDLER_H_

#include "PID_Handler.h"

#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#include <string>
#include <vector>

#define PIDTYPE PID_Handler<UnixPID>*
#define PIDHANDLER UNIX_PID_HANDLER

struct UnixPID
{
    UnixPID(pid_t pid, int in, int out)
        :processID(pid), inpipe(in), outpipe(out) {}
    pid_t processID;
    int inpipe;
    int outpipe;
};

class UNIX_PID_HANDLER : public PID_Handler<UnixPID>
{
protected:
    UNIX_PID_HANDLER()
        :PID_Handler<UnixPID>() {}
    static UNIX_PID_HANDLER* ptr;

public:
    bool CreateFork(const std::string& forkid, const std::string& command) override;
    bool EndFork(const std::string& forkid) override;
    int WriteToFork(const std::string& forkid, const std::string& input) override;
    int ReadFromFork(const std::string& forkid, std::string& output) override;
    int ReadFromForkTimed(const std::string& forkid, std::string& output, float seconds) override;

    const UnixPID& GetForkData(const std::string& forkid) const override;

    static PID_Handler* Get();
    void DeleteHandler() override;

    void nix_get_output(int outpipe, std::string& output_str);
    void nix_get_output_timed(int outpipe, std::string& output_str, float num_secs);
};

#endif