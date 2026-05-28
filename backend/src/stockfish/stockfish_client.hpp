#pragma once

#include <cstdint>
#include <string>

namespace smart_chess::stockfish {

class StockfishClient {
public:
    explicit StockfishClient(std::string executable_path = "");

    std::string GetBestMove(
        const std::string& fen,
        std::int32_t engine_level
    ) const;

private:
    std::string executable_path_;
};

}  // namespace smart_chess::stockfish