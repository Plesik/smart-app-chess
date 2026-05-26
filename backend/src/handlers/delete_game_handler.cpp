#include "delete_game_handler.hpp"

#include <stdexcept>
#include <string>

#include <userver/components/component_context.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/storages/postgres/component.hpp>

#include "../repositories/game_repository.hpp"
#include "../services/game_service.hpp"
#include "handler_utils.hpp"

namespace smart_chess::handlers {

DeleteGameHandler::DeleteGameHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : HttpHandlerBase(config, context),
      pg_cluster_(
          context.FindComponent<userver::components::Postgres>("postgres-db-1")
              .GetCluster()
      ) {}

std::string DeleteGameHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    try {
        const auto game_id = std::string{request.GetPathArg("game_id")};
        const auto owner_id = utils::GetOwnerId(request);

        repositories::GameRepository repository{pg_cluster_};
        services::GameService service{repository};

        const bool deleted = service.DeleteGame(game_id, owner_id);

        if (!deleted) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
            return utils::MakeErrorJson("GAME_NOT_FOUND", "game not found");
        }

        request.SetResponseStatus(userver::server::http::HttpStatus::kNoContent);
        return {};
    } catch (const std::invalid_argument& ex) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        return utils::MakeErrorJson("VALIDATION_ERROR", ex.what());
    } catch (const std::exception& ex) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kInternalServerError);
        return utils::MakeErrorJson("INTERNAL_ERROR", ex.what());
    }
}

}  // namespace smart_chess::handlers