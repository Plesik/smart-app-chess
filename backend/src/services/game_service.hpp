#pragma once

#include "../models/game.hpp"
#include <optional>
#include <string>
#include <vector>
#include "../repositories/game_repository.hpp"

namespace smart_chess::services {

class GameService {
public:
    explicit GameService(repositories::GameRepository game_repository);

    models::Game CreateGame(const models::CreateGameRequest& request) const;
    std::optional<models::Game> GetGameById(const std::string& game_id, const std::string& owner_id) const;
    std::vector<models::Game> GetGamesByOwner(const std::string& owner_id, std::int32_t limit) const;
    std::optional<models::MakeMoveResult> MakeMove(const models::MakeMoveRequest& request) const;
    std::optional<models::Game> ResignGame(const std::string& game_id, const std::string& owner_id) const;
    std::optional<std::vector<std::string>> GetLegalDestinations(const std::string& game_id,
        const std::string& owner_id, const std::string& from) const;
    bool DeleteGame(const std::string& game_id, const std::string& owner_id) const;

private:
    repositories::GameRepository game_repository_;
};

}  // namespace smart_chess::services