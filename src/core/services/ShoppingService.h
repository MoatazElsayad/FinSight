#pragma once

#include "../models/Shopping.h"

#include <cstddef>
#include <string>
#include <vector>

using namespace std;

namespace finsight::core::services {

class ShoppingService {
public:
    // Creates a new pantry item or updates an existing one.
    models::PantryItem upsertPantryItem(models::PantryItem item);
    // Deletes a pantry item.
    void deletePantryItem(const std::string& userId, const std::string& itemId);
    // Returns all pantry items for one user.
    std::vector<models::PantryItem> listPantryItems(const std::string& userId) const;

    // Adds a new shopping list item.
    models::ShoppingItem addShoppingItem(models::ShoppingItem item);
    // Marks a shopping list item as purchased or not purchased.
    models::ShoppingItem markPurchased(const std::string& userId, const std::string& itemId, bool purchased);
    // Deletes a shopping list item.
    void deleteShoppingItem(const std::string& userId, const std::string& itemId);
    // Returns all shopping list items for one user.
    std::vector<models::ShoppingItem> listShoppingItems(const std::string& userId) const;

    // Builds a combined pantry and shopping snapshot.
    models::ShoppingSnapshot snapshot(const std::string& userId) const;
    // Returns every stored pantry item.
    std::vector<models::PantryItem> allPantryItems() const;
    // Returns every stored shopping item.
    std::vector<models::ShoppingItem> allShoppingItems() const;
    // Replaces shopping state from persisted data.
    void loadState(std::vector<models::PantryItem> pantryItems,
                   std::vector<models::ShoppingItem> shoppingItems);

private:
    // Generates the next pantry item id.
    std::string nextPantryId();
    // Generates the next shopping item id.
    std::string nextShoppingId();

    std::vector<models::PantryItem> pantryItems_;
    std::vector<models::ShoppingItem> shoppingItems_;
    std::size_t nextPantryId_ {1};
    std::size_t nextShoppingId_ {1};
};

}  // namespace finsight::core::services
