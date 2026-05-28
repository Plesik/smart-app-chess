# API

## Общая информация

Backend предоставляет REST API для шахматной игры против Stockfish.

API используется двумя клиентами:

- Canvas App frontend;
- SmartApp Code сценарий.

Backend не занимается распознаванием речи. Голос пользователя обрабатывается на стороне Sber SmartApp / SmartApp Code. Backend получает уже нормализованные данные, например ход в формате `e2e4`.

## Base URL

Для локальной разработки:

```text
http://localhost:8080
```

## Версия API

Все основные методы находятся под префиксом:

```text
/v1
```

## Идентификация пользователя

В MVP нет регистрации и авторизации.

Для связи партий с пользователем используется HTTP-header:

```http
X-Owner-Id: local-dev-user
```

В production значение `X-Owner-Id` будет формироваться на стороне SmartApp Code из идентификатора пользователя Sber SmartApp.

Если header не передан в локальной разработке, backend может использовать значение по умолчанию:

```text
local-dev-user
```

## Форматы данных

### Цвет стороны

```text
white
black
```

### Уровень Stockfish

```text
easy
normal
hard
```

### Статус партии

```text
in_progress
checkmate
draw
resigned
```

### Результат партии

```text
white_win
black_win
draw
null
```

### Ход

Ход внутри backend хранится в формате UCI:

```text
e2e4
g1f3
e7e8q
e1g1
```

Примеры:

- `e2e4` — пешка с e2 на e4;
- `g1f3` — конь с g1 на f3;
- `e7e8q` — превращение пешки в ферзя;
- `e1g1` — короткая рокировка белых.

### Состояние доски

Состояние доски хранится и передаётся в формате FEN.

---

# Healthcheck

## GET /ping

Проверяет, что backend работает.

### Request

```http
GET /ping
```

### Response 200

```json
{
  "status": "ok"
}
```

---

# Games

## POST /v1/games

Создаёт новую партию против Stockfish.

Если пользователь выбрал белые, первым ходит пользователь.

Если пользователь выбрал чёрные, backend сразу вызывает Stockfish, применяет ход движка и возвращает позицию после первого хода Stockfish.

### Request

```http
POST /v1/games
X-Owner-Id: local-dev-user
Content-Type: application/json
```

```json
{
  "player_name": "Антон",
  "player_color": "white",
  "engine_level": "normal"
}
```

### Request fields

| Field | Type | Required | Description |
|---|---|---:|---|
| `player_name` | string | no | Имя игрока |
| `player_color` | string | yes | Цвет игрока: `white` или `black` |
| `engine_level` | string | yes | Уровень Stockfish: `easy`, `normal`, `hard` |

### Response 201

```json
{
  "game_id": "b2fd6f8e-8a7e-4f4c-8f08-62a08df2d111",
  "status": "in_progress",
  "player_name": "Антон",
  "player_color": "white",
  "engine_level": "normal",
  "fen": "startpos",
  "turn": "white",
  "last_move": null,
  "engine_move": null,
  "created_at": "2026-05-13T12:00:00Z",
  "updated_at": "2026-05-13T12:00:00Z"
}
```

### Response 201, если игрок выбрал чёрные

```json
{
  "game_id": "b2fd6f8e-8a7e-4f4c-8f08-62a08df2d111",
  "status": "in_progress",
  "player_name": "Антон",
  "player_color": "black",
  "engine_level": "normal",
  "fen": "fen_after_stockfish_first_move",
  "turn": "black",
  "last_move": "e2e4",
  "engine_move": "e2e4",
  "created_at": "2026-05-13T12:00:00Z",
  "updated_at": "2026-05-13T12:00:01Z"
}
```

---

## GET /v1/games

Возвращает список партий пользователя.

### Request

```http
GET /v1/games
X-Owner-Id: local-dev-user
```

### Query parameters

| Parameter | Type | Required | Description |
|---|---|---:|---|
| `limit` | integer | no | Количество партий, по умолчанию `20` |
| `offset` | integer | no | Смещение, по умолчанию `0` |
| `status` | string | no | Фильтр по статусу партии |

### Response 200

```json
{
  "games": [
    {
      "game_id": "b2fd6f8e-8a7e-4f4c-8f08-62a08df2d111",
      "status": "in_progress",
      "player_name": "Антон",
      "player_color": "white",
      "engine_level": "normal",
      "fen": "current_fen",
      "turn": "white",
      "result": null,
      "created_at": "2026-05-13T12:00:00Z",
      "updated_at": "2026-05-13T12:10:00Z"
    }
  ],
  "limit": 20,
  "offset": 0
}
```

---

## GET /v1/games/{game_id}

Возвращает текущее состояние конкретной партии.

### Request

```http
GET /v1/games/{game_id}
X-Owner-Id: local-dev-user
```

### Response 200

```json
{
  "game_id": "b2fd6f8e-8a7e-4f4c-8f08-62a08df2d111",
  "status": "in_progress",
  "player_name": "Антон",
  "player_color": "white",
  "engine_level": "normal",
  "fen": "current_fen",
  "turn": "white",
  "last_move": "e7e5",
  "result": null,
  "created_at": "2026-05-13T12:00:00Z",
  "updated_at": "2026-05-13T12:10:00Z"
}
```

### Response 404

```json
{
  "error": {
    "code": "GAME_NOT_FOUND",
    "message": "Game not found"
  }
}
```

---

# Moves

## POST /v1/games/{game_id}/moves

Делает ход игрока.

Backend:

1. Проверяет, что партия существует.
2. Проверяет, что партия принадлежит пользователю.
3. Проверяет, что партия ещё идёт.
4. Проверяет легальность хода.
5. Применяет ход игрока.
6. Сохраняет ход игрока и новый FEN.
7. Если партия не закончилась, вызывает Stockfish.
8. Применяет ход Stockfish.
9. Сохраняет ход Stockfish и новый FEN.
10. Возвращает актуальное состояние партии.

### Request

```http
POST /v1/games/{game_id}/moves
X-Owner-Id: local-dev-user
Content-Type: application/json
```

```json
{
  "move": "e2e4"
}
```

### Request fields

| Field | Type | Required | Description |
|---|---|---:|---|
| `move` | string | yes | Ход игрока в формате UCI |

### Response 200

```json
{
  "game_id": "b2fd6f8e-8a7e-4f4c-8f08-62a08df2d111",
  "status": "in_progress",
  "fen": "fen_after_engine_move",
  "turn": "white",
  "player_move": "e2e4",
  "engine_move": "e7e5",
  "last_move": "e7e5",
  "result": null
}
```

### Response 200, если партия закончилась после хода игрока

```json
{
  "game_id": "b2fd6f8e-8a7e-4f4c-8f08-62a08df2d111",
  "status": "checkmate",
  "fen": "final_fen",
  "turn": null,
  "player_move": "e7e8q",
  "engine_move": null,
  "last_move": "e7e8q",
  "result": "white_win"
}
```

### Response 400, если ход невалидный

```json
{
  "error": {
    "code": "INVALID_MOVE_FORMAT",
    "message": "Move must be in UCI format"
  }
}
```

### Response 400, если ход нелегальный

```json
{
  "error": {
    "code": "ILLEGAL_MOVE",
    "message": "Move is illegal in current position"
  }
}
```

### Response 409, если партия уже завершена

```json
{
  "error": {
    "code": "GAME_ALREADY_FINISHED",
    "message": "Game is already finished"
  }
}
```

---

## GET /v1/games/{game_id}/moves

Возвращает историю ходов партии.

### Request

```http
GET /v1/games/{game_id}/moves
X-Owner-Id: local-dev-user
```

### Response 200

```json
{
  "game_id": "b2fd6f8e-8a7e-4f4c-8f08-62a08df2d111",
  "moves": [
    {
      "id": 1,
      "move_number": 1,
      "side": "white",
      "move": "e2e4",
      "fen_before": "startpos",
      "fen_after": "fen_after_e2e4",
      "created_at": "2026-05-13T12:01:00Z"
    },
    {
      "id": 2,
      "move_number": 1,
      "side": "black",
      "move": "e7e5",
      "fen_before": "fen_after_e2e4",
      "fen_after": "fen_after_e7e5",
      "created_at": "2026-05-13T12:01:01Z"
    }
  ]
}
```

---

# Resign

## POST /v1/games/{game_id}/resign

Завершает партию сдачей игрока.

### Request

```http
POST /v1/games/{game_id}/resign
X-Owner-Id: local-dev-user
```

### Response 200

```json
{
  "game_id": "b2fd6f8e-8a7e-4f4c-8f08-62a08df2d111",
  "status": "resigned",
  "result": "black_win",
  "updated_at": "2026-05-13T12:15:00Z"
}
```

### Response 409, если партия уже завершена

```json
{
  "error": {
    "code": "GAME_ALREADY_FINISHED",
    "message": "Game is already finished"
  }
}
```

---

# Общий формат ошибок

Все ошибки возвращаются в едином формате:

```json
{
  "error": {
    "code": "ERROR_CODE",
    "message": "Human readable error message"
  }
}
```

## Возможные коды ошибок

| HTTP status | Code | Description |
|---:|---|---|
| 400 | `INVALID_JSON` | Некорректный JSON |
| 400 | `VALIDATION_ERROR` | Ошибка валидации входных данных |
| 400 | `INVALID_MOVE_FORMAT` | Некорректный формат хода |
| 400 | `ILLEGAL_MOVE` | Нелегальный ход |
| 404 | `GAME_NOT_FOUND` | Партия не найдена |
| 409 | `GAME_ALREADY_FINISHED` | Партия уже завершена |
| 500 | `INTERNAL_ERROR` | Внутренняя ошибка backend |
| 503 | `ENGINE_UNAVAILABLE` | Stockfish недоступен |

---

# Что не входит в API MVP

Эти методы не реализуются в первой версии:

```http
POST /v1/games/{game_id}/undo
POST /v1/games/{game_id}/analysis
GET  /v1/games/{game_id}/analysis
PATCH /v1/settings
```

Они могут быть добавлены позже.