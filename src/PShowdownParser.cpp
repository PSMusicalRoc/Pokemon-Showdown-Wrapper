#include "PShowdownParser.h"
#include <vector>
#include <regex>

#include "AI_Types.h"

#define OUTPUT_MAC          (*output)

#define EFFECT_PROTECT      "move: Protect"

#define SINGLETURN_PROTECT  "Protect"

#define STAT_RAISED_0       "won't go higher!"
#define STAT_RAISED_1       "rose!"
#define STAT_RAISED_2       "sharply rose!"
#define STAT_RAISED_3PLUS   "drastically rose!"
#define STAT_LOWER_0        "won't go lower!"
#define STAT_LOWER_1        "fell!"
#define STAT_LOWER_2        "harshly fell!"
#define STAT_LOWER_3PLUS    "severely fell!"

#define STAT_ATTACK         "atk"
#define STAT_DEFENSE        "def"
#define STAT_SPEC_ATTACK    "spa"
#define STAT_SPEC_DEFENSE   "spd"
#define STAT_SPEED          "spe"


#define STATUS_BURN         "brn"
#define STATUS_POISON       "psn"
#define STATUS_TOXIC        "tox"

#define WEATHER_SANDSTORM   "Sandstorm"
#define WEATHER_RAIN        "Rain"
#define WEATHER_HAIL        "Hail"
#define WEATHER_SUN         "Sun"
#define WEATHER_NULL        "none"


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
            //if (substr != "")
                readable.push_back(substr);
            readable.push_back("\n"); // useful for things like split and ability
            index = nl_pos + 1;
        }
    }


    /** From here, we have all the output neatly sorted in the readable vector */


    // This is a hacky fix for split...
    // split means that the same command is essentially
    // just sent twice. We don't want that, so we're gonna do
    // this.
    bool justcaughtsplit = false;
    bool donefirstsplit = false;
    bool donesecondsplit = false;
    std::string out, buff_to_remove;
    std::string* output = &out;

    for (int i = 0; i < readable.size(); i++)
    {
        std::string& curr = readable[i];

        // if it's a newline outside of the newline
        // supported commands, just continue, make
        // the program go as fast as possible.
        if (curr == "\n" || curr == "") continue;

        // Split handling
        if (justcaughtsplit)
        {
            justcaughtsplit = false;
            donefirstsplit = true;
        }
        else if (donefirstsplit)
        {
            donefirstsplit = false;
            donesecondsplit = true;
            output = &buff_to_remove;
        }
        else if (donesecondsplit)
        {
            donesecondsplit = false;
            output = &out;
        }


        /*
        * ################################
        * #        BATTLE PROGRESS       #
        * ################################
        * (sorted alphabetically)
        */

        if (curr == "clearpoke")
        {
            //|clearpoke
            continue;
        }

        if (curr == "error")
        {
            //|error|[Invalid choice] MESSAGE
            i++;
            continue;
        }

        if (curr == "gametype")
        {
            //|gametype|GAMETYPE
            i++; continue;
        }

        if (curr == "gen")
        {
            //|gen|GENNUM
            i++; continue;
        }

        if (curr == "inactive")
        {
            //|inactive|MESSAGE
            i++; continue;
        }

        if (curr == "inactiveoff")
        {
            //|inactiveoff|MESSAGE
            i++; continue;
        }

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

        if (curr == "poke")
        {
            //|poke|PLAYER|DETAILS|ITEM
            i++;i++;i++;
            continue;
        }

        if (curr == "rated")
        {
            i++; std::string msg = readable[i];
            if (msg == "\n")
            {
                //|rated
                continue;
            }
            else
            {
                //|rated|MESSAGE
                continue;
            }
        }

        if (curr == "request")
        {
            // m_p#data["active"]["moves"] will be the active pokemon's
            // moves for that player, while m_p#data["active"]["pokemon"]
            // will be the actual pokemon's data
            // m_p#data["party"] will be the party

            i++; std::string request = readable[i];
            json parsed = json::parse(request);

            if (!parsed["wait"].is_null() && parsed["wait"] == true)
                continue;

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

            // Select a move
            if (m_curr_editing == 2)
            {
                if (parsed["forceswitch"][0] == true)
                {
                    // do nothing for the moment lol
                }
                else
                {
                    AI_Choose_Move(m_p2data, fork, handler, AI_TYPE_RANDOM);
                }
            }
        }

        if (curr == "rule")
        {
            //|rule|RULE: DESCRIPTION
            i++; continue;
        }

        if (curr == "sideupdate")
        {
            i++; std::string& player = readable[i];
            if (player == "\n")
            {
                i++; player = readable[i];
            }
            // player number will always be the second character of the string
            if (player == "p1") m_curr_editing = 1;
            else if (player == "p2") m_curr_editing = 2;
            else m_curr_editing = 0;
            continue;
        }

        if (curr == "split")
        {
            //|split|PLAYER
            i++; std::string player_ident = readable[i];
            justcaughtsplit = true;
            continue;
        }

        if (curr == "start")
        {
            OUTPUT_MAC += "Pokemon battle between ";
            OUTPUT_MAC += m_p1data["name"];
            OUTPUT_MAC += " and ";
            OUTPUT_MAC += m_p2data["name"];
            OUTPUT_MAC += " begins!\n";

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

            OUTPUT_MAC += "Opponent has ";
            OUTPUT_MAC += std::to_string((int)(*opp)["teamsize"]);
            OUTPUT_MAC += " Pokemon.\n";
        }

        if (curr == "t:")
        {
            //|t:|TIMESTAMP
            i++; continue;
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

        if (curr == "tie")
        {
            //|tie
            continue;
        }

        if (curr == "tier")
        {
            //|tier|FORMATNAME
            i++; continue;
        }

        if (curr == "turn")
        {
            //|turn|NUMBER
            i++; std::string turn_num = readable[i];
            OUTPUT_MAC += "Turn ";
            OUTPUT_MAC += turn_num;
            OUTPUT_MAC += "\n\n";
        }

        if (curr == "upkeep")
        {
            //|upkeep
            continue;
        }

        if (curr == "win")
        {
            i++; std::string winner = readable[i];
            OUTPUT_MAC += winner + " won the battle!\n";
        }


        /*
        * ################################
        * #         MAJOR ACTIONS        #
        * ################################
        * (sorted alphabetically)
        */

        if (curr == "cant")
        {
            i++; i++;
            i++; std::string move = readable[i];
            if (move == "\n")
            {
                //|cant|POKEMON|REASON
                continue;
            }
            else
            {
                //|cant|POKEMON|REASON|MOVE
                continue;
            }
        }

        if (curr == "detailschange")
        {
            //|detailschange|POKEMON|DETAILS|HP STATUS
            // This is a permanent forme change (ex. Mega Evolution)
            i++; i++; i++;
            continue;
        }

        if (curr == "drag")
        {
            //|drag|POKEMON|DETAILS|HP STATUS
            // Much like 'switch', but for things that drag
            // the pokemon out (ex. Fazing)
            i++; i++; i++;
            continue;
        }

        if (curr == "-formechange")
        {
            //|-formechange|POKEMON|SPECIES|HP STATUS
            // Temporary forme change (ex. Castform, Shaymin-Sky)
            i++; i++; i++;
            continue;
        }

        if (curr == "faint")
        {
            // first get the player that the fainted pokemon belonged to
            i++; std::string& pk_ident = readable[i];
            std::string player_ident = pk_ident.substr(0, 4);
            std::regex p1_test("p1([a-z]?)\\:");
            std::regex p2_test("p2([a-z]?)\\:");

            json* m_pdata = nullptr;
            if (std::regex_match(player_ident, p1_test))
            {
                m_pdata = &m_p1data;
            }
            else if (std::regex_match(player_ident, p2_test))
            {
                m_pdata = &m_p2data;
            }

            if (m_pdata == nullptr)
            {
                continue;
            }

            pk_ident = pk_ident.substr(0, 2) + pk_ident.substr(3);

            for (json& pk : (*m_pdata)["party"])
            {
                std::string testident(pk["ident"]);
                if (testident == pk_ident)
                {
                    std::string pkdetails(pk["details"]);
                    OUTPUT_MAC += pkdetails + " fainted!\n";
                }
            }
        }

        if (curr == "move")
        {
            // list out the move
            //|move|POKEMON|MOVE|TARGET
            i++; std::string user = readable[i];
            i++; std::string move = readable[i];
            i++; std::string target = readable[i];

            OUTPUT_MAC += user;
            OUTPUT_MAC += " used ";
            OUTPUT_MAC += move;
            OUTPUT_MAC += "!\n";
        }

        if (curr == "replace")
        {
            //|replace|POKEMON|DETAILS|HP STATUS
            // This is called when Illusion ends
            i++; i++; i++;
            continue;
        }

        if (curr == "swap")
        {
            //|swap|POKEMON|POSITION
            i++; i++;
            continue;
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
                OUTPUT_MAC += (*player)["name"];
                OUTPUT_MAC += " withdrew ";
                OUTPUT_MAC += (*player)["active"]["pokemon"]["name"];
                OUTPUT_MAC += "!\n";
            }

            (*player)["active"]["pokemon"] = 
            {
                {"ident",pk_ident},
                {"name",pk_details},
                {"hp",pk_hp}
            };

            OUTPUT_MAC += (*player)["name"];
            OUTPUT_MAC += " sent out ";
            OUTPUT_MAC += (*player)["active"]["pokemon"]["name"];
            OUTPUT_MAC += "!\n";
        }



        /*
        * ################################
        * #         MINOR ACTIONS        #
        * ################################
        * (sorted alphabetically)
        */

        if (curr == "-ability")
        {
            i++; std::string pk_ident = readable[i];
            i++; std::string ability = readable[i];
            i++; std::string& effect = readable[i];
            if (effect == "\n")
            {
                //|-ability|POKEMON|ABILITY
                OUTPUT_MAC += pk_ident;
                OUTPUT_MAC += "'s ";
                OUTPUT_MAC += ability + "!\n";
                continue;
            }
            else
            {
                //|-ability|POKEMON|ABILITY|[from]EFFECT
                continue;
            }
        }

        if (curr == "-activate")
        {
            //|-activate|POKEMON|EFFECT
            i++; std::string pk_ident = readable[i];
            i++; std::string effect = readable[i];

            if (effect == EFFECT_PROTECT)
            {
                OUTPUT_MAC += pk_ident;
                OUTPUT_MAC += " protected itself!\n";
            }
            continue;
        }

        if (curr == "-block")
        {
            //|-block|POKEMON|EFFECT|MOVE|ATTACKER
            i++; i++; i++; i++;
            continue;
        }

        if (curr == "-boost")
        {
            //|-boost|POKEMON|STAT|AMOUNT
            i++; std::string pk_ident = readable[i];
            i++; std::string stat = readable[i];
            i++; int amount = std::stoi(readable[i]);

            std::string player_ident = pk_ident.substr(0, 4);
            std::regex p1_test("p1([a-z]?)\\:");
            std::regex p2_test("p2([a-z]?)\\:");

            json* player = nullptr;

            if (std::regex_match(player_ident, p1_test))
                player = &m_p1data;
            else if (std::regex_match(player_ident, p2_test))
                player = &m_p2data;
            
            if (player == nullptr) continue;
            
            json& pokemon = (*player)["active"]["pokemon"];

            std::string stat_output;
            if (stat == STAT_ATTACK)
                stat_output = "attack";
            else if (stat == STAT_DEFENSE)
                stat_output = "defense";
            else if (stat == STAT_SPEC_ATTACK)
                stat_output = "special attack";
            else if (stat == STAT_SPEC_DEFENSE)
                stat_output = "special defense";
            else if (stat == STAT_SPEED)
                stat_output = "speed";
            
            OUTPUT_MAC += pokemon["name"];
            OUTPUT_MAC += "'s ";
            OUTPUT_MAC += stat_output;
            OUTPUT_MAC += " ";

            switch (amount)
            {
            case 0:
                OUTPUT_MAC += STAT_RAISED_0;
                break;
            case 1:
                OUTPUT_MAC += STAT_RAISED_1;
                break;
            case 2:
                OUTPUT_MAC += STAT_RAISED_2;
                break;
            default:
                OUTPUT_MAC += STAT_RAISED_3PLUS;
                break;
            }
            OUTPUT_MAC += "\n";
        }

        if (curr == "-burst")
        {
            //|-burst|POKEMON|SPECIES|ITEM
            i++;
            i++;
            i++;
            continue;
        }

        if (curr == "-center")
        {
            //|-center
            continue;
        }

        if (curr == "-clearallboost")
        {
            //|-clearallboost
            continue;
        }

        if (curr == "-clearboost")
        {
            //|-clearboost|POKEMON
            i++;
            continue;
        }

        if (curr == "-clearnegativeboost")
        {
            //|-clearnegativeboost|POKEMON
            i++;
            continue;
        }

        if (curr == "-clearpositiveboost")
        {
            //|-clearpositiveboost|TARGET|POKEMON|EFFECT
            i++;
            i++;
            i++;
            continue;
        }

        if (curr == "-combine")
        {
            //|-combine
            continue;
        }

        if (curr == "-copyboost")
        {
            //|-copyboost|SOURCE|TARGET
            i++;
            i++;
            continue;
        }

        if (curr == "-crit")
        {
            //|-crit|POKEMON
            i++; std::string pk_ident = readable[i];
            OUTPUT_MAC += "Critical hit on ";
            OUTPUT_MAC += pk_ident + "!\n";
        }

        if (curr == "-curestatus")
        {
            //|-curestatus|POKEMON|STATUS
            i++;
            i++;
            continue;
        }

        if (curr == "-cureteam")
        {
            //|-cureteam|POKEMON
            i++;
            continue;
        }

        if (curr == "-damage")
        {
            //|-damage|POKEMON|HP STATUS|[from] SOURCE
            i++; std::string pk_ident = readable[i];
            i++;
            if (readable[i+1] != "\n")
            {
                i++; std::string fromsource = readable[i];
                std::string source = fromsource.substr(7);
                if (source == STATUS_POISON)
                {
                    OUTPUT_MAC += pk_ident;
                    OUTPUT_MAC += " was hurt by poison!\n";
                    continue;
                }
                if (source == STATUS_BURN)
                {
                    OUTPUT_MAC += pk_ident;
                    OUTPUT_MAC += " was hurt by its burn!\n";
                    continue;
                }
                
                OUTPUT_MAC += pk_ident;
                OUTPUT_MAC += " was damaged by ";
                OUTPUT_MAC += source + "!\n";
            }
            continue;
        }

        if (curr == "-end")
        {
            //|-end|POKEMON|EFFECT
            i++;
            i++;
            continue;
        }

        if (curr == "-endability")
        {
            //|-endability|POKEMON
            i++;
            continue;
        }

        if (curr == "-enditem")
        {
            i++;
            i++;
            i++; std::string effect = readable[i];
            if (effect == "\n")
            {
                //|-enditem|POKEMON|ITEM
                continue;
            }
            else
            {
                //|-enditem|POKEMON|ITEM|[from]EFFECT
                continue;
            }
        }

        if (curr == "-fail")
        {
            //|-fail|POKEMON|ACTION
            i++; i++;
            OUTPUT_MAC += "But it failed!\n";
            continue;
        }

        if (curr == "-fieldend")
        {
            //|-fieldend|CONDITION
            i++;
            continue;
        }

        if (curr == "-fieldstart")
        {
            //|-fieldstart|CONDITION
            i++;
            continue;
        }

        if (curr == "-heal")
        {
            //|-heal|POKEMON|HP STATUS
            i++;
            i++;
            continue;
        }

        if (curr == "-hint")
        {
            //|-hint|MESSAGE
            i++;
            continue;
        }

        if (curr == "-hitcount")
        {
            //|-hitcount|POKEMON|NUM
            i++;
            i++;
            continue;
        }

        if (curr == "-immune")
        {
            //|-immune|POKEMON
            i++; std::string pk_ident = readable[i];
            OUTPUT_MAC += "It didn't affect ";
            OUTPUT_MAC += pk_ident + "...\n";
        }

        if (curr == "-invertboost")
        {
            //|-invertboost|POKEMON
            i++;
            continue;
        }

        if (curr == "-item")
        {
            i++;
            i++;
            i++; std::string effect = readable[i];
            if (effect == "\n")
            {
                //|-item|POKEMON|ITEM
                continue;
            }
            else
            {
                //|-item|POKEMON|ITEM|[from]EFFECT
                continue;
            }
        }

        if (curr == "-mega")
        {
            //|-mega|POKEMON|MEGASTONE
            i++;
            i++;
            continue;
        }

        if (curr == "-message")
        {
            //|-message|MESSAGE
            i++;
            continue;
        }

        if (curr == "-miss")
        {
            //|-miss|SOURCE|TARGET
            i++; std::string pk_ident = readable[i];
            i++; std::string target = readable[i];
            OUTPUT_MAC += target;
            OUTPUT_MAC += " avoided the attack!\n";
        }

        if (curr == "-mustrecharge")
        {
            //|-mustrecharge|POKEMON
            i++;
            continue;
        }

        if (curr == "-notarget")
        {
            //|-notarget|POKEMON
            i++;
            continue;
        }

        if (curr == "-nothing")
        {
            //|-nothing
            // [note]: this is apparently deprecated, in
            // the future this may change.
            OUTPUT_MAC += "But nothing happened!\n";
            continue;
        }

        if (curr == "-prepare")
        {
            i++;
            i++;
            i++; std::string& defender = readable[i];

            if (defender == "\n")
            {
                //|-prepare|ATTACKER|MOVE
                continue;
            }
            else
            {
                //|-prepare|ATTACKER|MOVE|DEFENDER
                continue;
            }
        }

        if (curr == "-primal")
        {
            //|-primal|POKEMON
            i++;
            continue;
        }

        if (curr == "-resisted")
        {
            i++;
            OUTPUT_MAC += "It wasn't very effective...\n";
        }

        if (curr == "-setboost")
        {
            //|-setboost|POKEMON|STAT|AMOUNT
            i++;
            i++;
            i++;
            continue;
        }

        if (curr == "-sethp")
        {
            //|-sethp|POKEMON|HP
            i++;
            i++;
            continue;
        }

        if (curr == "-sideend")
        {
            //|-sideend|SIDE|CONDITION
            i++;
            i++;
            continue;
        }

        if (curr == "-sidestart")
        {
            //|-sidestart|SIDE|CONDITION
            i++;
            i++;
            continue;
        }

        if (curr == "-singlemove")
        {
            //|-singlemove|POKEMON|MOVE
            i++;
            i++;
            continue;
        }

        if (curr == "-singleturn")
        {
            //|-singleturn|POKEMON|MOVE
            i++; std::string pk_ident = readable[i];
            i++; std::string effect = readable[i];

            if (effect == SINGLETURN_PROTECT)
            {
                OUTPUT_MAC += pk_ident;
                OUTPUT_MAC += " protected itself!\n";
            }
            continue;
        }

        if (curr == "-start")
        {
            //|-start|POKEMON|EFFECT
            i++;
            i++;
            continue;
        }

        if (curr == "-status")
        {
            //|-status|POKEMON|STATUS
            i++;
            i++;
            continue;
        }

        if (curr == "-supereffective")
        {
            i++;
            OUTPUT_MAC += "It was super-effective!\n";
        }

        if (curr == "-swapboost")
        {
            //|-swapboost|SOURCE|TARGET|STATS
            i++;
            i++;
            i++;
            continue;
        }

        if (curr == "-swapsideconditions")
        {
            //|-swapsideconditions
            continue;
        }

        if (curr == "-transform")
        {
            //|-transform|POKEMON|SPECIES
            i++;
            i++;
            continue;
        }

        if (curr == "-unboost")
        {
            //|-unboost|POKEMON|STAT|AMOUNT
            i++; std::string pk_ident = readable[i];
            i++; std::string stat = readable[i];
            i++; int amount = std::stoi(readable[i]);

            std::string player_ident = pk_ident.substr(0, 4);
            std::regex p1_test("p1([a-z]?)\\:");
            std::regex p2_test("p2([a-z]?)\\:");

            json* player = nullptr;

            if (std::regex_match(player_ident, p1_test))
                player = &m_p1data;
            else if (std::regex_match(player_ident, p2_test))
                player = &m_p2data;
            
            if (player == nullptr) continue;
            
            json& pokemon = (*player)["active"]["pokemon"];

            std::string stat_output;
            if (stat == STAT_ATTACK)
                stat_output = "attack";
            else if (stat == STAT_DEFENSE)
                stat_output = "defense";
            else if (stat == STAT_SPEC_ATTACK)
                stat_output = "special attack";
            else if (stat == STAT_SPEC_DEFENSE)
                stat_output = "special defense";
            else if (stat == STAT_SPEED)
                stat_output = "speed";
            
            OUTPUT_MAC += pokemon["name"];
            OUTPUT_MAC += "'s ";
            OUTPUT_MAC += stat_output;
            OUTPUT_MAC += " ";

            switch (amount)
            {
            case 0:
                OUTPUT_MAC += STAT_LOWER_0;
                break;
            case 1:
                OUTPUT_MAC += STAT_LOWER_1;
                break;
            case 2:
                OUTPUT_MAC += STAT_LOWER_2;
                break;
            default:
                OUTPUT_MAC += STAT_LOWER_3PLUS;
                break;
            }
            OUTPUT_MAC += "\n";
        }
    
        if (curr == "-waiting")
        {
            //|-waiting|SOURCE|TARGET
            i++;
            i++;
            continue;
        }

        if (curr == "-weather")
        {
            //|-weather|WEATHER
            i++; std::string weather = readable[i];
            // weather is stored in m_battledata["weather"]
            if (weather == WEATHER_NULL)
            {
                weather = m_battledata["weather"];
                if (weather == WEATHER_SANDSTORM)
                {
                    OUTPUT_MAC += "Sandstorm over.\n";
                }
                if (weather == WEATHER_SUN)
                {
                    OUTPUT_MAC += "Bright Sunlight over.\n";
                }
                if (weather == WEATHER_RAIN)
                {
                    OUTPUT_MAC += "The rain stopped.\n";
                }
                if (weather == WEATHER_HAIL)
                {
                    OUTPUT_MAC += "Hail over.\n";
                }
                m_battledata["weather"] = WEATHER_NULL;
                continue;
            }
            if (weather == WEATHER_SANDSTORM)
            {
                if (m_battledata["weather"].is_null() || m_battledata["weather"] != WEATHER_SANDSTORM)
                {
                    OUTPUT_MAC += "A sandstorm kicked up!\n";
                }
                else
                {
                    OUTPUT_MAC += "The sandstorm rages.\n";
                }
                m_battledata["weather"] = WEATHER_SANDSTORM;
            }
            if (weather == WEATHER_SUN)
            {
                if (m_battledata["weather"].is_null() || m_battledata["weather"] != WEATHER_SUN)
                {
                    OUTPUT_MAC += "The sunlight turned harsh!\n";
                }
                else
                {
                    OUTPUT_MAC += "The sunlight is strong.\n";
                }
                m_battledata["weather"] = WEATHER_SUN;
            }
            if (weather == WEATHER_RAIN)
            {
                if (m_battledata["weather"].is_null() || m_battledata["weather"] != WEATHER_RAIN)
                {
                    OUTPUT_MAC += "It began to rain!\n";
                }
                else
                {
                    OUTPUT_MAC += "It is raining.\n";
                }
                m_battledata["weather"] = WEATHER_RAIN;
            }
            if (weather == WEATHER_HAIL)
            {
                if (m_battledata["weather"].is_null() || m_battledata["weather"] != WEATHER_HAIL)
                {
                    OUTPUT_MAC += "It began to hail!\n";
                }
                else
                {
                    OUTPUT_MAC += "It is hailing.\n";
                }
                m_battledata["weather"] = WEATHER_HAIL;
            }
            continue;
        }

        if (curr == "-zbroken")
        {
            //|-zbroken|POKEMON
            i++;
            continue;
        }

        if (curr == "-zpower")
        {
            //|-zpower|POKEMON
            i++;
            continue;
        }

    }

    return out;
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