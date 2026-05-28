#pragma once

#include <string>
#include <vector>

namespace smart_chess::chess {

struct ApplyMoveResult {
    std::string fen_after;
    std::string side_moved;
    std::string side_to_move_after;
    bool is_checkmate{};
    bool is_draw{};
};

class ChessService {
public:
    ApplyMoveResult ApplyMove(
        const std::string& fen_before,
        const std::string& move_uci
    ) const;
    std::vector<std::string> GetLegalDestinations(
        const std::string& fen,
        const std::string& from
    ) const;
};

}  // namespace smart_chess::chess