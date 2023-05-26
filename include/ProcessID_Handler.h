#ifndef _PROCESS_ID_HANDLER_H_
#define _PROCESS_ID_HANDLER_H_

#ifdef ROC_NIX
#include "UNIX/UNIX_PID_HANDLER.h"
#elif ROC_WINDOWS
#include "Windows/WINDOWS_PID_HANDLER.h"
#endif

#endif