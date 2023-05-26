#include "Windows/WINDOWS_PID_HANDLER.h"
#include <iostream>

bool WINDOWS_PID_HANDLER::CreateFork(const std::string& forkid, const std::string& command)
{
    WindowsPID pid;
    STARTUPINFO si;
    SECURITY_ATTRIBUTES saAttr;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    si.hStdError = pid.std_out_write;
    si.hStdOutput = NULL;
    si.hStdInput = pid.std_in_read;
    si.dwFlags |= STARTF_USESTDHANDLES;

    ZeroMemory( &pid.pi, sizeof(pid.pi) );

    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = FALSE;
    saAttr.lpSecurityDescriptor = NULL;

    // Create a pipe for the child process's STDOUT. 
 
    if ( ! CreatePipe(&pid.std_out_read, &pid.std_in_write, &saAttr, 0) ) 
    {
        std::cerr << "Could not initialize the stdout pipe" << std::endl;
        return false;
    }

    // Create a pipe for the child process's STDIN. 
    
    if (! CreatePipe(&pid.std_in_read, &pid.std_in_write, &saAttr, 0)) 
    {
        std::cerr << "Could not initialize the stdin pipe" << std::endl;
        return false;
    }

    LPSTR cmd_win = strdup(command.c_str());

    if (
        !CreateProcess(
            NULL,
            cmd_win,
            NULL,
            NULL,
            TRUE,
            0,
            NULL,
            NULL,
            &si,
            &pid.pi
        )
    )
    {
        std::cerr << "Could not start the process." << std::endl;
        return false;
    }

    // if it succeeds, pi has the data we need.
    _processes.emplace(forkid, pid);
    return true;
}

bool WINDOWS_PID_HANDLER::EndFork(const std::string& forkid)
{
    WindowsPID& pid = _processes.at(forkid);
    CloseHandle(pid.pi.hProcess);
    CloseHandle(pid.pi.hThread);

    return true;
}

int WINDOWS_PID_HANDLER::WriteToFork(const std::string& forkid, const std::string& input)
{

}

int WINDOWS_PID_HANDLER::ReadFromFork(const std::string& forkid, std::string& output)
{

}

int WINDOWS_PID_HANDLER::ReadFromForkTimed(const std::string& forkid, std::string& output, float seconds)
{

}


const WindowsPID& WINDOWS_PID_HANDLER::GetForkData(const std::string& forkid) const
{

}

PID_Handler<WindowsPID>* WINDOWS_PID_HANDLER::Get()
{

}

void WINDOWS_PID_HANDLER::DeleteHandler()
{

}
