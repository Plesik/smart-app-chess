CREATE EXTENSION IF NOT EXISTS pgcrypto;

CREATE TABLE IF NOT EXISTS games (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    owner_id TEXT NOT NULL,
    user_name TEXT,
    user_side TEXT NOT NULL CHECK (user_side IN ('white', 'black')),
    engine_level SMALLINT NOT NULL CHECK (engine_level BETWEEN 1 AND 8),
    current_fen TEXT NOT NULL,
    status TEXT NOT NULL CHECK (
        status IN ('in_progress', 'checkmate', 'draw', 'resigned')
    ) DEFAULT 'in_progress',
    result TEXT CHECK (
        result IN ('white_win', 'black_win', 'draw')
    ),
    side_to_move TEXT NOT NULL CHECK (side_to_move IN ('white', 'black')) DEFAULT 'white',
    created_at TIMESTAMPTZ NOT NULL DEFAULT now(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT now()
    
);


CREATE INDEX IF NOT EXISTS idx_games_owner_id ON games(owner_id);

CREATE TABLE IF NOT EXISTS moves (
    id BIGSERIAL PRIMARY KEY,
    game_id UUID NOT NULL REFERENCES games(id) ON DELETE CASCADE,
    move_uci TEXT NOT NULL,
    move_number INT NOT NULL,
    side TEXT NOT NULL CHECK (side IN ('white', 'black')),
    fen_before TEXT NOT NULL,
    fen_after TEXT NOT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE INDEX IF NOT EXISTS idx_moves_game_id ON moves(game_id);
CREATE INDEX IF NOT EXISTS idx_moves_game_id_id ON moves(game_id, id);