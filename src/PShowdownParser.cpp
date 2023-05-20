#include "PShowdownParser.h"
#include <vector>
#include <regex>

#include "AI_Types.h"

std::string PShowdownParser::parsePShowdownOutput(const std::string& input)
{
    // first things first, we need to separate all the parts of this input.
    // pshowdown uses '|' and newline delimiters.
    // create a vector where each indicies is a 'word'

    std::vector<std::string> readable;
    size_t index = 0;
    while (index < input.size())
    {
        size_t bar_pos = input.find('|', index);
        size_t nl_pos = input.find('\n', index);
        if (bar_pos < nl_pos)
        {
            // we're separated by a pipe
            std::string substr = input.substr(index, bar_pos - index);
            readable.push_back(substr);
            index = bar_pos + 1;
        }
        else if (nl_pos == input.npos)
        {
            // no more output after this
            std::string substr = input.substr(index);
            readable.push_back(substr);
            index = input.size();
        }
        else
        {
            // we're separated by a newline
            std::string substr = input.substr(index, nl_pos - index);
            readable.push_back(substr);
            index = nl_pos + 1;
        }
    }


    /** From here, we have all the output neatly sorted in the readable vector */


    std::string output = "";
    for (int i = 0; i < readable.size(); i++)
    {
        std::string& curr = readable[i];

        if (curr == "player")
        {
            i++; std::string& playerid = readable[i];
            i++; std::string& playername = readable[i];
            i++; std::string& avatar = readable[i];
            i++; std::string& rating = readable[i];
            json* playerdata = nullptr;
            if (playerid == "p1")
            {
                playerdata = &m_p1data;
            }
            else if (playerid == "p2")
            {
                playerdata = &m_p2data;
            }
            
            if (playerdata == nullptr) continue;

            (*playerdata)["name"] = playername;
            (*playerdata)["avatar"] = avatar;
            (*playerdata)["rating"] = rating;

            if (!m_p1data.is_null() && !m_p2data.is_null())
                m_playerdatainitialized = true;
        }
        
        if (curr == "teamsize")
        {
            // teamsize will be stored in the corresponding player's json
            i++; std::string& playerid = readable[i];
            i++; std::string& teamsize_str = readable[i];
            int teamsize = std::stoi(teamsize_str);
            json* playerdata = nullptr;
            if (playerid == "p1")
                playerdata = &m_p1data;
            else if (playerid == "p2")
                playerdata = &m_p2data;
            
            if (playerdata == nullptr) continue;

            (*playerdata)["teamsize"] = teamsize;
        }

        if (curr == "sideupdate")
        {
            i++; std::string& player = readable[i];
            // player number will always be the second character of the string
            if (player == "p1") m_curr_editing = 1;
            else if (player == "p2") m_curr_editing = 2;
            else m_curr_editing = 0;
        }

        if (curr == "request")
        {
            // oh boy this is gonna be a doozy
            // basically the team is written in json, which
            // is somewhat handy because of nlohmann, but
            // i'm still not 100% sure how i want to implement
            // the state of this class. so we'll see how
            // the request is gonna get handled.

            // m_p#data["active"]["moves"] will be the active pokemon's
            // moves for that player, while m_p#data["active"]["pokemon"]
            // will be the actual pokemon's data
            // m_p#data["party"] will be the party

            i++; std::string request = readable[i];
            json parsed = json::parse(request);

            json* player = nullptr;
            switch (m_curr_editing)
            {
            case 1:
                player = &m_p1data;
                break;
            case 2:
                player = &m_p2data;
                break;
            default:
                break;
            }
            if (player == nullptr) continue;

            // it's active[0] because for whatever reason
            // pshowdown has active as a list :|
            // (probably for double/triple battles)
            (*player)["active"] = parsed["active"][0];
            (*player)["party"] = parsed["side"]["pokemon"];

            if (parsed["rqid"].is_null())
            {
                output += "No request ID\n";
            }
            else
            {
                output += "Request ID: ";
                output += std::to_string((int)parsed["rqid"]);
                output += "\n";
            }

            if (parsed["rqid"].is_null())
            {
                // Select a move
                if (m_curr_editing == 2)
                {
                    std::cout << "Selecting move for p2" << std::endl;
                    AI_Choose_Move(m_p2data, m_inpipe, AI_TYPE_RANDOM);
                }
            }
        }


        if (curr == "start")
        {
            output += "Pokemon battle between ";
            output += m_p1data["name"];
            output += " and ";
            output += m_p2data["name"];
            output += " begins!\n";

            json* you = nullptr;
            json* opp = nullptr;

            switch (m_perspective)
            {
            case 1:
                you = &m_p1data;
                opp = &m_p2data;
                break;
            case 2:
                you = &m_p2data;
                opp = &m_p1data;
            }

            output += "Opponent has ";
            output += std::to_string((int)(*opp)["teamsize"]);
            output += " Pokemon.\n";
        }

        if (curr == "switch")
        {
            // we need a variable that keeps track of the
            // active pokemon for each player

            i++; std::string& pk_ident = readable[i];
            i++; std::string& pk_details = readable[i];
            i++; std::string& pk_hp = readable[i];

            std::string player_ident = pk_ident.substr(0, 4);
            std::regex p1_test("p1([a-z]?)\\:");
            std::regex p2_test("p2([a-z]?)\\:");
            
            json* player = nullptr;

            if (std::regex_match(player_ident, p1_test))
                player = &m_p1data;
            else if (std::regex_match(player_ident, p2_test))
                player = &m_p2data;
            
            if (player == nullptr) continue;

            if (!(*player)["active"]["pokemon"].is_null())
            {
                if ((*player)["active"]["pokemon"]["name"] == pk_details) continue;
                output += (*player)["name"];
                output += " withdrew ";
                output += (*player)["active"]["pokemon"]["name"];
                output += "!\n";
            }

            (*player)["active"]["pokemon"] = 
            {
                {"ident",pk_ident},
                {"name",pk_details},
                {"hp",pk_hp}
            };

            output += (*player)["name"];
            output += " sent out ";
            output += (*player)["active"]["pokemon"]["name"];
            output += "!\n";
        }
    }

    return output;
}


std::string PShowdownParser::QueryMoves()
{
    json* player = nullptr;
    switch (m_perspective)
    {
    case 1:
        player = &m_p1data;
        break;
    case 2:
        player = &m_p2data;
        break;
    }
    if (player == nullptr) return "";

    if ((*player)["active"]["moves"].is_null()) return "";

    std::string output = "You have these options:\n";

    for (json& move : (*player)["active"]["moves"])
    {
        if (move.is_null())
        {
            output += "This move is null\n";
            continue;
        }
        output += move["move"];
        output += " PP: ";
        output += std::to_string((int)move["pp"]);
        output += "/";
        output += std::to_string((int)move["maxpp"]);
        output += "\n";
    }

    return output;
}