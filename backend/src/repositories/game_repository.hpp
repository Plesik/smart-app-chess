#pragma once

#include <userver/storages/postgres/cluster.hpp>
#include <vector>
#include "../models/game.hpp"
#include "../models/move.hpp"
#include <optional>

namespace smart_chess::repositories {

class GameRepository {
public:
    explicit GameRepository(userver::storages::postgres::ClusterPtr pg_cluster);

    models::Game CreateGame(const models::CreateGameRequest& request) const;
    std::optional<models::Game> GetGameById(const std::string& game_id, const std::string& owner_id) const;
    std::vector<models::Game> GetGamesByOwner(const std::string& owner_id, std::int32_t limit) const;

    std::int32_t GetMovesCount(const std::string& game_id) const;


    models::Move CreateMove(
        const std::string& game_id,
        const std::string& move_uci,
        std::int32_t move_number,
        const std::string& side,
        const std::string& fen_before,
        const std::string& fen_after
    ) const;

    models::Game UpdateGameAfterMove(
        const std::string& game_id,
        const std::string& owner_id,
        const std::string& current_fen,
        const std::string& side_to_move,
        const std::string& status,
        const std::optional<std::string>& result
    ) const;

    models::Game CreateGameWithInitialMove(
        const models::CreateGameRequest& request,
        const std::string& move_uci,
        const std::string& move_side,
        const std::string& fen_before,
        const std::string& fen_after,
        const std::string& side_to_move_after,
        const std::string& status,
        const std::optional<std::string>& result
    ) const;

    std::optional<models::Game> UpdateGameStatus(
        const std::string& game_id,
        const std::string& owner_id,
        const std::string& status,
        const std::optional<std::string>& result
    ) const;

    bool DeleteGame(
        const std::string& game_id,
        const std::string& owner_id
    ) const;
private:
    userver::storages::postgres::ClusterPtr pg_cluster_;
};

}  // namespace smart_chess::repositories