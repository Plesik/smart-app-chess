#include "handler_utils.hpp"

#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/json/serialize.hpp>

namespace smart_chess::handlers::utils {

std::string GetOwnerId(const userver::server::http::HttpRequest& request) {
    const auto owner_id = request.GetHeader("X-Owner-Id");

    if (owner_id.empty()) {
        return "local-dev-user";
    }

    return std::string{owner_id};
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

}  // namespace smart_chess::handlers::utils