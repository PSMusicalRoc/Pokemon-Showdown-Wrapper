#ifndef _WINDOWS_PID_HANDLER_H_
#define _WINDOWS_PID_HANDLER_H_

#include "PID_Handler.h"

class WindowsPIDHandler : public PID_Handler<int>
{
public:
    bool CreateFork(std::string& forkid, std::string& command) override;
};

#endif