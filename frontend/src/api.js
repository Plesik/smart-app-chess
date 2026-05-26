const OWNER_ID_STORAGE_KEY = 'smart_chess_owner_id';

const API_BASE_URL = import.meta.env.VITE_API_BASE_URL || '/api';

function generateOwnerId() {
  if (window.crypto?.randomUUID) {
    return `smart-chess-${window.crypto.randomUUID()}`;
  }

  return `smart-chess-${Date.now()}-${Math.random().toString(16).slice(2)}`;
}

export function getOwnerId() {
  try {
    const existingOwnerId = window.localStorage.getItem(OWNER_ID_STORAGE_KEY);

    if (existingOwnerId) {
      return existingOwnerId;
    }

    const newOwnerId = generateOwnerId();
    window.localStorage.setItem(OWNER_ID_STORAGE_KEY, newOwnerId);

    return newOwnerId;
  } catch {
    return generateOwnerId();
  }
}

async function request(path, options = {}) {
  const response = await fetch(`${API_BASE_URL}${path}`, {
    ...options,
    headers: {
      'Content-Type': 'application/json',
      'X-Owner-Id': getOwnerId(),
      ...(options.headers || {}),
    },
  });

  const text = await response.text();
  const data = text ? JSON.parse(text) : null;

  if (!response.ok) {
    const message = data?.error?.message || `HTTP error ${response.status}`;
    throw new Error(message);
  }

  return data;
}

export function createGame({ userName, userSide, engineLevel }) {
  return request('/v1/games', {
    method: 'POST',
    body: JSON.stringify({
      user_name: userName,
      user_side: userSide,
      engine_level: Number(engineLevel),
    }),
  });
}

export function getGames(limit = 20) {
  return request(`/v1/games?limit=${limit}`);
}

export function getGame(gameId) {
  return request(`/v1/games/${gameId}`);
}

export function makeMove(gameId, moveUci) {
  return request(`/v1/games/${gameId}/moves`, {
    method: 'POST',
    body: JSON.stringify({
      move_uci: moveUci,
    }),
  });
}

export function resignGame(gameId) {
  return request(`/v1/games/${gameId}/resign`, {
    method: 'POST',
  });
}

export function deleteGame(gameId) {
  return request(`/v1/games/${gameId}`, {
    method: 'DELETE',
  });
}

export function getLegalMoves(gameId, from) {
  return request(`/v1/games/${gameId}/legal-moves?from=${from}`);
}