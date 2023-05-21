#include "AI_Types.h"

#include <unistd.h>
#include <time.h>
#include "PK_Rand.h"

#include <iostream>

void AI_Choose_Move(json& playerdata, int writepipe, std::string aitype)
{
    if (aitype == AI_TYPE_RANDOM)
    {
        __AI_Choose_Random(playerdata, writepipe);
    }
    else
    {
        std::string command = ">p2 move 1\n";
        write(writepipe, command.c_str(), command.size());
    }
}

void __AI_Choose_Random(json& playerdata, int writepipe)
{
    // this line inconsistently freezes the program
    // need to figure out why but too lazy atm lol
    //int choice = PK_RAND::mt_rand() % 4;
    int choice = PK_RAND::mt_rand() % 4;
    std::cout << choice << std::endl;
    std::string command = ">p2 move ";
    command += std::to_string(choice);
    command += "\n";
    write(writepipe, command.c_str(), command.size());
}