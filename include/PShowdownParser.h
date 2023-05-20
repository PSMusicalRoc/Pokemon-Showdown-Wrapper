#ifndef _ROC_POKEMON_H_
#define _ROC_POKEMON_H_

#include <string>
#include <iostream>
#include <nlohmann/json.hpp>

using json=nlohmann::json;

class PShowdownParser
{
public:
    PShowdownParser(int inpipe, int outpipe, int player_perspective = 1)
        :m_inpipe(inpipe), m_outpipe(outpipe), m_perspective(player_perspective) {}

    std::string parsePShowdownOutput(const std::string& input);

    const std::string GetPlayer1Name() const { return m_p1data["name"]; }
    const std::string GetPlayer2Name() const { return m_p2data["name"]; }

    std::string QueryMoves();

    bool OppSelectMove() const { return m_opp_needs_select_move; }
    void OppSelectedMove() { m_opp_needs_select_move = false; }


private:
    bool m_playerdatainitialized = false;
    bool m_battledatainitialized = false;

    int m_perspective;
    int m_curr_editing = 0;

    json m_battledata;

    json m_p1data;
    json m_p2data;
    json m_p3data;
    json m_p4data;

    bool m_opp_needs_select_move = false;

    int m_inpipe;
    int m_outpipe;
};

#endif