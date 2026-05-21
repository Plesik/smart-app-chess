#include "game_service.hpp"

#include <stdexcept>
#include <utility>

namespace smart_chess::services {

GameService::GameService(repositories::GameRepository game_repository)
    : game_repository_(std::move(game_repository)) {}

models::Game GameService::CreateGame(
    const models::CreateGameRequest& request
) const {
    if (request.user_side != "white" && request.user_side != "black") {
        throw std::invalid_argument("user_side must be white or black");
    }

    if (request.engine_level < 1 || request.engine_level > 8) {
        throw std::invalid_argument("engine_level must be between 1 and 8");
    }

    return game_repository_.CreateGame(request);
}

std::optional<models::Game> GameService::GetGameById(const std::string& game_id, const std::string& owner_id) const {
    if (game_id.empty()) {
        throw std::invalid_argument("game_id is empty");
    }
    if (owner_id.empty()) {
        throw std::invalid_argument("owner_id is empty");
    }
    return game_repository_.GetGameById(game_id, owner_id);
}

}  // namespace smart_chess::services