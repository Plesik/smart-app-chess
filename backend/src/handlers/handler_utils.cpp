#include "handler_utils.hpp"

#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/json/serialize.hpp>

namespace smart_chess::handlers::utils {

std::string GetOwnerId(const userver::server::http::HttpRequest& request) {
    const std::string owner_id = request.GetHeader("X-Owner-Id");

    if (owner_id.empty()) {
        return "local-dev-user";
    }

    return owner_id;
}

userver::formats::json::Value MakeGameJson(const models::Game& game) {
    userver::formats::json::ValueBuilder builder;

    builder["game_id"] = game.id;
    builder["owner_id"] = game.owner_id;
    builder["user_name"] = game.user_name;
    builder["user_side"] = game.user_side;
    builder["engine_level"] = game.engine_level;
    builder["fen"] = game.current_fen;
    builder["status"] = game.status;

    if (game.result.has_value()) {
        builder["result"] = *game.result;
    } else {
        builder["result"] = nullptr;
    }

    builder["side_to_move"] = game.side_to_move;
    builder["created_at"] = game.created_at;
    builder["updated_at"] = game.updated_at;

    return builder.ExtractValue();
}

std::string MakeErrorJson(
    const std::string& code,
    const std::string& message
) {
    userver::formats::json::ValueBuilder builder;

    builder["error"]["code"] = code;
    builder["error"]["message"] = message;

    return userver::formats::json::ToString(builder.ExtractValue());
}

userver::formats::json::Value MakeGamesListJson(const std::vector<models::Game>& games) {
    userver::formats::json::ValueBuilder builder;
    builder["count"] = static_cast<int>(games.size());
    for (auto& game : games) {
        builder["games"].PushBack(MakeGameJson(game));
    }
    return builder.ExtractValue();
}

userver::formats::json::Value MakeMoveJson(const models::Move& move) {
    userver::formats::json::ValueBuilder builder;

    builder["id"] = move.id;
    builder["game_id"] = move.game_id;
    builder["move_uci"] = move.move_uci;
    builder["move_number"] = move.move_number;
    builder["side"] = move.side;
    builder["fen_before"] = move.fen_before;
    builder["fen_after"] = move.fen_after;
    builder["created_at"] = move.created_at;

    return builder.ExtractValue();
}

userver::formats::json::Value MakeMoveResultJson(
    const models::MakeMoveResult& result
) {
    userver::formats::json::ValueBuilder builder;

    builder["move"] = MakeMoveJson(result.move);
    builder["game"] = MakeGameJson(result.game);

    return builder.ExtractValue();
}

}  // namespace smart_chess::handlers::utils