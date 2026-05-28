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
#include "handler_utils.hpp"

namespace smart_chess::handlers {

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
            .owner_id = utils::GetOwnerId(request),
            .user_name = body["user_name"].As<std::string>(),
            .engine_level = body["engine_level"].As<std::int32_t>(),
            .user_side = body["user_side"].As<std::string>(),
        };

        repositories::GameRepository repository{pg_cluster_};
        services::GameService service{repository};

        const auto game = service.CreateGame(create_request);

        request.SetResponseStatus(userver::server::http::HttpStatus::kCreated);

        return userver::formats::json::ToString(utils::MakeGameJson(game));
    } catch (const std::invalid_argument& ex) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        return utils::MakeErrorJson("VALIDATION_ERROR", ex.what());
    } catch (const std::exception& ex) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kInternalServerError);
        return utils::MakeErrorJson("INTERNAL_ERROR", ex.what());
    }
}

}  // namespace smart_chess::handlers