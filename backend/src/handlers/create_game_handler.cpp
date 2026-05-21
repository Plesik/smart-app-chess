#include "create_game_handler.hpp"

#include <optional>
#include <stdexcept>
#include <string>

#include <userver/components/component_context.hpp>
#include <userver/formats/json.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/storages/postgres/component.hpp>

#include "../models/game.hpp"
#include "../repositories/game_repository.hpp"
#include "../services/game_service.hpp"

namespace smart_chess::handlers {

namespace {

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

std::string MakeErrorJson(const std::string& code, const std::string& message) {
    userver::formats::json::ValueBuilder builder;
    builder["error"]["code"] = code;
    builder["error"]["message"] = message;

    return userver::formats::json::ToString(builder.ExtractValue());
}

}  // namespace

CreateGameHandler::CreateGameHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : HttpHandlerBase(config, context),
      pg_cluster_(
          context.FindComponent<userver::components::Postgres>("postgres-db-1")
              .GetCluster()) {}

std::string CreateGameHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    try {
        const auto body = userver::formats::json::FromString(request.RequestBody());

        models::CreateGameRequest create_request{
            .owner_id = GetOwnerId(request),
            .user_name = body["user_name"].As<std::string>(),
            .engine_level = body["engine_level"].As<std::int32_t>(),
            .user_side = body["user_side"].As<std::string>(),
        };

        repositories::GameRepository repository{pg_cluster_};
        services::GameService service{repository};

        const auto game = service.CreateGame(create_request);

        request.SetResponseStatus(userver::server::http::HttpStatus::kCreated);

        return userver::formats::json::ToString(MakeGameJson(game));
    } catch (const std::invalid_argument& ex) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        return MakeErrorJson("VALIDATION_ERROR", ex.what());
    } catch (const std::exception& ex) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kInternalServerError);
        return MakeErrorJson("INTERNAL_ERROR", ex.what());
    }
}

}  // namespace smart_chess::handlers