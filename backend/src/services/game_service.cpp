#include "game_service.hpp"
#include "../models/move.hpp"
#include <stdexcept>
#include <utility>
#include "chess_service.hpp"
#include "../stockfish/stockfish_client.hpp"

namespace {

struct GameEndState {
    std::string status = "in_progress";
    std::optional<std::string> result = std::nullopt;
};

GameEndState BuildGameEndState(
    const smart_chess::chess::ApplyMoveResult& apply_move_result
) {
    GameEndState state{};

    if (apply_move_result.is_checkmate) {
        state.status = "checkmate";

        if (apply_move_result.side_to_move_after == "white") {
            state.result = "black_win";
        } else {
            state.result = "white_win";
        }
    } else if (apply_move_result.is_draw) {
        state.status = "draw";
        state.result = "draw";
    }

    return state;
}


    
constexpr std::string_view kStartFen =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

void ValidateMoveUci(const std::string& move_uci) {
    if (move_uci.size() < 4 || move_uci.size() > 5) {
        throw std::invalid_argument("move_uci must be in UCI format");
    }
}

}  // namespace

namespace smart_chess::services {

GameService::GameService(repositories::GameRepository game_repository)
    : game_repository_(std::move(game_repository)) {}

models::Game GameService::CreateGame(
    const models::CreateGameRequest& request
) const {
    if (request.user_side != "white" && request.user_side != "black") {
        throw std::invalid_argument("user_side must be white or black");
    }

    if (request.engine_level < 1 || request.engine_level > 8) {
        throw std::invalid_argument("engine_level must be between 1 and 8");
    }

    if (request.user_side == "black") {
        chess::ChessService chess_service;
        stockfish::StockfishClient stockfish_client;

        const auto engine_move_uci = stockfish_client.GetBestMove(
            std::string{kStartFen},
            request.engine_level
        );

        const auto engine_apply_result = chess_service.ApplyMove(
            std::string{kStartFen},
            engine_move_uci
        );

        const auto state = BuildGameEndState(engine_apply_result);

        return game_repository_.CreateGameWithInitialMove(
            request,
            engine_move_uci,
            engine_apply_result.side_moved,
            std::string{kStartFen},
            engine_apply_result.fen_after,
            engine_apply_result.side_to_move_after,
            state.status,
            state.result
        );
    }

    return game_repository_.CreateGame(request);
}

std::optional<models::Game> GameService::GetGameById(const std::string& game_id, const std::string& owner_id) const {
    if (game_id.empty()) {
        throw std::invalid_argument("game_id is empty");
    }
    if (owner_id.empty()) {
        throw std::invalid_argument("owner_id is empty");
    }
    return game_repository_.GetGameById(game_id, owner_id);
}

std::vector<models::Game> GameService::GetGamesByOwner(const std::string& owner_id, std::int32_t limit) const {
    if (owner_id.empty()) {
        throw std::invalid_argument("owner_id is empty");
    }
    if (limit < 1 || limit > 100) {
        throw std::invalid_argument("limit have to be between 1 and 100");
    }
    return game_repository_.GetGamesByOwner(owner_id, limit);
}

std::optional<models::MakeMoveResult> GameService::MakeMove(const models::MakeMoveRequest& request) const {
    if (request.game_id.empty()) {
        throw std::invalid_argument("game_id is empty");
    }

    if (request.owner_id.empty()) {
        throw std::invalid_argument("owner_id is empty");
    }

    ValidateMoveUci(request.move_uci);

    const auto game = game_repository_.GetGameById(
        request.game_id,
        request.owner_id
    );

    if (!game.has_value()) {
        return std::nullopt;
    }

    if (game->status != "in_progress") {
        throw std::invalid_argument("game is not in progress");
    }

    if (game->side_to_move != game->user_side) {
        throw std::invalid_argument("it is not your turn");
    }

    const auto moves_count = game_repository_.GetMovesCount(game->id);

    const std::int32_t move_number = moves_count / 2 + 1;

    const std::string fen_before = game->current_fen;

    chess::ChessService chess_service;
    const auto apply_move_result = chess_service.ApplyMove(fen_before, request.move_uci);
    if (apply_move_result.side_moved != game->user_side) {
        throw std::invalid_argument("you can move only your side");
    }
    const std::string fen_after = apply_move_result.fen_after;
    const std::string side = apply_move_result.side_moved;

    const auto move = game_repository_.CreateMove(
        game->id,
        request.move_uci,
        move_number,
        side,
        fen_before,
        fen_after
    );

    std::string final_fen = fen_after;
    std::string final_side_to_move = apply_move_result.side_to_move_after;
    auto final_state = BuildGameEndState(apply_move_result);

    if (
        final_state.status == "in_progress" &&
        apply_move_result.side_to_move_after != game->user_side
    ) {
        stockfish::StockfishClient stockfish_client;

        const auto engine_move_uci = stockfish_client.GetBestMove(
            final_fen,
            game->engine_level
        );

        const auto engine_apply_result = chess_service.ApplyMove(
            final_fen,
            engine_move_uci
        );

        const std::int32_t engine_move_number = (moves_count + 1) / 2 + 1;

        game_repository_.CreateMove(
            game->id,
            engine_move_uci,
            engine_move_number,
            engine_apply_result.side_moved,
            final_fen,
            engine_apply_result.fen_after
        );

        final_fen = engine_apply_result.fen_after;
        final_side_to_move = engine_apply_result.side_to_move_after;
        final_state = BuildGameEndState(engine_apply_result);
    }

    const auto updated_game = game_repository_.UpdateGameAfterMove(
        game->id,
        request.owner_id,
        final_fen,
        final_side_to_move,
        final_state.status,
        final_state.result
    );

    models::MakeMoveResult result{};
    result.move = move;
    result.game = updated_game;

    return result;
}

std::optional<std::vector<std::string>> GameService::GetLegalDestinations(
    const std::string& game_id,
    const std::string& owner_id,
    const std::string& from
) const {
    if (game_id.empty()) {
        throw std::invalid_argument("game_id is empty");
    }

    if (owner_id.empty()) {
        throw std::invalid_argument("owner_id is empty");
    }

    const auto game = game_repository_.GetGameById(game_id, owner_id);

    if (!game.has_value()) {
        return std::nullopt;
    }

    if (game->status != "in_progress") {
        throw std::invalid_argument("game is not in progress");
    }

    if (game->side_to_move != game->user_side) {
        throw std::invalid_argument("it is not your turn");
    }

    chess::ChessService chess_service;

    return chess_service.GetLegalDestinations(game->current_fen, from);
}

std::optional<models::Game> GameService::ResignGame(
    const std::string& game_id,
    const std::string& owner_id
) const {
    if (game_id.empty()) {
        throw std::invalid_argument("game_id is empty");
    }

    if (owner_id.empty()) {
        throw std::invalid_argument("owner_id is empty");
    }

    const auto game = game_repository_.GetGameById(game_id, owner_id);

    if (!game.has_value()) {
        return std::nullopt;
    }

    if (game->status != "in_progress") {
        throw std::invalid_argument("game is not in progress");
    }

    const std::string result =
        game->user_side == "white" ? "black_win" : "white_win";

    return game_repository_.UpdateGameStatus(
        game_id,
        owner_id,
        "resigned",
        result
    );
}

bool GameService::DeleteGame(
    const std::string& game_id,
    const std::string& owner_id
) const {
    if (game_id.empty()) {
        throw std::invalid_argument("game_id is empty");
    }

    if (owner_id.empty()) {
        throw std::invalid_argument("owner_id is empty");
    }

    return game_repository_.DeleteGame(game_id, owner_id);
}

}  // namespace smart_chess::services