#include "stockfish_client.hpp"

#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <string>
#include <cstdlib>

#include <boost/process.hpp>

namespace smart_chess::stockfish {

namespace {

std::string GetDefaultStockfishPath() {
    const char* env_path = std::getenv("STOCKFISH_PATH");

    if (env_path != nullptr && std::string{env_path}.size() > 0) {
        return std::string{env_path};
    }

    return "/usr/games/stockfish";
}

namespace bp = boost::process;

int DepthFromLevel(std::int32_t engine_level) {
    if (engine_level < 1 || engine_level > 8) {
        throw std::invalid_argument("engine_level must be between 1 and 8");
    }

    return 2 + engine_level * 2;
}

int SkillFromLevel(std::int32_t engine_level) {
    if (engine_level < 1 || engine_level > 8) {
        throw std::invalid_argument("engine_level must be between 1 and 8");
    }

    return std::clamp(engine_level * 2, 0, 20);
}

void SendCommand(bp::opstream& input, const std::string& command) {
    input << command << std::endl;
}

void WaitForLine(bp::ipstream& output, const std::string& expected_line) {
    std::string line;

    while (std::getline(output, line)) {
        if (line == expected_line) {
            return;
        }
    }

    throw std::runtime_error("stockfish did not return " + expected_line);
}

std::string ReadBestMove(bp::ipstream& output) {
    std::string line;

    while (std::getline(output, line)) {
        if (line.rfind("bestmove ", 0) == 0) {
            std::istringstream stream(line);

            std::string tag;
            std::string best_move;

            stream >> tag >> best_move;

            if (best_move.empty() || best_move == "(none)") {
                throw std::runtime_error("stockfish returned empty bestmove");
            }

            return best_move;
        }
    }

    throw std::runtime_error("stockfish did not return bestmove");
}

}  // namespace

StockfishClient::StockfishClient(std::string executable_path)
    : executable_path_(
          executable_path.empty()
              ? GetDefaultStockfishPath()
              : std::move(executable_path)
      ) {}

std::string StockfishClient::GetBestMove(
    const std::string& fen,
    std::int32_t engine_level
) const {
    bp::ipstream engine_output;
    bp::opstream engine_input;

    bp::child engine(
        executable_path_,
        bp::std_out > engine_output,
        bp::std_in < engine_input
    );

    try {
        SendCommand(engine_input, "uci");
        WaitForLine(engine_output, "uciok");

        SendCommand(
            engine_input,
            "setoption name Skill Level value " + std::to_string(SkillFromLevel(engine_level))
        );

        SendCommand(engine_input, "isready");
        WaitForLine(engine_output, "readyok");

        SendCommand(engine_input, "ucinewgame");
        SendCommand(engine_input, "position fen " + fen);
        SendCommand(engine_input, "go depth " + std::to_string(DepthFromLevel(engine_level)));

        const auto best_move = ReadBestMove(engine_output);

        SendCommand(engine_input, "quit");
        engine.wait();

        return best_move;
    } catch (...) {
        if (engine.running()) {
            engine.terminate();
        }

        throw;
    }
}

}  // namespace smart_chess::stockfish