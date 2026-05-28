#pragma once

#include <string>
#include <cstdint>
#include "game.hpp"


namespace smart_chess::models {
struct MakeMoveRequest {
    std::string game_id;
    std::string owner_id;
    std::string move_uci;
};

struct Move {
    std::int64_t id{};
    std::string game_id;
    std::string move_uci;
    std::int32_t move_number{};
    std::string side;
    std::string fen_before;
    std::string fen_after;
    std::string created_at;
};

struct MakeMoveResult {
    Move move;
    Game game;
};

}