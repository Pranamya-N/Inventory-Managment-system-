#ifndef ADMIN_PAGE_H
#define ADMIN_PAGE_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>      // For QLineEdit usage
#include <QPushButton>    // For QPushButton usage
#include "backend_classes.h"

namespace Ui {
class Admin_page;
}

class Admin_page : public QWidget
{
    Q_OBJECT

public:
    explicit Admin_page(UserDataStore& userDataRef, InventoryManager& inventoryRef, QWidget* parent = nullptr);
    ~Admin_page();

signals:
    void logoutRequested();

private slots:
    // User tab
    void onSearchUser();
    void onDeleteUser();

    // Inventory tab
    void onAddItem();
    void onRemoveItem();
    void onSearchItem();
    void onUpdateItem();
    void onAddDescription();  // <-- New slot added here

    // Shared
    void refreshAllTables();

private:
    Ui::Admin_page *ui;

    UserDataStore& userDataStore;
    InventoryManager& inventoryManager;

    // Helper functions
    void setupUserTable();
    void setupInventoryTables();

    void populateUserTable();
    void populateInventoryTables();

    void clearAddItemInputs();
    void clearUpdateInputs();

    // Optional: get inventory table widget pointer by category string
    QTableWidget* getTableForCategory(const QString& category);
};

#endif // ADMIN_PAGE_H
