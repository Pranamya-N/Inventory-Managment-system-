#ifndef ADMIN_PAGE_H
#define ADMIN_PAGE_H

#include <QWidget>
#include <QTableWidget>
#include "backend_classes.h"

namespace Ui {
class Admin_page;
}

class Admin_page : public QWidget
{
    Q_OBJECT

public:
    explicit Admin_page(InventoryManager& inventoryRef, UserDataStore& userDataRef, QWidget *parent = nullptr);
    ~Admin_page();

signals:
    void logoutRequested();

private slots:
    // Inventory tab slots
    void on_addItemBtn_clicked();
    void on_removeItemBtn_clicked();
    void on_searchItemBtn_clicked();

    // Users tab slots
    void on_deleteUserBtn_clicked();
    void on_searchUserBtn_clicked();

    // Shared slots
    void on_refreshBtn_clicked();
    void on_logoutBtn_clicked();

private:
    Ui::Admin_page *ui;

    InventoryManager& inventory;
    UserDataStore& userData;

    void loadItemsToTable(const QString &category, QTableWidget *table, const QString &search = "");
    void refreshAllItemTables(const QString &search = "");
    void loadUsersToTable(const QString &search = "");
    void refreshUsersTable();

    // Add a helper method to get table pointer by category (optional but recommended)
    QTableWidget* getTableForCategory(const QString& category);
};

#endif // ADMIN_PAGE_H
