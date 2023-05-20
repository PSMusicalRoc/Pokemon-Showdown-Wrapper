#include "AI_Types.h"

#include <unistd.h>

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
    int choice = random() % 4;
    std::string command = ">p2 move ";
    command += std::to_string(choice);
    command += "\n";
    write(writepipe, command.c_str(), command.size());
}