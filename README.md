# FinSight Backend

This repository now contains the core backend logic for a C++ personal finance tracker, scoped to your team role only:

- User registration, login, and profile updates
- Built-in and custom categories
- Transaction CRUD, bulk delete, filtering, and monthly totals
- Monthly category budgets with budget health summaries
- Savings deposits and withdrawals with monthly and long-term goals
- Investment tracking with current value and profit/loss snapshots
- Financial goals with progress updates
- Session/token state for authenticated users
- Receipt upload, heuristic parsing, and transaction confirmation flow
- Shopping list and pantry state management
- Date-range financial report generation
- Dashboard analytics that combine transactions, budgets, savings, investments, and goals

## Project Structure

- `src/core/models`: Domain models and shared value types
- `src/core/services`: Business logic for each backend feature area
- `src/core/managers`: A single backend facade that coordinates all services
- `src/main.cpp`: Small console demo that exercises the backend without Qt
- `src/core/services/ReceiptService.*`: Receipt intake and parsing flow
- `src/core/services/ShoppingService.*`: Pantry and shopping list state
- `src/core/services/ReportService.*`: Exportable report summaries
- `src/core/services/SessionService.*`: Simple token/session state
- `src/data/json`: Lightweight text escaping/encoding helpers for persistence
- `src/data/storage`: Hybrid persistence with SQLite for core finance data and JSON sidecar files for flexible app state

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Environment File

You can keep your AI provider settings in a local `.env` file at the project root instead of pasting them into source files.

Supported keys:

- `FINSIGHT_OPENROUTER_API_URL`
- `FINSIGHT_OPENROUTER_API_KEY`
- `FINSIGHT_OPENROUTER_MODEL`

Copy `.env.example` to `.env` and fill in your real values. Both `src/main.cpp` and `src/main_test.cpp` load the nearest `.env` file automatically.

## Current Scope

This backend is intentionally:

- In-memory for live runtime, with persistence support in `src/data`
- Independent from Qt
- Independent from networking
- Independent from automated tests

## Persistence Design

The backend now uses a hybrid storage design:

- `SQLite` for structured finance data such as users, transactions, categories, budgets, savings, investments, goals, and sessions
- `JSON` sidecar storage for flexible state such as receipts, parsed receipt results, pantry items, and shopping list items

That makes it a clean base for your teammates to connect to later using the GUI and transport layers they own.
