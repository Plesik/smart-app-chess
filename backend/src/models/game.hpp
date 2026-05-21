#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace smart_chess::models {
struct CreateGameRequest {
    std::string owner_id;
    std::string user_name;
    std::int32_t engine_level;
    std::string user_side;
};

struct Game {
    std::string id;
    std::string current_fen;
    std::string owner_id;
    std::int32_t engine_level;
    std::string user_name;
    std::string user_side;
    std::string status;
    std::optional<std::string> result;
    std::string side_to_move;
    std::string created_at;
    std::string updated_at;
};
}