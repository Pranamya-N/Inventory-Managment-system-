#ifndef BACKEND_CLASSES_H
#define BACKEND_CLASSES_H

#include <string>
#include <vector>
#include <iostream>

using namespace std;

// ------------------ UserRecord ------------------
struct UserRecord {
    string username;
    string password;
    string role;
    string status;
};

// ------------------ InventoryItem ------------------
struct InventoryItem {
    int id;
    string name;
    int quantity;
    double price;
    string category;

    void display() const;
};

// ------------------ UserDataStore ------------------
class UserDataStore {
    vector<UserRecord> users;

public:
    void addUser(const string& username, const string& password, const string& role);
    bool isValidUser(const string& username, const string& password);
    string getUserRole(const string& username);
    bool userExists(const string& username);
    const vector<UserRecord>& getAllUsers() const;
    bool deleteUser(const string& username);
};

// ------------------ InventoryManager ------------------
class InventoryManager {
    vector<InventoryItem> items;

public:
    void addItem(const InventoryItem& item);
    void showItems() const;
    bool getItem(int id, InventoryItem& resultItem) const;
    bool removeItemById(int id);
    void updateItemQuantity(int id, int newQuantity);
    const vector<InventoryItem>& getAllItems() const;
    double getPrice(int id) const;
    string getItemName(int id) const;
    bool hasItem(int id) const;
};

// ------------------ Order ------------------
struct Order {
    int orderId;
    string username;
    int itemId;
    int quantity;
    string status;
};

// ------------------ OrderManager ------------------
class OrderManager {
    vector<Order> orders;
    int nextOrderId;

public:
    OrderManager();
    bool placeOrder(const string& username, InventoryManager& inventory, int itemId, int quantity);
    vector<Order> getUserOrders(const string& username) const;
    bool removeOrder(const string& username, int itemId);
    bool decreaseOrderQuantity(const string& username, int itemId);

    // Added method to clear all orders for a user
    void clearUserOrders(const string& username);
};

// ------------------ UserPanel ------------------
class UserPanel {
    string username;
    InventoryManager& inventory;
    OrderManager& orderManager;

public:
    UserPanel(const string& user, InventoryManager& inv, OrderManager& ordMgr);
    vector<InventoryItem> getAvailableItems() const;
    string getHelpText() const;
    string getSupportInfo() const;
    void run();
};

#endif // BACKEND_CLASSES_H
