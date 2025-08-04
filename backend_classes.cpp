#include "backend_classes.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include <iostream>

using namespace std;

// ------------------ UserDataStore ------------------

void UserDataStore::addUser(const string& username, const string& password, const string& role)
{
    QSqlQuery q;
    q.prepare("INSERT INTO users (username, password, role, status) VALUES (?, ?, ?, ?)");
    q.addBindValue(QString::fromStdString(username));
    q.addBindValue(QString::fromStdString(password));
    q.addBindValue(QString::fromStdString(role));
    q.addBindValue("active");
    if (!q.exec())
        qDebug() << "addUser failed:" << q.lastError().text();
}

bool UserDataStore::isValidUser(const string& username, const string& password)
{
    QSqlQuery q;
    q.prepare("SELECT 1 FROM users WHERE username = ? AND password = ?");
    q.addBindValue(QString::fromStdString(username));
    q.addBindValue(QString::fromStdString(password));
    if (!q.exec())
    {
        qDebug() << "isValidUser query failed:" << q.lastError().text();
        return false;
    }
    return q.next();
}

string UserDataStore::getUserRole(const string& username)
{
    QSqlQuery q;
    q.prepare("SELECT role FROM users WHERE username = ?");
    q.addBindValue(QString::fromStdString(username));
    if (!q.exec())
    {
        qDebug() << "getUserRole query failed:" << q.lastError().text();
        return "";
    }
    if (q.next())
        return q.value(0).toString().toStdString();
    return "";
}

bool UserDataStore::userExists(const string& username)
{
    QSqlQuery q;
    q.prepare("SELECT 1 FROM users WHERE username = ?");
    q.addBindValue(QString::fromStdString(username));
    if (!q.exec())
    {
        qDebug() << "userExists query failed:" << q.lastError().text();
        return false;
    }
    return q.next();
}

const vector<UserRecord>& UserDataStore::getAllUsers() const
{
    static vector<UserRecord> cache;
    cache.clear();

    QSqlQuery q("SELECT username, password, role, status FROM users");
    while (q.next())
    {
        UserRecord u;
        u.username = q.value(0).toString().toStdString();
        u.password = q.value(1).toString().toStdString();
        u.role = q.value(2).toString().toStdString();
        u.status = q.value(3).toString().toStdString();
        cache.push_back(u);
    }
    return cache;
}

bool UserDataStore::deleteUser(const string& username)
{
    QSqlQuery q;
    q.prepare("DELETE FROM users WHERE username = ? AND role != 'admin'");
    q.addBindValue(QString::fromStdString(username));
    if (!q.exec())
    {
        qDebug() << "deleteUser failed:" << q.lastError().text();
        return false;
    }
    return q.numRowsAffected() > 0;
}

// ------------------ InventoryManager ------------------

void InventoryManager::addItem(const InventoryItem& item) {
    QSqlQuery query;
    query.prepare("INSERT OR REPLACE INTO inventory (id, name, quantity, price, category, description) "
                  "VALUES (:id, :name, :quantity, :price, :category, :description)");
    query.bindValue(":id", item.id);
    query.bindValue(":name", QString::fromStdString(item.name));
    query.bindValue(":quantity", item.quantity);
    query.bindValue(":price", item.price);
    query.bindValue(":category", QString::fromStdString(item.category));
    query.bindValue(":description", QString::fromStdString(item.description)); // ✅ MUST include this line

    if (!query.exec()) {
        qDebug() << "AddItem Error:" << query.lastError().text();
    }
}


void InventoryManager::showItems() const
{
    QSqlQuery q("SELECT id, name, quantity, price, category FROM inventory");
    while (q.next())
    {
        InventoryItem item;
        item.id = q.value(0).toInt();
        item.name = q.value(1).toString().toStdString();
        item.quantity = q.value(2).toInt();
        item.price = q.value(3).toDouble();
        item.category = q.value(4).toString().toStdString();
        item.display();
    }
}

bool InventoryManager::getItem(int id, InventoryItem& resultItem) const
{
    QSqlQuery q;
    q.prepare("SELECT id, name, quantity, price, category, description FROM inventory WHERE id = ?");
    q.addBindValue(id);
    if (!q.exec())
    {
        qDebug() << "getItem query failed:" << q.lastError().text();
        return false;
    }
    if (q.next())
    {
        resultItem.id = q.value(0).toInt();
        resultItem.name = q.value(1).toString().toStdString();
        resultItem.quantity = q.value(2).toInt();
        resultItem.price = q.value(3).toDouble();
        resultItem.category = q.value(4).toString().toStdString();
        resultItem.description = q.value(5).toString().toStdString();
        return true;
    }
    return false;
}

bool InventoryManager::removeItemById(int id)
{
    QSqlQuery q;
    q.prepare("DELETE FROM inventory WHERE id = ?");
    q.addBindValue(id);
    if (!q.exec())
    {
        qDebug() << "removeItemById failed:" << q.lastError().text();
        return false;
    }
    return q.numRowsAffected() > 0;
}

void InventoryManager::updateItemQuantity(int id, int newQuantity)
{
    QSqlQuery q;
    q.prepare("UPDATE inventory SET quantity = ? WHERE id = ?");
    q.addBindValue(newQuantity);
    q.addBindValue(id);
    if (!q.exec())
        qDebug() << "updateItemQuantity failed:" << q.lastError().text();
}

bool InventoryManager::updateItemPriceAndQuantity(int id, double priceIncrease, int quantityIncrease) {
    InventoryItem item;
    if (!getItem(id, item)) {
        qDebug() << "updateItemPriceAndQuantity: Item not found, id=" << id;
        return false;
    }

    double newPrice = item.price + priceIncrease;
    int newQuantity = item.quantity + quantityIncrease;

    QSqlQuery q;
    q.prepare("UPDATE inventory SET price = ?, quantity = ? WHERE id = ?");
    q.addBindValue(newPrice);
    q.addBindValue(newQuantity);
    q.addBindValue(id);

    if (!q.exec()) {
        qDebug() << "updateItemPriceAndQuantity failed:" << q.lastError().text();
        return false;
    }
    return true;
}

void InventoryManager::setItemQuantity(int id, int newQuantity)
{
    QSqlQuery q;
    q.prepare("UPDATE inventory SET quantity = ? WHERE id = ?");
    q.addBindValue(newQuantity);
    q.addBindValue(id);
    if (!q.exec())
        qDebug() << "setItemQuantity failed:" << q.lastError().text();
}

int InventoryManager::getItemQuantity(int id) const
{
    InventoryItem item;
    if (getItem(id, item)) {
        return item.quantity;
    }
    return 0;
}

const vector<InventoryItem>& InventoryManager::getAllItems() const
{
    static vector<InventoryItem> cache;
    cache.clear();

    QSqlQuery q("SELECT id, name, quantity, price, category, description FROM inventory");
    while (q.next())
    {
        InventoryItem item;
        item.id = q.value(0).toInt();
        item.name = q.value(1).toString().toStdString();
        item.quantity = q.value(2).toInt();
        item.price = q.value(3).toDouble();
        item.category = q.value(4).toString().toStdString();
        item.description = q.value(5).toString().toStdString();
        cache.push_back(item);
    }
    return cache;
}


double InventoryManager::getPrice(int id) const
{
    QSqlQuery q;
    q.prepare("SELECT price FROM inventory WHERE id = ?");
    q.addBindValue(id);
    if (!q.exec())
    {
        qDebug() << "getPrice failed:" << q.lastError().text();
        return 0.0;
    }
    if (q.next())
        return q.value(0).toDouble();
    return 0.0;
}

string InventoryManager::getItemName(int id) const
{
    QSqlQuery q;
    q.prepare("SELECT name FROM inventory WHERE id = ?");
    q.addBindValue(id);
    if (!q.exec())
    {
        qDebug() << "getItemName failed:" << q.lastError().text();
        return "";
    }
    if (q.next())
        return q.value(0).toString().toStdString();
    return "";
}

bool InventoryManager::hasItem(int id) const
{
    QSqlQuery q;
    q.prepare("SELECT 1 FROM inventory WHERE id = ?");
    q.addBindValue(id);
    if (!q.exec())
    {
        qDebug() << "hasItem failed:" << q.lastError().text();
        return false;
    }
    return q.next();
}

// FIXED: Missing function that caused linker error
bool InventoryManager::decreaseQuantity(int itemId, int amount)
{
    int currentQuantity = getItemQuantity(itemId);
    if (currentQuantity < amount) {
        qDebug() << "Insufficient stock for item" << itemId << ": need" << amount << "have" << currentQuantity;
        return false; // not enough stock
    }

    int newQuantity = currentQuantity - amount;

    QSqlQuery q;
    q.prepare("UPDATE inventory SET quantity = ? WHERE id = ?");
    q.addBindValue(newQuantity);
    q.addBindValue(itemId);
    if (!q.exec())
    {
        qDebug() << "decreaseQuantity failed:" << q.lastError().text();
        return false;
    }

    qDebug() << "Successfully decreased quantity for item" << itemId << "from" << currentQuantity << "to" << newQuantity;
    return true;
}

bool InventoryManager::updateItemDescription(int id, const std::string& newDescription)
{
    QSqlQuery query;
    query.prepare("UPDATE inventory SET description = ? WHERE id = ?");
    query.addBindValue(QString::fromStdString(newDescription));
    query.addBindValue(id);

    if (!query.exec())
    {
        qDebug() << "updateItemDescription failed:" << query.lastError().text();
        return false;
    }
    return true;
}

// ------------------ OrderManager ------------------

OrderManager::OrderManager() {}

// Add item(s) to cart
bool OrderManager::addToCart(const string& username, int itemId, int quantity)
{
    QSqlQuery q;
    q.prepare("INSERT INTO cart (username, itemId, quantity) VALUES (?, ?, ?) "
              "ON CONFLICT(username, itemId) DO UPDATE SET quantity = quantity + ?");
    q.addBindValue(QString::fromStdString(username));
    q.addBindValue(itemId);
    q.addBindValue(quantity);
    q.addBindValue(quantity);
    if (!q.exec())
    {
        qDebug() << "addToCart failed:" << q.lastError().text();
        return false;
    }
    return true;
}

// Get all items currently in user's cart
vector<CartItem> OrderManager::getCartItems(const string& username) const
{
    vector<CartItem> items;
    QSqlQuery q;
    q.prepare("SELECT c.itemId, i.name, c.quantity, i.price FROM cart c "
              "JOIN inventory i ON c.itemId = i.id WHERE c.username = ?");
    q.addBindValue(QString::fromStdString(username));
    if (q.exec())
    {
        while (q.next())
        {
            CartItem item;
            item.itemId = q.value(0).toInt();
            item.itemName = q.value(1).toString().toStdString();
            item.quantity = q.value(2).toInt();
            item.price = q.value(3).toDouble();
            items.push_back(item);
        }
    }
    else
    {
        qDebug() << "getCartItems failed:" << q.lastError().text();
    }
    return items;
}

// Remove item completely from cart
bool OrderManager::removeFromCart(const string& username, int itemId)
{
    QSqlQuery q;
    q.prepare("DELETE FROM cart WHERE username = ? AND itemId = ?");
    q.addBindValue(QString::fromStdString(username));
    q.addBindValue(itemId);
    if (!q.exec())
    {
        qDebug() << "removeFromCart failed:" << q.lastError().text();
        return false;
    }
    return q.numRowsAffected() > 0;
}

// Decrease item quantity in cart by one
bool OrderManager::decreaseOrderQuantity(const string& username, int itemId)
{
    // Get current quantity
    QSqlQuery q;
    q.prepare("SELECT quantity FROM cart WHERE username = ? AND itemId = ?");
    q.addBindValue(QString::fromStdString(username));
    q.addBindValue(itemId);

    if (!q.exec())
    {
        qDebug() << "decreaseOrderQuantity select failed:" << q.lastError().text();
        return false;
    }
    if (!q.next()) return false;

    int currentQty = q.value(0).toInt();
    if (currentQty <= 1)
    {
        // Remove item from cart
        return removeFromCart(username, itemId);
    }
    else
    {
        // Decrement quantity by 1
        QSqlQuery updateQ;
        updateQ.prepare("UPDATE cart SET quantity = quantity - 1 WHERE username = ? AND itemId = ?");
        updateQ.addBindValue(QString::fromStdString(username));
        updateQ.addBindValue(itemId);
        if (!updateQ.exec())
        {
            qDebug() << "decreaseOrderQuantity update failed:" << updateQ.lastError().text();
            return false;
        }
        return true;
    }
}

// FIXED: Place an order for all items currently in cart with improved error handling
bool OrderManager::placeOrder(const std::string& username, InventoryManager& inventory) {
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) return false;

    QSqlQuery query(db);
    db.transaction();

    auto items = getCartItems(username);

    if (items.empty()) {
        db.rollback();
        return false; // Cart empty
    }

    for (const auto& item : items) {
        // Insert each cart item as a separate order record
        query.prepare(R"(
            INSERT INTO orders (username, itemId, quantity, status, order_date)
            VALUES (?, ?, ?, 'completed', datetime('now'))
        )");
        query.addBindValue(QString::fromStdString(username));
        query.addBindValue(item.itemId);
        query.addBindValue(item.quantity);

        if (!query.exec()) {
            qDebug() << "Failed to insert order:" << query.lastError().text();
            db.rollback();
            return false;
        }

        // Decrease inventory stock
        if (!inventory.decreaseQuantity(item.itemId, item.quantity)) {
            db.rollback();
            return false;
        }
    }

    // Clear cart after all orders inserted
    query.prepare("DELETE FROM cart WHERE username = ?");
    query.addBindValue(QString::fromStdString(username));
    if (!query.exec()) {
        qDebug() << "Failed to clear cart:" << query.lastError().text();
        db.rollback();
        return false;
    }

    db.commit();
    return true;
}

bool OrderManager::placeOrder(const string& username, InventoryManager& inventory, int itemId, int quantity)
{
    QSqlDatabase::database().transaction();

    try
    {
        if (inventory.getItemQuantity(itemId) < quantity)
            throw runtime_error("Insufficient stock");

        QSqlQuery orderQuery;
        orderQuery.prepare("INSERT INTO orders (username, itemId, quantity, status, order_date) "
                           "VALUES (?, ?, ?, 'completed', datetime('now'))");

        // Deduct inventory
        if (!inventory.decreaseQuantity(itemId, quantity))
            throw runtime_error("Failed to decrease inventory");

        orderQuery.addBindValue(QString::fromStdString(username));
        orderQuery.addBindValue(itemId);
        orderQuery.addBindValue(quantity);
        if (!orderQuery.exec())
            throw runtime_error("Failed to create order record");

        QSqlDatabase::database().commit();
        return true;
    }
    catch (const exception& e)
    {
        QSqlDatabase::database().rollback();
        qDebug() << "placeOrder (single) failed:" << e.what();
        return false;
    }
}

// Get completed orders for user (purchase history)
vector<Order> OrderManager::getCompletedOrders(const string& username) const
{
    vector<Order> orders;
    QSqlQuery q;
    q.prepare("SELECT orderId, itemId, quantity, order_date FROM orders "
              "WHERE username = ? AND status = 'completed' ORDER BY order_date DESC");
    q.addBindValue(QString::fromStdString(username));

    if (q.exec())
    {
        while (q.next())
        {
            Order o;
            o.orderId = q.value(0).toInt();
            o.itemId = q.value(1).toInt();
            o.quantity = q.value(2).toInt();
            o.order_date = q.value(3).toString().toStdString();
            o.status = "completed";
            orders.push_back(o);
        }
    }
    else
    {
        qDebug() << "getCompletedOrders failed:" << q.lastError().text();
    }
    return orders;
}

// Get all orders (including non-completed)
vector<Order> OrderManager::getUserOrders(const string& username) const
{
    vector<Order> orders;
    QSqlQuery q;
    q.prepare("SELECT orderId, itemId, quantity, status, order_date FROM orders WHERE username = ?");
    q.addBindValue(QString::fromStdString(username));
    if (q.exec())
    {
        while (q.next())
        {
            Order o;
            o.orderId = q.value(0).toInt();
            o.itemId = q.value(1).toInt();
            o.quantity = q.value(2).toInt();
            o.status = q.value(3).toString().toStdString();
            o.order_date = q.value(4).toString().toStdString();
            orders.push_back(o);
        }
    }
    else
    {
        qDebug() << "getUserOrders failed:" << q.lastError().text();
    }
    return orders;
}

// Remove order by username and itemId (for cart)
bool OrderManager::removeOrder(const string& username, int itemId)
{
    QSqlQuery q;
    q.prepare("DELETE FROM orders WHERE username = ? AND itemId = ?");
    q.addBindValue(QString::fromStdString(username));
    q.addBindValue(itemId);
    if (!q.exec())
    {
        qDebug() << "removeOrder failed:" << q.lastError().text();
        return false;
    }
    return q.numRowsAffected() > 0;
}

// Clear all orders of a user
void OrderManager::clearUserOrders(const string& username)
{
    QSqlQuery q;
    q.prepare("DELETE FROM orders WHERE username = ?");
    q.addBindValue(QString::fromStdString(username));
    if (!q.exec())
        qDebug() << "clearUserOrders failed:" << q.lastError().text();
}

// Finalize order (for future use if needed)
bool OrderManager::finalizeOrder(const std::string& username, InventoryManager& inventory)
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qDebug() << "Database not open!";
        return false;
    }

    QSqlQuery query(db);
    db.transaction();

    auto cartItems = getUserOrders(username);
    if (cartItems.empty()) {
        qDebug() << "Cart is empty for user:" << QString::fromStdString(username);
        db.rollback();
        return false;
    }

    for (const auto& item : cartItems) {
        // Reduce inventory
        if (!inventory.decreaseQuantity(item.itemId, item.quantity)) {
            qDebug() << "Inventory decrease failed for item" << item.itemId;
            db.rollback();
            return false;
        }

        // Insert into orders table
        query.prepare(R"(
            INSERT INTO orders (username, itemId, quantity, status, order_date)
            VALUES (?, ?, ?, 'completed', CURRENT_TIMESTAMP)
        )");
        query.addBindValue(QString::fromStdString(username));
        query.addBindValue(item.itemId);
        query.addBindValue(item.quantity);

        if (!query.exec()) {
            qDebug() << "Failed to create order record:" << query.lastError().text();
            db.rollback();
            return false;
        }
    }

    // Clear from cart table
    QSqlQuery clearQuery(db);
    clearQuery.prepare("DELETE FROM cart WHERE username = ?");
    clearQuery.addBindValue(QString::fromStdString(username));

    if (!clearQuery.exec()) {
        qDebug() << "Failed to clear cart:" << clearQuery.lastError().text();
        db.rollback();
        return false;
    }

    db.commit();
    return true;
}

// ------------------ UserPanel ------------------

UserPanel::UserPanel(const string& user, InventoryManager& inv, OrderManager& ordMgr)
    : username(user), inventory(inv), orderManager(ordMgr) {}

vector<InventoryItem> UserPanel::getAvailableItems() const
{
    return inventory.getAllItems();
}

string UserPanel::getHelpText() const
{
    return "Help: Contact support for ordering assistance.";
}

string UserPanel::getSupportInfo() const
{
    return "Support: support@example.com";
}

void UserPanel::run()
{
    // CLI or GUI related logic
}

// ------------------ InventoryItem ------------------

void InventoryItem::display() const
{
    cout << "ID: " << id
         << ", Name: " << name
         << ", Quantity: " << quantity
         << ", Price: " << price
         << ", Category: " << category << endl;
}
