cmake_minimum_required(VERSION 3.20)
project(FinSight LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(SQLite3 QUIET)

if(NOT SQLite3_FOUND)
    find_path(SQLITE3_INCLUDE_DIR sqlite3.h)
    find_library(SQLITE3_LIBRARY NAMES sqlite3)

    if(SQLITE3_INCLUDE_DIR AND SQLITE3_LIBRARY)
        add_library(SQLite::SQLite3 UNKNOWN IMPORTED)
        set_target_properties(SQLite::SQLite3 PROPERTIES
            IMPORTED_LOCATION "${SQLITE3_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${SQLITE3_INCLUDE_DIR}"
        )
        set(SQLite3_FOUND TRUE)
    else()
        message(FATAL_ERROR
            "SQLite3 was not found. Install the SQLite development package "
            "(for Ubuntu/WSL: sudo apt install libsqlite3-dev) and rerun CMake."
        )
    endif()
endif()

find_package(Qt6 COMPONENTS Widgets QUIET)

if(Qt6_FOUND)
    set(QT_PACKAGE Qt6)
else()
    find_package(Qt5 REQUIRED COMPONENTS Widgets)
    set(QT_PACKAGE Qt5)
endif()

add_library(finsight_core
    src/core/managers/FinanceTrackerBackend.cpp
    src/core/utils/EnvLoader.cpp
    src/core/services/AIService.cpp
    src/core/services/AnalyticsService.cpp
    src/core/services/AuthService.cpp
    src/core/services/BudgetAlertService.cpp
    src/core/services/BudgetService.cpp
    src/core/services/EmailService.cpp
    src/core/services/GoalService.cpp
    src/core/services/ReceiptService.cpp
    src/core/services/ReportService.cpp
    src/core/services/SavingsService.cpp
    src/core/services/SessionService.cpp
    src/core/services/ShoppingService.cpp
    src/core/services/TransactionService.cpp
    src/data/json/TextCodec.cpp
    src/data/storage/BackendStore.cpp
    src/network/ai/ChatCompletionClient.cpp
    src/network/email/ResendEmailClient.cpp
)

target_include_directories(finsight_core PUBLIC src)
target_link_libraries(finsight_core PUBLIC SQLite::SQLite3)

add_executable(finsight_backend_demo src/main.cpp)
target_link_libraries(finsight_backend_demo PRIVATE finsight_core)

add_executable(finsight_terminal_demo src/main_test.cpp)
target_link_libraries(finsight_terminal_demo PRIVATE finsight_core)

add_executable(finsight_gui
    src/gui/main_gui.cpp

    src/gui/MainWindow.cpp
    src/gui/MainWindow.h

    src/gui/auth/LoginDialog.cpp
    src/gui/auth/LoginDialog.h
    src/gui/auth/RegisterDialog.cpp
    src/gui/auth/RegisterDialog.h

    src/gui/dashboard/DashboardWindow.cpp
    src/gui/dashboard/DashboardWindow.h

    src/gui/profile/ProfileWindow.cpp
    src/gui/profile/ProfileWindow.h

    src/gui/transactions/TransactionsWindow.cpp
    src/gui/transactions/TransactionsWindow.h
    src/gui/transactions/TransactionDialog.cpp
    src/gui/transactions/TransactionDialog.h

    src/gui/budgets/BudgetsWindow.cpp
    src/gui/budgets/BudgetsWindow.h

    src/gui/ai/AIInsightsWindow.cpp
    src/gui/ai/AIInsightsWindow.h

    src/gui/savings/SavingsWindow.cpp
    src/gui/savings/SavingsWindow.h

    src/gui/investments/InvestmentsWindow.cpp
    src/gui/investments/InvestmentsWindow.h

    src/gui/goals/GoalsWindow.cpp
    src/gui/goals/GoalsWindow.h

    src/gui/reports/ReportsWindow.cpp
    src/gui/reports/ReportsWindow.h

    src/gui/receipts/ReceiptsWindow.cpp
    src/gui/receipts/ReceiptsWindow.h

    src/gui/categories/CategoriesWindow.cpp
    src/gui/categories/CategoriesWindow.h
)

target_include_directories(finsight_gui PRIVATE src)

target_link_libraries(finsight_gui PRIVATE
    finsight_core
    ${QT_PACKAGE}::Widgets
)
