#ifndef _PID_HANDLER_H_
#define _PID_HANDLER_H_

#include <string>
#include <vector>
#include <map>

template<typename V>
class PID_Handler
{
public:
    virtual bool CreateFork(const std::string& forkid, const std::string& command) = 0;
    virtual bool EndFork(const std::string& forkid) = 0;
    virtual int WriteToFork(const std::string& forkid, const std::string& input) = 0;
    virtual int ReadFromFork(const std::string& forkid, std::string& output) = 0;
    virtual int ReadFromForkTimed(const std::string& forkid, std::string& output, float seconds) = 0;

    virtual const V& GetForkData(const std::string& forkid) const = 0;

    virtual void DeleteHandler() = 0;

protected:
    PID_Handler() {}
    std::map<std::string, V> _processes;
};

#endif