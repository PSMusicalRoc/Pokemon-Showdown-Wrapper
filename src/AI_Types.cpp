#include "AI_Types.h"

#include <unistd.h>
#include <time.h>
#include "PK_Rand.h"

#include <iostream>

void AI_Choose_Move(json& playerdata, std::string& fork, PIDTYPE handler, std::string aitype)
{
    if (aitype == AI_TYPE_RANDOM)
    {
        __AI_Choose_Random(playerdata, fork, handler);
    }
    else
    {
        std::string command = ">p2 move 1\n";
        handler->WriteToFork(fork, command);
    }
}

void __AI_Choose_Random(json& playerdata, std::string& fork, PIDTYPE handler)
{
    // this line inconsistently freezes the program
    // need to figure out why but too lazy atm lol
    //int choice = PK_RAND::mt_rand() % 4;
    int choice = 1 + PK_RAND::mt_rand() % 4;
#ifdef ROC_DEBUG
    std::cout << choice << std::endl;
#endif
    std::string command = ">p2 move ";
    command += std::to_string(choice);
    command += "\n";
    handler->WriteToFork(fork, command);
}