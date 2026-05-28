#include "get_games_list_handler.hpp"

#include <stdexcept>
#include <string>

#include <userver/components/component_context.hpp>
#include <userver/formats/json.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/storages/postgres/component.hpp>

#include "../models/game.hpp"
#include "../repositories/game_repository.hpp"
#include "../services/game_service.hpp"
#include "handler_utils.hpp"

namespace smart_chess::handlers {

GetGamesListHandler::GetGamesListHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : HttpHandlerBase(config, context),
      pg_cluster_(
          context.FindComponent<userver::components::Postgres>("postgres-db-1")
              .GetCluster()
      ) {}

std::string GetGamesListHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    try {
        const auto owner_id = utils::GetOwnerId(request);
        const std::string limit_str = request.GetArg("limit");
        std::int32_t limit = 20;
        if (!limit_str.empty()) {
            try {
                limit = std::stoi(limit_str);
            } catch (const std::exception&) {
                throw std::invalid_argument("limit must be integer");
            }
        }

        repositories::GameRepository repository{pg_cluster_};
        services::GameService service{repository};

        const auto games = service.GetGamesByOwner(owner_id, limit);

        return userver::formats::json::ToString(utils::MakeGamesListJson(games));
    } catch (const std::invalid_argument& ex) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        return utils::MakeErrorJson("VALIDATION_ERROR", ex.what());
    } catch (const std::exception& ex) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kInternalServerError);
        return utils::MakeErrorJson("INTERNAL_ERROR", ex.what());
    }
}

}  // namespace smart_chess::handlers