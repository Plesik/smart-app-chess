#pragma once

#include <string>

#include <userver/formats/json/value.hpp>
#include <userver/server/http/http_request.hpp>

#include "../models/game.hpp"

namespace smart_chess::handlers::utils {

std::string GetOwnerId(const userver::server::http::HttpRequest& request);

userver::formats::json::Value MakeGameJson(const models::Game& game);

std::string MakeErrorJson(
    const std::string& code,
    const std::string& message
);

}  // namespace smart_chess::handlers::utils