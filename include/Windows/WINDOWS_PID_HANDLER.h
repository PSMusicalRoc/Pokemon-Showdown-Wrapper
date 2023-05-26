#ifndef _WINDOWS_PID_HANDLER_H_
#define _WINDOWS_PID_HANDLER_H_

#include <windows.h>
#include "PID_Handler.h"

#define PIDTYPE PID_Handler<WindowsPID>*
#define PIDHANDLER WINDOWS_PID_HANDLER

struct WindowsPID
{
    WindowsPID() {}
    PROCESS_INFORMATION pi;
    HANDLE std_in_read;
    HANDLE std_in_write;
    HANDLE std_out_read;
    HANDLE std_out_write;
};

class WINDOWS_PID_HANDLER : public PID_Handler<WindowsPID>
{
protected:
    WINDOWS_PID_HANDLER()
        :PID_Handler<WindowsPID>() {}
    static WINDOWS_PID_HANDLER* ptr;

public:
    bool CreateFork(const std::string& forkid, const std::string& command) override;
    bool EndFork(const std::string& forkid) override;
    int WriteToFork(const std::string& forkid, const std::string& input) override;
    int ReadFromFork(const std::string& forkid, std::string& output) override;
    int ReadFromForkTimed(const std::string& forkid, std::string& output, float seconds) override;

    const WindowsPID& GetForkData(const std::string& forkid) const override;

    static PID_Handler* Get();
    void DeleteHandler() override;
};

#endif