#include "database_manager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

bool DatabaseManager::connectToDatabase(const QString& path)
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);

    if (!db.open()) {
        qDebug() << "Database connection failed:" << db.lastError().text();
        return false;
    }

    qDebug() << "Connected to database successfully!";
    return true;
}

void DatabaseManager::setupTables()
{
    QSqlQuery query;

    // Enable foreign key support
    query.exec("PRAGMA foreign_keys = ON");

    // 1. Users table
    query.exec(R"(
        CREATE TABLE IF NOT EXISTS users (
            username TEXT PRIMARY KEY,
            password TEXT,
            role TEXT,
            status TEXT
        )
    )");

    // 2. Inventory table
    query.exec(R"(
        CREATE TABLE IF NOT EXISTS inventory (
            id INTEGER PRIMARY KEY,
            name TEXT,
            quantity INTEGER,
            price REAL,
            category TEXT
        )
    )");

    // 3. Cart table
    query.exec(R"(
        CREATE TABLE IF NOT EXISTS cart (
            username TEXT NOT NULL,
            itemId INTEGER NOT NULL,
            quantity INTEGER NOT NULL DEFAULT 1,
            PRIMARY KEY (username, itemId),
            FOREIGN KEY (itemId) REFERENCES inventory(id) ON DELETE CASCADE
        )
    )");

    // 4. Orders table (create basic structure if not exists)
    query.exec(R"(
        CREATE TABLE IF NOT EXISTS orders (
            orderId INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT NOT NULL,
            itemId INTEGER NOT NULL,
            quantity INTEGER NOT NULL,
            status TEXT NOT NULL DEFAULT 'completed',
            order_date TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (itemId) REFERENCES inventory(id) ON DELETE CASCADE
        )
    )");

    // 5. Check for missing 'order_date' column and recreate table if necessary
    query.exec("PRAGMA table_info(orders)");
    bool hasOrderDate = false;
    while (query.next()) {
        QString colName = query.value(1).toString();
        if (colName == "order_date") {
            hasOrderDate = true;
            break;
        }
    }

    if (!hasOrderDate) {
        qDebug() << "Recreating orders table to add 'order_date' with CURRENT_TIMESTAMP default";

        QSqlQuery recreate;
        recreate.exec("ALTER TABLE orders RENAME TO orders_old");

        recreate.exec(R"(
            CREATE TABLE orders (
                orderId INTEGER PRIMARY KEY AUTOINCREMENT,
                username TEXT NOT NULL,
                itemId INTEGER NOT NULL,
                quantity INTEGER NOT NULL,
                status TEXT NOT NULL DEFAULT 'completed',
                order_date TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (itemId) REFERENCES inventory(id) ON DELETE CASCADE
            )
        )");

        recreate.exec(R"(
            INSERT INTO orders (orderId, username, itemId, quantity, status)
            SELECT orderId, username, itemId, quantity, status FROM orders_old
        )");

        recreate.exec("DROP TABLE orders_old");

        qDebug() << "Orders table recreated with 'order_date'";
    }

    qDebug() << "Database tables initialized successfully";
}
