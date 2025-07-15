#include "backend_classes.h"
#include <algorithm>
#include <ostream>

using namespace std;

// ------------------ UserDataStore ------------------

void UserDataStore::addUser(const string& username, const string& password, const string& role) {
    users.push_back({username, password, role, "active"});
}

bool UserDataStore::isValidUser(const string& username, const string& password) {
    for (const auto& user : users) {
        if (user.username == username && user.password == password) {
            return true;
        }
    }
    return false;
}

string UserDataStore::getUserRole(const string& username) {
    for (const auto& user : users) {
        if (user.username == username) {
            return user.role;
        }
    }
    return "";
}

bool UserDataStore::userExists(const string& username) {
    for (const auto& user : users) {
        if (user.username == username) return true;
    }
    return false;
}

const vector<UserRecord>& UserDataStore::getAllUsers() const {
    return users;
}

bool UserDataStore::deleteUser(const string& username) {
    auto it = remove_if(users.begin(), users.end(), [&](const UserRecord& u) {
        return u.username == username && u.role != "admin";
    });

    if (it != users.end()) {
        users.erase(it, users.end());
        return true;
    }

    return false;
}

// ------------------ InventoryManager ------------------

void InventoryManager::addItem(const InventoryItem& item) {
    items.push_back(item);
}

void InventoryManager::showItems() const {
    for (const auto& item : items) {
        item.display();
    }
}

bool InventoryManager::getItem(int id, InventoryItem& resultItem) const {
    for (const auto& item : items) {
        if (item.id == id) {
            resultItem = item;
            return true;
        }
    }
    return false;
}

bool InventoryManager::removeItemById(int id) {
    auto it = std::remove_if(items.begin(), items.end(), [=](const InventoryItem& item) {
        return item.id == id;
    });

    if (it != items.end()) {
        items.erase(it, items.end());
        return true;
    }

    return false;
}

void InventoryManager::updateItemQuantity(int id, int newQuantity) {
    for (auto& item : items) {
        if (item.id == id) {
            item.quantity = newQuantity;
            return;
        }
    }
}

const vector<InventoryItem>& InventoryManager::getAllItems() const {
    return items;
}

double InventoryManager::getPrice(int id) const {
    for (const auto& item : items) {
        if (item.id == id) {
            return item.price;
        }
    }
    return 0.0;
}

string InventoryManager::getItemName(int id) const {
    for (const auto& item : items) {
        if (item.id == id) {
            return item.name;
        }
    }
    return "";
}

bool InventoryManager::hasItem(int id) const {
    for (const auto& item : items) {
        if (item.id == id) {
            return true;
        }
    }
    return false;
}

// ------------------ OrderManager ------------------

OrderManager::OrderManager() : nextOrderId(1) {}

bool OrderManager::placeOrder(const string& username, InventoryManager& inventory, int itemId, int quantity) {
    InventoryItem item;
    if (inventory.getItem(itemId, item) && item.quantity >= quantity) {
        inventory.updateItemQuantity(itemId, item.quantity - quantity);
        orders.push_back({nextOrderId++, username, itemId, quantity, "placed"});
        return true;
    }
    return false;
}

vector<Order> OrderManager::getUserOrders(const string& username) const {
    vector<Order> result;
    for (const auto& order : orders) {
        if (order.username == username) {
            result.push_back(order);
        }
    }
    return result;
}

bool OrderManager::removeOrder(const string& username, int itemId) {
    auto originalSize = orders.size();
    orders.erase(std::remove_if(orders.begin(), orders.end(),
                                [&](const Order& order) {
                                    return order.username == username && order.itemId == itemId;
                                }),
                 orders.end());
    return orders.size() < originalSize;
}

bool OrderManager::decreaseOrderQuantity(const string& username, int itemId) {
    for (auto it = orders.begin(); it != orders.end(); ++it) {
        if (it->username == username && it->itemId == itemId) {
            if (it->quantity > 1) {
                it->quantity -= 1;
                return true;
            } else {
                orders.erase(it);
                return true;
            }
        }
    }
    return false;
}

// New method to clear all orders of a user
void OrderManager::clearUserOrders(const string& username) {
    orders.erase(
        remove_if(orders.begin(), orders.end(),
                  [&](const Order& order) { return order.username == username; }),
        orders.end()
        );
}

// ------------------ UserPanel ------------------

UserPanel::UserPanel(const string& user, InventoryManager& inv, OrderManager& ordMgr)
    : username(user), inventory(inv), orderManager(ordMgr) {}

vector<InventoryItem> UserPanel::getAvailableItems() const {
    return inventory.getAllItems();
}

string UserPanel::getHelpText() const {
    return "Help: Contact support for ordering assistance.";
}

string UserPanel::getSupportInfo() const {
    return "Support: support@example.com";
}

void UserPanel::run() {
    // Optional CLI interface
}

// ------------------ InventoryItem ------------------

void InventoryItem::display() const {
    cout << "ID: " << id
         << ", Name: " << name
         << ", Quantity: " << quantity
         << ", Price: " << price
         << ", Category: " << category << endl;
}
