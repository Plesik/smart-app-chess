#include <userver/clients/dns/component.hpp>
#include <userver/clients/http/component_list.hpp>
#include <userver/components/component.hpp>
#include <userver/components/component_list.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/congestion_control/component.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/server/handlers/tests_control.hpp>
#include <userver/testsuite/testsuite_support.hpp>

#include <userver/storages/postgres/component.hpp> 

#include <userver/utils/daemon_run.hpp>

#include "handlers/create_game_handler.hpp"
#include "handlers/get_game_handler.hpp"
#include "handlers/get_games_list_handler.hpp"
#include "handlers/get_legal_moves_handler.hpp"
#include "handlers/make_move_handler.hpp"
#include "handlers/resign_game_handler.hpp"
#include "handlers/delete_game_handler.hpp"

int main(int argc, char* argv[]) {
    auto component_list =
        userver::components::MinimalServerComponentList()
            .Append<userver::server::handlers::Ping>()
            .AppendComponentList(userver::clients::http::ComponentList())
            .Append<userver::clients::dns::Component>()
            .Append<userver::congestion_control::Component>()
            .Append<userver::components::Postgres>("postgres-db-1")
            .Append<userver::components::TestsuiteSupport>()
            .Append<smart_chess::handlers::CreateGameHandler>()
            .Append<smart_chess::handlers::GetGameHandler>()
            .Append<smart_chess::handlers::GetGamesListHandler>()
            .Append<smart_chess::handlers::MakeMoveHandler>()
            .Append<smart_chess::handlers::ResignGameHandler>()
            .Append<smart_chess::handlers::DeleteGameHandler>()
            .Append<smart_chess::handlers::GetLegalMovesHandler>()
        ;

    return userver::utils::DaemonMain(argc, argv, component_list);
}