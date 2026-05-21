#include "game_repository.hpp"

#include <utility>
#include <userver/storages/postgres/cluster.hpp>

namespace smart_chess::repositories {
GameRepository::GameRepository(userver::storages::postgres::ClusterPtr pg_cluster) 
    : pg_cluster_(std::move(pg_cluster)) {}
models::Game GameRepository::CreateGame(
    const models::CreateGameRequest& request
) const {
    const auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        R"(
        INSERT INTO games (
            owner_id,
            user_name,
            user_side,
            engine_level,
            current_fen,
            status,
            side_to_move
        )
        VALUES ($1, $2, $3, $4, $5, $6, $7)
            RETURNING
                id::text,
                owner_id,
                user_name,
                user_side,
                engine_level,
                current_fen,
                status,
                result,
                side_to_move,
                created_at::text,
                updated_at::text
        )",
        request.owner_id,
        request.user_name,
        request.user_side,
        request.engine_level,
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        "in_progress",
        "white"
    );

    const auto row = result[0];

    models::Game game{};

    game.id = row["id"].As<std::string>();
    game.owner_id = row["owner_id"].As<std::string>();
    game.user_name = row["user_name"].As<std::string>();
    game.user_side = row["user_side"].As<std::string>();
    game.engine_level = row["engine_level"].As<std::int32_t>();
    game.current_fen = row["current_fen"].As<std::string>();
    game.status = row["status"].As<std::string>();
    game.result = row["result"].As<std::optional<std::string>>();
    game.side_to_move = row["side_to_move"].As<std::string>();
    game.created_at = row["created_at"].As<std::string>();
    game.updated_at = row["updated_at"].As<std::string>();

    return game;
}

}