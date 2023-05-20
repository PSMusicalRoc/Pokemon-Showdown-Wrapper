#ifndef _AI_TYPES_H_
#define _AI_TYPES_H_

#define AI_TYPE_RANDOM "random"

#include <string>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

void AI_Choose_Move(json& playerdata, int writepipe, std::string aitype);

void __AI_Choose_Random(json& playerdata, int writepipe);

#endif