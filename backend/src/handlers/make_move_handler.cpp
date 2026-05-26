#include "make_move_handler.hpp"

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
    MakeMoveHandler::MakeMoveHandler(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    )
        : HttpHandlerBase(config, context),
        pg_cluster_(
            context.FindComponent<userver::components::Postgres>("postgres-db-1")
                .GetCluster()
        ) {}

std::string MakeMoveHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    try {
        const auto body = userver::formats::json::FromString(request.RequestBody());

        models::MakeMoveRequest make_move_request{
            .game_id = std::string{request.GetPathArg("game_id")},
            .owner_id = utils::GetOwnerId(request),
            .move_uci = body["move_uci"].As<std::string>(),
        };

        repositories::GameRepository repository{pg_cluster_};
        services::GameService service{repository};

        const auto result = service.MakeMove(make_move_request);

        if (!result.has_value()) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
            return utils::MakeErrorJson("GAME_NOT_FOUND", "game not found");
        }

        return userver::formats::json::ToString(
            utils::MakeMoveResultJson(*result)
        );
    } catch (const std::invalid_argument& ex) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        return utils::MakeErrorJson("VALIDATION_ERROR", ex.what());
    } catch (const std::exception& ex) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kInternalServerError);
        return utils::MakeErrorJson("INTERNAL_ERROR", ex.what());
    }
}

}