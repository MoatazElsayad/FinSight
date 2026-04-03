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
- `src/data/storage`: File-backed backend save/load support

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Current Scope

This backend is intentionally:

- In-memory for live runtime, with file persistence support in `src/data`
- Independent from Qt
- Independent from networking
- Independent from automated tests

That makes it a clean base for your teammates to connect to later using the GUI and transport layers they own.
