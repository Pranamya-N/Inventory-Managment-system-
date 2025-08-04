#ifndef USER_PAGE_H
#define USER_PAGE_H

#include <QWidget>
#include <QMap>
#include <QString>
#include <QTableWidget>
#include "backend_classes.h"

namespace Ui {
class User_page;
}

class User_page : public QWidget
{
    Q_OBJECT

public:
    explicit User_page(const QString& username, InventoryManager& inventory, OrderManager& orders, QWidget *parent = nullptr);
    QString getUsername() const { return username; }
    ~User_page();

private slots:
    void on_addToCartBtn_clicked();
    void on_checkoutBtn_clicked();
    void on_feedbackBtn_clicked();
    void on_settingsBtn_clicked();
    void on_categoryTabs_currentChanged(int index);
    void on_viewCartBtn_clicked();
    void increaseQTY();
    void decreaseQTY();
    void on_logoutBtn_clicked();

    void on_searchItemBtn_clicked();  // Slot for Search button clicked
    void on_refreshBtn_clicked();     // Slot for Refresh button clicked

private:
    Ui::User_page *ui;
    QString username;
    InventoryManager& inventory;
    OrderManager& orderManager;
    QMap<int, QString> tabIndexToCategory;
    QMap<int, QString> itemIdToCategory;
    int selectedItemId = -1;

    void loadItemsForCategory(const QString& category);
    QTableWidget* getTableForCategory(const QString& category) const; // <-- const added here
    void showSettingsDialog();
    int getTotalPurchases() const;

    // New helper to setup table columns with Description included
    void setupTableColumns(QTableWidget* table);

signals:
    void logoutRequested();
    void checkoutRequested();
};

#endif // USER_PAGE_H
