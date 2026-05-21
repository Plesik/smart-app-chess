#pragma once

#include <userver/storages/postgres/cluster.hpp>
#include "../models/game.hpp"

namespace smart_chess::repositories {

class GameRepository {
public:
    explicit GameRepository(userver::storages::postgres::ClusterPtr pg_cluster);

    models::Game CreateGame(const models::CreateGameRequest& request) const;
    std::optional<models::Game> GetGameById(const std::string& game_id, const std::string& owner_id) const;

private:
    userver::storages::postgres::ClusterPtr pg_cluster_;
};

}  // namespace smart_chess::repositories