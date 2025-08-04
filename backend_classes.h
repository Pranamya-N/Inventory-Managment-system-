

#ifndef BACKEND_CLASSES_H
#define BACKEND_CLASSES_H

#include <string>
#include <vector>
#include <QString>

// Forward declarations
class InventoryManager;

struct UserRecord {
    std::string username;
    std::string password;
    std::string role;
    std::string status;
};

struct InventoryItem {
    int id;
    std::string name;
    int quantity;
    double price;
    std::string category;

    void display() const;
};

struct Order {
    int orderId;
    int itemId;
    int quantity;
    std::string status;
    std::string order_date;
};

struct CartItem {
    int itemId;
    std::string itemName;
    int quantity;
    double price;
};

struct PurchaseRecord {
    QString productName;
    int quantity;
    double amount;
};

class UserDataStore {
public:
    void addUser(const std::string& username, const std::string& password, const std::string& role);
    bool isValidUser(const std::string& username, const std::string& password);
    std::string getUserRole(const std::string& username);
    bool userExists(const std::string& username);
    const std::vector<UserRecord>& getAllUsers() const;
    bool deleteUser(const std::string& username);
};

class InventoryManager {
public:
    void addItem(const InventoryItem& item);
    void showItems() const;
    bool getItem(int id, InventoryItem& resultItem) const;
    bool removeItemById(int id);
    void updateItemQuantity(int id, int newQuantity);
    void setItemQuantity(int id, int newQuantity);
    int getItemQuantity(int id) const;
    const std::vector<InventoryItem>& getAllItems() const;
    double getPrice(int id) const;
    std::string getItemName(int id) const;
    bool hasItem(int id) const;
    bool decreaseQuantity(int itemId, int amount);

};

class OrderManager {
public:
    OrderManager();

    // Cart management
    bool addToCart(const std::string& username, int itemId, int quantity);
    std::vector<CartItem> getCartItems(const std::string& username) const;
    bool removeFromCart(const std::string& username, int itemId);

    // Order processing (two versions)
    bool placeOrder(const std::string& username, InventoryManager& inventory);  // For entire cart
    bool placeOrder(const std::string& username, InventoryManager& inventory, int itemId, int quantity);  // For single item

    // Order management
    std::vector<Order> getCompletedOrders(const std::string& username) const;
    std::vector<Order> getUserOrders(const std::string& username) const;
    bool removeOrder(const std::string& username, int itemId);
    bool decreaseOrderQuantity(const std::string& username, int itemId);
    void clearUserOrders(const std::string& username);
    bool finalizeOrder(const std::string& username, InventoryManager& inventory);
};

class UserPanel {
public:
    UserPanel(const std::string& user, InventoryManager& inv, OrderManager& ordMgr);
    std::vector<InventoryItem> getAvailableItems() const;
    std::string getHelpText() const;
    std::string getSupportInfo() const;
    void run();

private:
    std::string username;
    InventoryManager& inventory;
    OrderManager& orderManager;
};

#endif // BACKEND_CLASSES_H
