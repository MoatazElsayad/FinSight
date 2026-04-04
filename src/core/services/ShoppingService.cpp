#include "ShoppingService.h"

#include <algorithm>
#include <regex>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace finsight::core::services {

// Creates a new pantry item or updates an existing one.
models::PantryItem ShoppingService::upsertPantryItem(models::PantryItem item) {
    if (item.userId.empty() || item.name.empty()) {
        throw std::invalid_argument("Pantry item requires user and name.");
    }

    for (auto& existing : pantryItems_) {
        if (existing.id == item.id && existing.userId == item.userId) {
            existing = item;
            return existing;
        }
    }

    item.id = nextPantryId();
    pantryItems_.push_back(item);
    return item;
}

// Deletes a pantry item.
void ShoppingService::deletePantryItem(const std::string& userId, const std::string& itemId) {
    pantryItems_.erase(std::remove_if(pantryItems_.begin(),
                                      pantryItems_.end(),
                                      [&](const auto& item) {
                                          return item.userId == userId && item.id == itemId;
                                      }),
                       pantryItems_.end());
}

// Returns all pantry items for one user.
std::vector<models::PantryItem> ShoppingService::listPantryItems(const std::string& userId) const {
    std::vector<models::PantryItem> result;
    for (const auto& item : pantryItems_) {
        if (item.userId == userId) {
            result.push_back(item);
        }
    }
    return result;
}

// Adds a new shopping list item.
models::ShoppingItem ShoppingService::addShoppingItem(models::ShoppingItem item) {
    if (item.userId.empty() || item.name.empty()) {
        throw std::invalid_argument("Shopping item requires user and name.");
    }
    item.id = nextShoppingId();
    shoppingItems_.push_back(item);
    return item;
}

// Updates the purchased flag for a shopping item.
models::ShoppingItem ShoppingService::markPurchased(const std::string& userId,
                                                    const std::string& itemId,
                                                    bool purchased) {
    for (auto& item : shoppingItems_) {
        if (item.userId == userId && item.id == itemId) {
            item.purchased = purchased;
            return item;
        }
    }
    throw std::out_of_range("Shopping item not found.");
}

// Deletes a shopping list item.
void ShoppingService::deleteShoppingItem(const std::string& userId, const std::string& itemId) {
    shoppingItems_.erase(std::remove_if(shoppingItems_.begin(),
                                        shoppingItems_.end(),
                                        [&](const auto& item) {
                                            return item.userId == userId && item.id == itemId;
                                        }),
                         shoppingItems_.end());
}

// Returns all shopping list items for one user.
std::vector<models::ShoppingItem> ShoppingService::listShoppingItems(const std::string& userId) const {
    std::vector<models::ShoppingItem> result;
    for (const auto& item : shoppingItems_) {
        if (item.userId == userId) {
            result.push_back(item);
        }
    }
    return result;
}

// Builds a combined pantry and shopping snapshot.
models::ShoppingSnapshot ShoppingService::snapshot(const std::string& userId) const {
    models::ShoppingSnapshot snapshot {
        .pantryItems = listPantryItems(userId),
        .shoppingItems = listShoppingItems(userId),
    };

    for (const auto& item : snapshot.pantryItems) {
        if (item.quantity <= item.lowStockThreshold) {
            snapshot.lowStockItems.push_back(item);
        }
    }
    return snapshot;
}

// Returns every stored pantry item.
std::vector<models::PantryItem> ShoppingService::allPantryItems() const {
    return pantryItems_;
}

// Returns every stored shopping item.
std::vector<models::ShoppingItem> ShoppingService::allShoppingItems() const {
    return shoppingItems_;
}

// Restores pantry and shopping state from persisted data.
void ShoppingService::loadState(std::vector<models::PantryItem> pantryItems,
                                std::vector<models::ShoppingItem> shoppingItems) {
    pantryItems_ = std::move(pantryItems);
    shoppingItems_ = std::move(shoppingItems);

    std::size_t maxPantryId = 0;
    std::size_t maxShoppingId = 0;
    const std::regex pantryPattern(R"(pantry-(\d+))");
    const std::regex shoppingPattern(R"(shop-(\d+))");
    for (const auto& item : pantryItems_) {
        std::smatch match;
        if (std::regex_match(item.id, match, pantryPattern)) {
            maxPantryId = std::max<std::size_t>(maxPantryId, static_cast<std::size_t>(std::stoull(match[1].str())));
        }
    }
    for (const auto& item : shoppingItems_) {
        std::smatch match;
        if (std::regex_match(item.id, match, shoppingPattern)) {
            maxShoppingId = std::max<std::size_t>(maxShoppingId, static_cast<std::size_t>(std::stoull(match[1].str())));
        }
    }
    nextPantryId_ = maxPantryId + 1;
    nextShoppingId_ = maxShoppingId + 1;
}

// Builds the next pantry item id string.
std::string ShoppingService::nextPantryId() {
    std::ostringstream stream;
    stream << "pantry-" << nextPantryId_++;
    return stream.str();
}

// Builds the next shopping item id string.
std::string ShoppingService::nextShoppingId() {
    std::ostringstream stream;
    stream << "shop-" << nextShoppingId_++;
    return stream.str();
}

}  // namespace finsight::core::services
