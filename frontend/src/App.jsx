import { useEffect, useMemo, useRef, useState } from 'react';
import { initSberAssistant } from './sber/assistant';
import './App.css';

import {
  createGame,
  deleteGame,
  getGame,
  getGames,
  getLegalMoves,
  makeMove,
  resignGame,
} from './api';

const PIECES = {
  p: '♟',
  r: '♜',
  n: '♞',
  b: '♝',
  q: '♛',
  k: '♚',
  P: '♙',
  R: '♖',
  N: '♘',
  B: '♗',
  Q: '♕',
  K: '♔',
};

const PIECE_VALUES = {
  p: 1,
  n: 3,
  b: 3,
  r: 5,
  q: 9,
  k: 0,
};

const INITIAL_PIECE_COUNTS = {
  p: 8,
  n: 2,
  b: 2,
  r: 2,
  q: 1,
  k: 1,
};

const CAPTURED_PIECE_ORDER = ['q', 'r', 'b', 'n', 'p'];

function parseFenBoard(fen) {
  const boardPart = fen.split(' ')[0];
  const rows = boardPart.split('/');

  return rows.map((row) => {
    const cells = [];

    for (const char of row) {
      if (Number.isInteger(Number(char)) && char !== '0') {
        for (let i = 0; i < Number(char); ++i) {
          cells.push(null);
        }
      } else {
        cells.push(char);
      }
    }

    return cells;
  });
}

function getOppositeSide(side) {
  return side === 'white' ? 'black' : 'white';
}

function getPieceColor(piece) {
  return piece === piece.toUpperCase() ? 'white' : 'black';
}

function getPieceType(piece) {
  return piece.toLowerCase();
}

function getPieceSymbol(type, color) {
  const piece = color === 'white' ? type.toUpperCase() : type.toLowerCase();
  return PIECES[piece] || '';
}

function getFenPieces(fen) {
  const board = parseFenBoard(fen);
  return board.flat().filter(Boolean);
}

function getCurrentPieceCounts(fen, color) {
  const counts = {
    p: 0,
    n: 0,
    b: 0,
    r: 0,
    q: 0,
    k: 0,
  };

  for (const piece of getFenPieces(fen)) {
    if (getPieceColor(piece) !== color) {
      continue;
    }

    counts[getPieceType(piece)] += 1;
  }

  return counts;
}

function getMissingPieces(fen, color) {
  const currentCounts = getCurrentPieceCounts(fen, color);
  const missing = [];

  for (const type of CAPTURED_PIECE_ORDER) {
    const count = INITIAL_PIECE_COUNTS[type] - currentCounts[type];

    for (let i = 0; i < count; i += 1) {
      missing.push({ type, color });
    }
  }

  return missing;
}

function getMaterialOnBoard(fen, color) {
  return getFenPieces(fen)
    .filter((piece) => getPieceColor(piece) === color)
    .reduce((sum, piece) => sum + PIECE_VALUES[getPieceType(piece)], 0);
}

function getMaterialAdvantage(fen, side) {
  const opponentSide = getOppositeSide(side);

  return (
    getMaterialOnBoard(fen, side) -
    getMaterialOnBoard(fen, opponentSide)
  );
}

function getCapturedPiecesBySide(fen, side) {
  const capturedColor = getOppositeSide(side);
  return getMissingPieces(fen, capturedColor);
}

function formatSide(side) {
  return side === 'white' ? 'белые' : 'чёрные';
}

function normalizeMoveUci(value) {
  return String(value)
    .trim()
    .toLowerCase()
    .replaceAll('-', '')
    .replaceAll(' ', '');
}

function getInitialUserName() {
  return localStorage.getItem('smart_chess_user_name') || 'Anton';
}

function pickRandom(items) {
  return items[Math.floor(Math.random() * items.length)];
}

function getWinnerSideFromResult(result) {
  const normalizedResult = String(result || '').toLowerCase();

  if (normalizedResult.includes('white')) {
    return 'white';
  }

  if (normalizedResult.includes('black')) {
    return 'black';
  }

  return null;
}

function isDrawResult(result) {
  const normalizedResult = String(result || '').toLowerCase();

  return (
    normalizedResult.includes('draw') ||
    normalizedResult.includes('stalemate') ||
    normalizedResult.includes('repetition') ||
    normalizedResult.includes('insufficient') ||
    normalizedResult.includes('fifty') ||
    normalizedResult.includes('50')
  );
}

function isResignResult(result) {
  return String(result || '').toLowerCase().includes('resign');
}

function getGameEndPhrase(game) {
  if (!game || game.status === 'in_progress' || !game.result) {
    return '';
  }

  if (isResignResult(game.result)) {
    return '';
  }

  if (isDrawResult(game.result)) {
    return pickRandom([
      'Ничья. Ровная партия.',
      'Партия завершилась вничью.',
      'Ничья — достойный результат.',
    ]);
  }

  const winnerSide = getWinnerSideFromResult(game.result);

  if (!winnerSide) {
    return '';
  }

  const userSide = game.user_side;
  const userWon = winnerSide === userSide;

  if (userWon) {
    return pickRandom([
      'Отличный мат!',
      'Великолепная партия!',
      'Побеждать всегда приятно!',
      'Отличная игра!',
    ]);
  }

  return pickRandom([
    'Мат. Ничего страшного, реванш всегда возможен.',
    'Партия окончена. Stockfish сегодня был силён.',
    'В этот раз победил движок, но следующая партия может быть вашей.',
    'Мат. Хорошая попытка, можно сыграть ещё раз.',
  ]);
}

function PlayerPanel({ name, side, capturedPieces, advantage }) {
  return (
    <div className="player-panel">
      <div>
        <div className="player-name">{name}</div>
        <div className="player-side">{formatSide(side)}</div>
      </div>

      <div className="captured-area">
        <div className="captured-pieces">
          {capturedPieces.map((piece, index) => (
            <span
              key={`${piece.color}-${piece.type}-${index}`}
              className="captured-piece"
            >
              {getPieceSymbol(piece.type, piece.color)}
            </span>
          ))}
        </div>

        {advantage > 0 && (
          <span className="material-advantage">+{advantage}</span>
        )}
      </div>
    </div>
  );
}

function ChessBoard({ fen, selectedSquare, legalMoves, onSquareClick }) {
  const board = useMemo(() => parseFenBoard(fen), [fen]);

  return (
    <div className="board-wrapper">
      <div className="rank-labels">
        {[8, 7, 6, 5, 4, 3, 2, 1].map((rank) => (
          <div key={rank} className="rank-label">
            {rank}
          </div>
        ))}
      </div>

      <div className="board">
        {board.flatMap((row, rowIndex) =>
          row.map((piece, colIndex) => {
            const isLight = (rowIndex + colIndex) % 2 === 0;
            const file = String.fromCharCode('a'.charCodeAt(0) + colIndex);
            const rank = 8 - rowIndex;
            const square = `${file}${rank}`;

            const isSelected = selectedSquare === square;
            const isLegalTarget = legalMoves.includes(square);
            const isCaptureTarget = isLegalTarget && Boolean(piece);

            return (
              <button
                key={square}
                type="button"
                data-square={square}
                data-focusable="true"
                className={[
                  'cell',
                  isLight ? 'light' : 'dark',
                  isSelected ? 'selected-square' : '',
                  isLegalTarget && !isCaptureTarget ? 'legal-target-empty' : '',
                  isCaptureTarget ? 'legal-target-capture' : '',
                ].join(' ')}
                onClick={() => onSquareClick(square)}
              >
                <span className="piece">{piece ? PIECES[piece] : ''}</span>
              </button>
            );
          })
        )}
      </div>

      <div className="file-labels">
        {['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'].map((file) => (
          <div key={file} className="file-label">
            {file}
          </div>
        ))}
      </div>
    </div>
  );
}

function App() {
  const initialUserName = getInitialUserName();

  const [games, setGames] = useState([]);
  const [currentGame, setCurrentGame] = useState(null);
  const [moveUci, setMoveUci] = useState('e2e4');
  const [userName, setUserName] = useState(initialUserName);
  const [draftUserName, setDraftUserName] = useState(initialUserName);
  const [isEditingName, setIsEditingName] = useState(false);
  const [userSide, setUserSide] = useState('white');
  const [engineLevel, setEngineLevel] = useState(3);
  const [error, setError] = useState('');
  const [infoMessage, setInfoMessage] = useState('');
  const [selectedSquare, setSelectedSquare] = useState(null);
  const [legalMoves, setLegalMoves] = useState([]);
  const appStateRef = useRef({});
  const dispatchCommandRef = useRef(null);

  function showInfo(message) {
    setInfoMessage(message);

    if (message) {
      window.setTimeout(() => {
        setInfoMessage('');
      }, 7000);
    }
  }

  function applyUserName(nextName) {
    const normalizedName = String(nextName || '').trim() || 'Игрок';

    setUserName(normalizedName);
    setDraftUserName(normalizedName);
    localStorage.setItem('smart_chess_user_name', normalizedName);

    return normalizedName;
  }

  async function loadGames() {
    const data = await getGames(20);
    setGames(data.games || []);
  }

  async function dispatchCommand(command) {
    try {
      setError('');
      setInfoMessage('');

      switch (command.type) {
        case 'create_game': {
          const payload = command.payload || {};

          const game = await createGame({
            userName: payload.userName || userName,
            userSide: payload.userSide || userSide,
            engineLevel: payload.engineLevel || engineLevel,
          });

          setCurrentGame(game);
          setSelectedSquare(null);
          setLegalMoves([]);
          await loadGames();
          return;
        }

        case 'make_move': {
          if (!currentGame) {
            setError('Сначала выбери или создай партию');
            return;
          }

          const payload = command.payload || {};
          const normalizedMoveUci = normalizeMoveUci(payload.moveUci);

          if (!normalizedMoveUci) {
            setError('Ход пустой');
            return;
          }

          const result = await makeMove(currentGame.game_id, normalizedMoveUci);

          setCurrentGame(result.game);
          setMoveUci('');
          setSelectedSquare(null);
          setLegalMoves([]);

          const endPhrase = getGameEndPhrase(result.game);
          if (endPhrase) {
            showInfo(endPhrase);
          }

          await loadGames();
          return;
        }

        case 'open_game': {
          const payload = command.payload || {};
          const game = await getGame(payload.gameId);
          setCurrentGame(game);
          setSelectedSquare(null);
          setLegalMoves([]);
          return;
        }

        case 'resign_game': {
          if (!currentGame) {
            setError('Сначала выбери или создай партию');
            return;
          }

          const game = await resignGame(currentGame.game_id);

          setCurrentGame(game);
          setSelectedSquare(null);
          setLegalMoves([]);
          await loadGames();
          return;
        }

        case 'delete_game': {
          if (!currentGame) {
            setError('Сначала выбери партию');
            return;
          }

          await deleteGame(currentGame.game_id);

          setCurrentGame(null);
          setSelectedSquare(null);
          setLegalMoves([]);
          await loadGames();
          return;
        }

        case 'change_user_name': {
          const newName = String(command.payload?.userName || '').trim();

          if (!newName) {
            setError('Не понял новое имя');
            return;
          }

          const appliedName = applyUserName(newName);
          showInfo(`Имя изменено на ${appliedName}`);
          return;
        }

        case 'refresh_games': {
          await loadGames();
          showInfo('Список партий обновлён');
          return;
        }

        default:
          setError(`Неизвестная команда: ${command.type}`);
      }
    } catch (err) {
      setError(err.message);
    }
  }

  useEffect(() => {
    dispatchCommandRef.current = dispatchCommand;
  }, [currentGame, games, userName, userSide, engineLevel, moveUci]);

  async function handleSquareClick(square) {
    if (!currentGame || currentGame.status !== 'in_progress') {
      return;
    }

    if (selectedSquare === square) {
      setSelectedSquare(null);
      setLegalMoves([]);
      return;
    }

    if (selectedSquare && legalMoves.includes(square)) {
      await dispatchCommand({
        type: 'make_move',
        source: 'board_click',
        payload: {
          moveUci: `${selectedSquare}${square}`,
        },
      });

      return;
    }

    try {
      setError('');

      const data = await getLegalMoves(currentGame.game_id, square);
      const moves = data.moves || [];

      if (moves.length === 0) {
        setSelectedSquare(null);
        setLegalMoves([]);
        return;
      }

      setSelectedSquare(square);
      setLegalMoves(moves);
    } catch (err) {
      setSelectedSquare(null);
      setLegalMoves([]);
      setError(err.message);
    }
  }

  useEffect(() => {
    function handleExternalCommand(event) {
      const command = event.detail;

      if (!command?.type) {
        setError('Внешняя команда не содержит type');
        return;
      }

      dispatchCommand({
        ...command,
        source: command.source || 'external',
      });
    }

    window.addEventListener('smartchess:command', handleExternalCommand);

    return () => {
      window.removeEventListener('smartchess:command', handleExternalCommand);
    };
  }, [currentGame]);

  useEffect(() => {
    function getFocusableElements() {
      return Array.from(
        document.querySelectorAll('[data-focusable="true"]:not(:disabled)')
      );
    }

    function focusNextElement(direction) {
      const elements = getFocusableElements();

      if (elements.length === 0) {
        return;
      }

      const activeElement = document.activeElement;
      const currentIndex = elements.indexOf(activeElement);

      if (currentIndex === -1) {
        elements[0].focus();
        return;
      }

      const nextIndex =
        direction === 'next'
          ? (currentIndex + 1) % elements.length
          : (currentIndex - 1 + elements.length) % elements.length;

      elements[nextIndex].focus();
    }

    function focusOutsideBoard(direction) {
      const elements = getFocusableElements();
      const firstBoardIndex = elements.findIndex((element) => element.dataset.square);

      if (firstBoardIndex === -1) {
        focusNextElement(direction);
        return;
      }

      const lastBoardIndex = elements.reduce((lastIndex, element, index) => {
        return element.dataset.square ? index : lastIndex;
      }, firstBoardIndex);

      const targetIndex = direction === 'previous'
        ? firstBoardIndex - 1
        : lastBoardIndex + 1;

      if (targetIndex >= 0 && targetIndex < elements.length) {
        elements[targetIndex].focus();
        return;
      }

      focusNextElement(direction);
    }

    function focusBoardSquare(currentSquare, key) {
      const file = currentSquare[0];
      const rank = Number(currentSquare[1]);

      let fileIndex = file.charCodeAt(0) - 'a'.charCodeAt(0);
      let nextRank = rank;

      if (key === 'ArrowRight') {
        fileIndex += 1;
      }

      if (key === 'ArrowLeft') {
        fileIndex -= 1;
      }

      if (key === 'ArrowUp') {
        nextRank += 1;
      }

      if (key === 'ArrowDown') {
        nextRank -= 1;
      }

      if (fileIndex < 0 || fileIndex > 7 || nextRank < 1 || nextRank > 8) {
        focusOutsideBoard(
          key === 'ArrowUp' || key === 'ArrowLeft' ? 'previous' : 'next'
        );
        return;
      }

      const nextFile = String.fromCharCode('a'.charCodeAt(0) + fileIndex);
      const nextSquare = `${nextFile}${nextRank}`;

      document.querySelector(`[data-square="${nextSquare}"]`)?.focus();
    }

    function handleRemoteKeyDown(event) {
      const activeElement = document.activeElement;
      const activeTag = activeElement?.tagName;
      const activeSquare = activeElement?.dataset?.square;

      const isTextEditing =
        activeTag === 'INPUT' ||
        activeTag === 'TEXTAREA';

      if (activeSquare && event.key.startsWith('Arrow')) {
        event.preventDefault();
        focusBoardSquare(activeSquare, event.key);
        return;
      }

      if (!isTextEditing && (event.key === 'ArrowDown' || event.key === 'ArrowRight')) {
        event.preventDefault();
        focusNextElement('next');
        return;
      }

      if (!isTextEditing && (event.key === 'ArrowUp' || event.key === 'ArrowLeft')) {
        event.preventDefault();
        focusNextElement('previous');
        return;
      }

      if (event.key === 'Enter') {
        if (activeElement?.dataset?.focusable === 'true') {
          event.preventDefault();
          activeElement.click();
        }
        return;
      }

      if (event.key === 'Escape' || event.key === 'Backspace') {
        if (selectedSquare) {
          event.preventDefault();
          setSelectedSquare(null);
          setLegalMoves([]);
          return;
        }

        if (currentGame) {
          event.preventDefault();
          setCurrentGame(null);
        }
      }
    }

    window.addEventListener('keydown', handleRemoteKeyDown);

    return () => {
      window.removeEventListener('keydown', handleRemoteKeyDown);
    };
  }, [selectedSquare, currentGame, games]);

  useEffect(() => {
    appStateRef.current = {
      screen: currentGame ? 'game' : 'menu',
      user: {
        name: userName,
        side: userSide,
        engineLevel,
      },
      game: currentGame
        ? {
            id: currentGame.game_id,
            status: currentGame.status,
            result: currentGame.result,
            sideToMove: currentGame.side_to_move,
            userSide: currentGame.user_side,
            engineLevel: currentGame.engine_level,
            fen: currentGame.fen,
          }
        : null,
      input: {
        moveUci,
        selectedSquare,
        legalMoves,
      },
      item_selector: {
        ignored_words: ['нажми', 'выбери', 'открой', 'покажи'],
        items: [
          {
            title: 'создать партию',
            aliases: ['новая игра', 'начать игру', 'создай игру'],
            action: { type: 'create_game' },
          },
          {
            title: 'сделать ход',
            aliases: ['ход', 'сходи'],
            action: { type: 'make_move' },
          },
          {
            title: 'сдаться',
            aliases: ['я сдаюсь', 'сдаюсь', 'завершить партию'],
            action: { type: 'resign_game' },
          },
          {
            title: 'удалить партию',
            aliases: ['удали игру', 'удалить игру'],
            action: { type: 'delete_game' },
          },
          {
            title: 'обновить список',
            aliases: ['обнови список', 'покажи партии'],
            action: { type: 'refresh_games' },
          },
          {
            title: 'изменить имя',
            aliases: ['поменяй имя', 'смени имя', 'меня зовут'],
            action: { type: 'change_user_name' },
          },
        ],
      },
    };
  }, [
    currentGame,
    userName,
    userSide,
    engineLevel,
    moveUci,
    selectedSquare,
    legalMoves,
  ]);

  useEffect(() => {
    const assistant = initSberAssistant({
      getState: () => appStateRef.current,
      getRecoveryState: () => appStateRef.current,
      onCommand: (command) => {
        dispatchCommandRef.current?.(command);
      },
      onNavigation: (navigation) => {
        if (!navigation?.command) {
          return;
        }

        const keyByCommand = {
          UP: 'ArrowUp',
          DOWN: 'ArrowDown',
          LEFT: 'ArrowLeft',
          RIGHT: 'ArrowRight',
          FORWARD: 'Enter',
          BACK: 'Escape',
        };

        const key = keyByCommand[navigation.command];

        if (key) {
          window.dispatchEvent(new KeyboardEvent('keydown', { key }));
        }
      },
    });

    return () => {
      assistant?.close?.();
    };
  }, []);

  useEffect(() => {
    loadGames().catch((err) => setError(err.message));
  }, []);

  return (
    <main className="app">
      <section className="panel">
        <h1>Голосовые шахматы</h1>

        <div className="form">
          <div className="name-setting">
            <span className="name-label">Имя</span>

            {isEditingName ? (
              <div className="name-edit-row">
                <input
                  data-focusable="true"
                  value={draftUserName}
                  autoFocus
                  onChange={(event) => setDraftUserName(event.target.value)}
                  onKeyDown={(event) => {
                    if (event.key === 'Enter') {
                      applyUserName(draftUserName);
                      setIsEditingName(false);
                    }

                    if (event.key === 'Escape') {
                      setDraftUserName(userName);
                      setIsEditingName(false);
                    }
                  }}
                />
              </div>
            ) : (
              <div className="name-view-row">
                <span className="name-value">{userName}</span>

                <button
                  data-focusable="true"
                  type="button"
                  className="small-button"
                  onClick={() => {
                    setDraftUserName(userName);
                    setIsEditingName(true);
                  }}
                >
                  Изменить
                </button>
              </div>
            )}
          </div>

          <div className="setting-row">
            <div>
              <div className="setting-label">Цвет</div>
              <div className="setting-value">
                {userSide === 'white' ? 'Белые' : 'Чёрные'}
              </div>
            </div>

            <button
              data-focusable="true"
              type="button"
              className="small-button"
              onClick={() =>
                setUserSide((currentSide) =>
                  currentSide === 'white' ? 'black' : 'white'
                )
              }
            >
              Сменить цвет
            </button>
          </div>

          <div className="setting-row">
            <div>
              <div className="setting-label">Уровень Stockfish</div>
              <div className="setting-value">{engineLevel}</div>
            </div>

            <div className="setting-actions">
              <button
                data-focusable="true"
                type="button"
                className="small-button square-button"
                onClick={() =>
                  setEngineLevel((currentLevel) => Math.max(1, currentLevel - 1))
                }
              >
                −
              </button>

              <button
                data-focusable="true"
                type="button"
                className="small-button square-button"
                onClick={() =>
                  setEngineLevel((currentLevel) => Math.min(8, currentLevel + 1))
                }
              >
                +
              </button>
            </div>
          </div>

          <button
            data-focusable="true"
            onClick={() =>
              dispatchCommand({
                type: 'create_game',
                source: 'manual',
                payload: {
                  userName,
                  userSide,
                  engineLevel,
                },
              })
            }
          >
            Создать партию
          </button>
        </div>

        <h2>Партии</h2>

        <button
          data-focusable="true"
          className="secondary"
          onClick={() =>
            dispatchCommand({
              type: 'refresh_games',
              source: 'manual',
            })
          }
        >
          Обновить список
        </button>

        <div className="games">
          {games.map((game) => (
            <button
              key={game.game_id}
              data-focusable="true"
              className={
                currentGame?.game_id === game.game_id
                  ? 'game active'
                  : 'game'
              }
              onClick={() =>
                dispatchCommand({
                  type: 'open_game',
                  source: 'manual',
                  payload: {
                    gameId: game.game_id,
                  },
                })
              }
            >
              <strong>{game.user_name || 'Без имени'}</strong>
              <span>{game.user_side}, level {game.engine_level}</span>
              <span>{game.status}, ход: {game.side_to_move}</span>
            </button>
          ))}
        </div>
      </section>

      <section className="game-area">
        {error && <div className="error">{error}</div>}
        {infoMessage && <div className="info">{infoMessage}</div>}

        {currentGame ? (
          <>
            <div className="game-header">
              <div>
                <h2>Текущая партия</h2>
                <p>ID: {currentGame.game_id}</p>
                <p>Ходит: {currentGame.side_to_move}</p>
                <p>Статус: {currentGame.status}</p>
                <p>Результат: {currentGame.result || 'пока нет'}</p>
              </div>

              <div className="game-actions">
                <button
                  data-focusable="true"
                  className="danger"
                  disabled={currentGame.status !== 'in_progress'}
                  onClick={() =>
                    dispatchCommand({
                      type: 'resign_game',
                      source: 'manual',
                    })
                  }
                >
                  Сдаться
                </button>

                <button
                  data-focusable="true"
                  className="danger-outline"
                  onClick={() =>
                    dispatchCommand({
                      type: 'delete_game',
                      source: 'manual',
                    })
                  }
                >
                  Удалить партию
                </button>
              </div>
            </div>

            {(() => {
              const currentUserSide = currentGame.user_side;
              const stockfishSide = getOppositeSide(currentUserSide);

              const stockfishCapturedPieces = getCapturedPiecesBySide(
                currentGame.fen,
                stockfishSide
              );

              const userCapturedPieces = getCapturedPiecesBySide(
                currentGame.fen,
                currentUserSide
              );

              const stockfishAdvantage = getMaterialAdvantage(
                currentGame.fen,
                stockfishSide
              );

              const userAdvantage = getMaterialAdvantage(
                currentGame.fen,
                currentUserSide
              );

              return (
                <div className="board-section">
                  <PlayerPanel
                    name={`Stockfish level ${currentGame.engine_level}`}
                    side={stockfishSide}
                    capturedPieces={stockfishCapturedPieces}
                    advantage={stockfishAdvantage}
                  />

                  <ChessBoard
                    fen={currentGame.fen}
                    selectedSquare={selectedSquare}
                    legalMoves={legalMoves}
                    onSquareClick={handleSquareClick}
                  />

                  <PlayerPanel
                    name={currentGame.user_name || userName || 'Вы'}
                    side={currentUserSide}
                    capturedPieces={userCapturedPieces}
                    advantage={userAdvantage}
                  />
                </div>
              );
            })()}

            <div className="move-form">
              <input
                data-focusable="true"
                placeholder="e2e4"
                value={moveUci}
                onChange={(event) => setMoveUci(event.target.value)}
                onKeyDown={(event) => {
                  if (event.key === 'Enter') {
                    dispatchCommand({
                      type: 'make_move',
                      source: 'manual',
                      payload: {
                        moveUci,
                      },
                    });
                  }
                }}
              />

              <button
                data-focusable="true"
                onClick={() =>
                  dispatchCommand({
                    type: 'make_move',
                    source: 'manual',
                    payload: {
                      moveUci,
                    },
                  })
                }
              >
                Сделать ход
              </button>
            </div>

            <pre className="fen">{currentGame.fen}</pre>
          </>
        ) : (
          <div className="empty">
            Создай или выбери партию
          </div>
        )}
      </section>
    </main>
  );
}

export default App;