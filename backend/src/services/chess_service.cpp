#include "chess_service.hpp"

#include <stdexcept>

#include <chess/chess.hpp>
#include <cctype>
#include <string_view>
#include <algorithm>
#include <vector>

namespace {

bool IsFile(char c) {
    return c >= 'a' && c <= 'h';
}

bool IsRank(char c) {
    return c >= '1' && c <= '8';
}

bool IsPromotionPiece(char c) {
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return c == 'q' || c == 'r' || c == 'b' || c == 'n';
}

void ValidateUciFormat(std::string_view move_uci) {
    if (move_uci.size() != 4 && move_uci.size() != 5) {
        throw std::invalid_argument("move_uci must be in UCI format, example: e2e4");
    }

    if (!IsFile(move_uci[0]) || !IsRank(move_uci[1]) ||
        !IsFile(move_uci[2]) || !IsRank(move_uci[3])) {
        throw std::invalid_argument("move_uci must contain valid squares, example: e2e4");
    }

    if (move_uci.size() == 5 && !IsPromotionPiece(move_uci[4])) {
        throw std::invalid_argument("promotion piece must be q, r, b or n");
    }
}

void ValidateUciSquare(std::string_view square) {
    if (square.size() != 2 || !IsFile(square[0]) || !IsRank(square[1])) {
        throw std::invalid_argument("square must be in UCI format, example: e2");
    }
}

bool IsLegalMove(const ::chess::Board& board, const ::chess::Move& move) {
    ::chess::Movelist legal_moves;
    ::chess::movegen::legalmoves(legal_moves, board);

    for (const auto& legal_move : legal_moves) {
        if (legal_move == move) {
            return true;
        }
    }

    return false;
}

std::string ColorToString(::chess::Color color) {
    if (color == ::chess::Color::WHITE) {
        return "white";
    }

    if (color == ::chess::Color::BLACK) {
        return "black";
    }

    throw std::invalid_argument("unknown chess color");
}

}  // namespace

namespace smart_chess::chess {

ApplyMoveResult ChessService::ApplyMove(
    const std::string& fen_before,
    const std::string& move_uci
) const {
    ValidateUciFormat(move_uci);

    ::chess::Board board{fen_before};

    const std::string side_moved = ColorToString(board.sideToMove());

    const auto move = ::chess::uci::uciToMove(board, move_uci);

    if (move == ::chess::Move::NO_MOVE || !IsLegalMove(board, move)) {
        throw std::invalid_argument("illegal move");
    }

    board.makeMove(move);

    const auto game_over = board.isGameOver();
    const auto game_result_reason = game_over.first;

    ApplyMoveResult result{};
    result.fen_after = board.getFen();
    result.side_moved = side_moved;
    result.side_to_move_after = ColorToString(board.sideToMove());

    result.is_checkmate =
        game_result_reason == ::chess::GameResultReason::CHECKMATE;

    result.is_draw =
        game_result_reason == ::chess::GameResultReason::STALEMATE ||
        game_result_reason == ::chess::GameResultReason::INSUFFICIENT_MATERIAL ||
        game_result_reason == ::chess::GameResultReason::FIFTY_MOVE_RULE ||
        game_result_reason == ::chess::GameResultReason::THREEFOLD_REPETITION;

    return result;
}

std::vector<std::string> ChessService::GetLegalDestinations(
    const std::string& fen,
    const std::string& from
) const {
    ValidateUciSquare(from);

    ::chess::Board board{fen};

    ::chess::Movelist legal_moves;
    ::chess::movegen::legalmoves(legal_moves, board);

    std::vector<std::string> destinations;

    for (const auto& move : legal_moves) {
        const auto move_uci = ::chess::uci::moveToUci(move);

        if (move_uci.size() < 4) {
            continue;
        }

        if (move_uci.substr(0, 2) != from) {
            continue;
        }

        const auto destination = move_uci.substr(2, 2);

        if (std::find(destinations.begin(), destinations.end(), destination) ==
            destinations.end()) {
            destinations.push_back(destination);
        }
    }

    return destinations;
}

}  // namespace smart_chess::chess