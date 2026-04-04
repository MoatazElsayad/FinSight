#pragma once

#include <string>

using namespace std;

namespace finsight::core::models {

// Describes the finance area a category belongs to.
enum class CategoryKind {
    Income,
    Expense,
    Savings,
    Investment
};

// Represents a built-in or user-created category.
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
