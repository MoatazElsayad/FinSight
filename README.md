# FinSight

Desktop personal finance app: **Qt 6 / Qt 5 GUI** on top of a **C++20** core (SQLite + JSON sidecar persistence).

## Features

- Registration, login, profile
- Categories, transactions, budgets, savings, goals
- Dashboard analytics and AI insights (OpenRouter)
- Optional budget alert email (Resend)

## Layout

- `src/core/` — models, services, `FinanceTrackerBackend`
- `src/data/` — persistence (`BackendStore`, codecs)
- `src/network/` — HTTP (OpenRouter chat, Resend), JSON helpers
- `src/gui/` — Qt application entrypoint and windows
- `assets/` — e.g. `logo.svg`
- `runtime_data/` — local DB and sidecar (created at run time next to the executable’s parent folder when run from a normal build layout)

## Build

Requires **CMake 3.20+**, **SQLite3**, **Boost.System**, and **Qt Widgets + Concurrent + Network**.

```bash
cmake -S . -B build
cmake --build build
```

The main target is **`finsight_gui`**.

## Configuration

Copy `.env.example` to `.env` at the project root (or rely on walking upward from the working directory or the executable directory — see `EnvLoader::loadFromNearestFile` in `main_gui.cpp`).

Supported variables match `src/gui/main_gui.cpp`:

- `OPENROUTER_API_KEY`, `OPENROUTER_API_URL`
- `EMAIL_ENABLED`, `EMAIL_API_URL`, `EMAIL_API_KEY`, `EMAIL_FROM_EMAIL`, `EMAIL_FROM_NAME`
