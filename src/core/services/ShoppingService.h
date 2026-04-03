#pragma once

#include "../models/Shopping.h"

#include <cstddef>
#include <string>
#include <vector>

namespace finsight::core::services {

class ShoppingService {
public:
    models::PantryItem upsertPantryItem(models::PantryItem item);
    void deletePantryItem(const std::string& userId, const std::string& itemId);
    std::vector<models::PantryItem> listPantryItems(const std::string& userId) const;

    models::ShoppingItem addShoppingItem(models::ShoppingItem item);
    models::ShoppingItem markPurchased(const std::string& userId, const std::string& itemId, bool purchased);
    void deleteShoppingItem(const std::string& userId, const std::string& itemId);
    std::vector<models::ShoppingItem> listShoppingItems(const std::string& userId) const;

    models::ShoppingSnapshot snapshot(const std::string& userId) const;
    std::vector<models::PantryItem> allPantryItems() const;
    std::vector<models::ShoppingItem> allShoppingItems() const;
    void loadState(std::vector<models::PantryItem> pantryItems,
                   std::vector<models::ShoppingItem> shoppingItems);

private:
    std::string nextPantryId();
    std::string nextShoppingId();

    std::vector<models::PantryItem> pantryItems_;
    std::vector<models::ShoppingItem> shoppingItems_;
    std::size_t nextPantryId_ {1};
    std::size_t nextShoppingId_ {1};
};

}  // namespace finsight::core::services
