#ifndef _PID_HANDLER_H_
#define _PID_HANDLER_H_

#include <string>
#include <vector>
#include <map>

template<typename V>
class PID_Handler
{
public:
    virtual bool CreateFork(std::string& forkid, std::string& command) = 0;
    virtual bool EndFork(std::string& forkid) = 0;
    virtual int WriteToFork(std::string& forkid, std::string& input) = 0;
    virtual int ReadFromFork(std::string& forkid, std::string& output) = 0;
    virtual int ReadFromForkTimed(std::string& forkid, std::string& output, float seconds) = 0;

    virtual PID_Handler* Get() = 0;
    virtual void DeleteHandler() = 0;

protected:
    PID_Handler() {}
    std::map<std::string, V> _processes;
};

#endif