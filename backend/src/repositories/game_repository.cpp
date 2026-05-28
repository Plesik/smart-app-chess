#include "game_repository.hpp"

#include <utility>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/transaction.hpp>


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

std::optional<models::Game> GameRepository::GetGameById(const std::string& game_id, const std::string& owner_id) const {
    const auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        R"(
            SELECT
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
            FROM games
            WHERE id = $1::uuid
              AND owner_id = $2
        )",
        game_id,
        owner_id
    );

    if (result.IsEmpty()) {
        return std::nullopt;
    }

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


std::vector<models::Game> GameRepository::GetGamesByOwner(const std::string& owner_id, std::int32_t limit) const {
        const auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        R"(
            SELECT
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
            FROM games
            WHERE owner_id = $1
            ORDER BY updated_at DESC
            LIMIT $2
        )",
        owner_id,
        limit
    );

    std::vector<models::Game> games;
    games.reserve(result.Size());

    for (const auto& row : result) {
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

        games.push_back(std::move(game));
    }
    return games;
}

std::int32_t GameRepository::GetMovesCount(const std::string& game_id) const {
    const auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        R"(
            SELECT COUNT(*)::int
            FROM moves
            WHERE game_id = $1::uuid
        )",
        game_id
    );

    return result[0][0].As<std::int32_t>();
}

models::Move GameRepository::CreateMove(
    const std::string& game_id,
    const std::string& move_uci,
    std::int32_t move_number,
    const std::string& side,
    const std::string& fen_before,
    const std::string& fen_after
) const {
    const auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        R"(
            INSERT INTO moves (
                game_id,
                move_uci,
                move_number,
                side,
                fen_before,
                fen_after
            )
            VALUES ($1::uuid, $2, $3, $4, $5, $6)
            RETURNING
                id,
                game_id::text,
                move_uci,
                move_number,
                side,
                fen_before,
                fen_after,
                created_at::text
        )",
        game_id,
        move_uci,
        move_number,
        side,
        fen_before,
        fen_after
    );

    const auto row = result[0];

    models::Move move{};
    move.id = row["id"].As<std::int64_t>();
    move.game_id = row["game_id"].As<std::string>();
    move.move_uci = row["move_uci"].As<std::string>();
    move.move_number = row["move_number"].As<std::int32_t>();
    move.side = row["side"].As<std::string>();
    move.fen_before = row["fen_before"].As<std::string>();
    move.fen_after = row["fen_after"].As<std::string>();
    move.created_at = row["created_at"].As<std::string>();

    return move;
}

models::Game GameRepository::UpdateGameAfterMove(
    const std::string& game_id,
    const std::string& owner_id,
    const std::string& current_fen,
    const std::string& side_to_move,
    const std::string& status,
    const std::optional<std::string>& result
) const {
    const auto update_result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        R"(
            UPDATE games
            SET
                current_fen = $3,
                side_to_move = $4,
                status = $5,
                result = $6,
                updated_at = now()
            WHERE id = $1::uuid
              AND owner_id = $2
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
        game_id,
        owner_id,
        current_fen,
        side_to_move,
        status,
        result
    );

    const auto row = update_result[0];

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

std::optional<models::Game> GameRepository::UpdateGameStatus(
    const std::string& game_id,
    const std::string& owner_id,
    const std::string& status,
    const std::optional<std::string>& result
) const {
    const auto update_result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        R"(
            UPDATE games
            SET
                status = $3,
                result = $4,
                updated_at = now()
            WHERE id = $1::uuid
              AND owner_id = $2
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
        game_id,
        owner_id,
        status,
        result
    );

    if (update_result.IsEmpty()) {
        return std::nullopt;
    }

    const auto row = update_result[0];

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

bool GameRepository::DeleteGame(
    const std::string& game_id,
    const std::string& owner_id
) const {
    const auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        R"(
            DELETE FROM games
            WHERE id = $1::uuid
              AND owner_id = $2
            RETURNING id::text
        )",
        game_id,
        owner_id
    );

    return !result.IsEmpty();
}

models::Game GameRepository::CreateGameWithInitialMove(
    const models::CreateGameRequest& request,
    const std::string& move_uci,
    const std::string& move_side,
    const std::string& fen_before,
    const std::string& fen_after,
    const std::string& side_to_move_after,
    const std::string& status,
    const std::optional<std::string>& result
) const {
    auto transaction = pg_cluster_->Begin(
        "create_game_with_initial_move",
        userver::storages::postgres::ClusterHostType::kMaster,
        {}
    );

    const auto game_result = transaction.Execute(
        R"(
            INSERT INTO games (
                owner_id,
                user_name,
                user_side,
                engine_level,
                current_fen,
                status,
                result,
                side_to_move
            )
            VALUES ($1, $2, $3, $4, $5, $6, $7, $8)
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
        fen_after,
        status,
        result,
        side_to_move_after
    );

    const auto game_row = game_result[0];

    models::Game game{};
    game.id = game_row["id"].As<std::string>();
    game.owner_id = game_row["owner_id"].As<std::string>();
    game.user_name = game_row["user_name"].As<std::string>();
    game.user_side = game_row["user_side"].As<std::string>();
    game.engine_level = game_row["engine_level"].As<std::int32_t>();
    game.current_fen = game_row["current_fen"].As<std::string>();
    game.status = game_row["status"].As<std::string>();
    game.result = game_row["result"].As<std::optional<std::string>>();
    game.side_to_move = game_row["side_to_move"].As<std::string>();
    game.created_at = game_row["created_at"].As<std::string>();
    game.updated_at = game_row["updated_at"].As<std::string>();

    transaction.Execute(
        R"(
            INSERT INTO moves (
                game_id,
                move_uci,
                move_number,
                side,
                fen_before,
                fen_after
            )
            VALUES ($1::uuid, $2, $3, $4, $5, $6)
        )",
        game.id,
        move_uci,
        1,
        move_side,
        fen_before,
        fen_after
    );

    transaction.Commit();

    return game;
}

}