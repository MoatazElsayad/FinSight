#pragma once

#include "Common.h"

#include <string>
#include <vector>

namespace finsight::core::models {

struct PantryItem {
    std::string id;
    std::string userId;
    std::string name;
    std::string unit;
    double quantity {0.0};
    double lowStockThreshold {0.0};
    Date updatedAt;
};

struct ShoppingItem {
    std::string id;
    std::string userId;
    std::string name;
    std::string category;
    double plannedQuantity {0.0};
    std::string unit;
    bool purchased {false};
    Date createdAt;
};

struct ShoppingSnapshot {
    std::vector<PantryItem> pantryItems;
    std::vector<ShoppingItem> shoppingItems;
    std::vector<PantryItem> lowStockItems;
};

}  // namespace finsight::core::models
