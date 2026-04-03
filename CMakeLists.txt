cmake_minimum_required(VERSION 3.20)
project(FinSightBackend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(finsight_core
    src/core/managers/FinanceTrackerBackend.cpp
    src/core/services/AIService.cpp
    src/core/services/AnalyticsService.cpp
    src/core/services/AuthService.cpp
    src/core/services/BudgetService.cpp
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
)

target_include_directories(finsight_core PUBLIC src)

add_executable(finsight_backend_demo src/main.cpp)
target_link_libraries(finsight_backend_demo PRIVATE finsight_core)
