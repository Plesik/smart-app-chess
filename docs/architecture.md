# Архитектура проекта

## Общая идея

SmartApp Chess — это голосовая шахматная игра для Sber SmartApp, где пользователь играет против Stockfish.

Основная backend-логика находится в C++ сервисе на userver. Backend отвечает за создание партий, хранение состояния игры, проверку ходов, вызов Stockfish и работу с PostgreSQL.

## Компоненты системы

```text
Пользователь
  ↓ голос / интерфейс
Sber SmartApp
  ↓ распознанный текст / команда
SmartApp Code
  ↓ HTTP-запрос
C++ backend на userver
  ↓ SQL
PostgreSQL

C++ backend на userver
  ↓ UCI protocol
Stockfish

Canvas App frontend
  ↓ HTTP-запрос
C++ backend на userver