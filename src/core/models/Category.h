#pragma once

#include <string>

namespace finsight::core::models {

enum class CategoryKind {
    Income,
    Expense,
    Savings,
    Investment
};

struct Category {
    std::string id;
    std::string userId;
    std::string name;
    std::string icon;
    CategoryKind kind {CategoryKind::Expense};
    bool builtIn {false};
    bool archived {false};
};

}  // namespace finsight::core::models
