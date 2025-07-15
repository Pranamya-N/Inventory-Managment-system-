#include "user_page.h"
#include "ui_user_page.h"

#include "backend_classes.h"

#include <QMessageBox>
#include <QInputDialog>
#include <QTableWidgetItem>
#include <QString>
#include <QDebug>
#include <QHeaderView>
#include <QPushButton>
#include <algorithm>
#include "settingspage.h"

User_page::User_page(const QString& user, InventoryManager& inv, OrderManager& orders, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::User_page)
    , username(user)
    , inventory(inv)
    , orderManager(orders)
    , selectedItemId(-1)
{
    ui->setupUi(this);

    tabIndexToCategory = {
        {0, "Groceries"},
        {1, "Electronics"},
        {2, "Liquors"},
        {3, "Smoking Products"},
        {4, "Cosmetics"},
        {5, "Clothes"},
        {6, "Furniture"}
    };

    for (int i = 0; i < ui->categoryTabs->count(); ++i) {
        QTableWidget* table = getTableForCategory(tabIndexToCategory[i]);
        if (!table) continue;

        table->setColumnCount(5);
        table->setHorizontalHeaderLabels({"ID", "Name", "Available Stock", "Price", "Buy"});
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->setAlternatingRowColors(true);

        table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
        table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);

        connect(table, &QTableWidget::cellClicked, this, [this, table](int row, int) {
            QTableWidgetItem* idItem = table->item(row, 0);
            if (idItem) {
                selectedItemId = idItem->text().toInt();
                ui->quantityInput->setValue(1);
            }
        });
    }

    loadItemsForCategory(tabIndexToCategory[0]);

    connect(ui->categoryTabs, &QTabWidget::currentChanged, this, &User_page::on_categoryTabs_currentChanged);
    connect(ui->increaseQtyBtn, &QPushButton::clicked, this, &User_page::increaseQTY);
    connect(ui->decreaseQtyBtn, &QPushButton::clicked, this, &User_page::decreaseQTY);
    connect(ui->logoutBtn, &QPushButton::clicked, this, &User_page::on_logoutBtn_clicked);
    connect(ui->searchItemBtn, &QPushButton::clicked, this, &User_page::on_searchItemBtn_clicked);
    connect(ui->refreshBtn, &QPushButton::clicked, this, &User_page::on_refreshBtn_clicked);
}

User_page::~User_page()
{
    delete ui;
}

QTableWidget* User_page::getTableForCategory(const QString& category)
{
    if (category == "Furniture") return ui->furnitureTable;
    if (category == "Electronics") return ui->electronicsTable;
    if (category == "Groceries") return ui->groceriesTable;
    if (category == "Cosmetics") return ui->cosmeticsTable;
    if (category == "Clothes") return ui->clothesTable;
    if (category == "Liquors") return ui->liquorsTable;
    if (category == "Smoking Products") return ui->smokingTable;
    return nullptr;
}

void User_page::loadItemsForCategory(const QString& category)
{
    QTableWidget* table = getTableForCategory(category);
    if (!table) return;

    table->clearContents();
    table->setRowCount(0);

    QVector<InventoryItem> filteredItems;
    for (const auto& item : inventory.getAllItems()) {
        if (QString::fromStdString(item.category).compare(category, Qt::CaseInsensitive) == 0) {
            filteredItems.push_back(item);
        }
    }

    table->setRowCount(filteredItems.size());

    for (int row = 0; row < filteredItems.size(); ++row) {
        const auto& item = filteredItems[row];
        table->setItem(row, 0, new QTableWidgetItem(QString::number(item.id)));
        table->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(item.name)));
        table->setItem(row, 2, new QTableWidgetItem(QString::number(item.quantity)));
        table->setItem(row, 3, new QTableWidgetItem(QString::number(item.price, 'f', 2)));

        QPushButton* buyBtn = new QPushButton("Buy");
        buyBtn->setProperty("itemId", item.id);
        buyBtn->setProperty("itemName", QString::fromStdString(item.name));
        buyBtn->setProperty("category", category);

        connect(buyBtn, &QPushButton::clicked, this, [this, buyBtn]() {
            int itemId = buyBtn->property("itemId").toInt();
            QString category = buyBtn->property("category").toString();

            bool ok;
            int qty = QInputDialog::getInt(this, "Quantity", "Enter quantity:", 1, 1, 100, 1, &ok);
            if (!ok) return;

            auto allItems = inventory.getAllItems();
            auto it = std::find_if(allItems.begin(), allItems.end(), [=](const InventoryItem& i) {
                return i.id == itemId;
            });

            if (it == allItems.end()) {
                QMessageBox::warning(this, "Error", "Item not found");
                return;
            }

            if (it->quantity < qty) {
                QMessageBox::warning(this, "Insufficient Stock", "Not enough stock available.");
                return;
            }

            if (orderManager.placeOrder(username.toStdString(), inventory, itemId, qty)) {
                QMessageBox::information(this, "Added to Cart", QString("Added %1 x %2 to your cart.")
                                             .arg(qty)
                                             .arg(buyBtn->property("itemName").toString()));
                loadItemsForCategory(category);
            } else {
                QMessageBox::warning(this, "Failed", "Could not add item to cart.");
            }
        });

        table->setCellWidget(row, 4, buyBtn);
    }
}

void User_page::on_categoryTabs_currentChanged(int index)
{
    if (tabIndexToCategory.contains(index)) {
        loadItemsForCategory(tabIndexToCategory[index]);
        selectedItemId = -1;
        ui->quantityInput->setValue(1);
    }
}

void User_page::on_addToCartBtn_clicked()
{
    int qty = ui->quantityInput->value();

    if (selectedItemId == -1) {
        QMessageBox::warning(this, "Selection Required", "Please click an item to add.");
        return;
    }

    auto allItems = inventory.getAllItems();
    auto it = std::find_if(allItems.begin(), allItems.end(), [=](const InventoryItem& i) {
        return i.id == selectedItemId;
    });

    if (it == allItems.end()) {
        QMessageBox::warning(this, "Error", "Item not found.");
        return;
    }

    if (it->quantity < qty) {
        QMessageBox::warning(this, "Insufficient Stock", "Not enough stock available.");
        return;
    }

    QString category = QString::fromStdString(it->category);

    if (orderManager.placeOrder(username.toStdString(), inventory, selectedItemId, qty)) {
        QMessageBox::information(this, "Added to Cart", QString("Added %1 x %2 to your cart.")
                                     .arg(qty)
                                     .arg(QString::fromStdString(it->name)));
        loadItemsForCategory(category);
    } else {
        QMessageBox::warning(this, "Failed", "Could not add item to cart.");
    }
}

void User_page::on_checkoutBtn_clicked()
{
    auto orders = orderManager.getUserOrders(username.toStdString());
    if (orders.empty()) {
        QMessageBox::information(this, "No Orders", "You have no orders to checkout.");
        return;
    }

    emit checkoutRequested();  //
}

void User_page::on_feedbackBtn_clicked()
{
    bool ok;
    QString feedback = QInputDialog::getMultiLineText(this, "User Feedback", "Enter your feedback:", "", &ok);
    if (ok && !feedback.trimmed().isEmpty()) {
        QMessageBox::information(this, "Thank You", "Your feedback has been submitted.");
    }
}

int User_page::getTotalPurchases() const
{
    int total = 0;
    auto orders = orderManager.getUserOrders(username.toStdString());
    for (const auto& order : orders) {
        total += order.quantity;
    }
    return total;
}
void User_page::showSettingsDialog()
{
    QString password = "Enter the password"; // 🔸 Replace this with actual user's password lookup logic if needed
    QVector<QPair<QString, double>> history;

    auto orders = orderManager.getUserOrders(username.toStdString());
    for (const auto& order : orders) {
        QString name = QString::fromStdString(inventory.getItemName(order.itemId));
        double price = inventory.getPrice(order.itemId);  // ✅ Corrected method name
        history.append(qMakePair(name, price * order.quantity));
    }

    SettingsPage settingsDialog(username, password, history, this);

    connect(&settingsDialog, &SettingsPage::changePasswordRequested, this, [=](const QString& oldPass, const QString& newPass) {
        if (oldPass != password) {
            QMessageBox::warning(this, "Wrong Password", "The current password is incorrect.");
            return;
        }

        // TODO: Save the new password to UserDataStore if needed
        QMessageBox::information(this, "Success", "Password changed successfully.");
    });

    settingsDialog.exec();
}


void User_page::on_settingsBtn_clicked()
{
    showSettingsDialog();
}

void User_page::on_viewCartBtn_clicked()
{
    auto orders = orderManager.getUserOrders(username.toStdString());
    if (orders.empty()) {
        QMessageBox::information(this, "Cart Empty", "Your cart is currently empty.");
        return;
    }

    QString summary = "Cart Summary:\n";
    for (const auto& order : orders) {
        QString itemName = QString::fromStdString(inventory.getItemName(order.itemId));
        summary += QString("Item: %1 | Quantity: %2 | Status: %3\n")
                       .arg(itemName)
                       .arg(order.quantity)
                       .arg(QString::fromStdString(order.status));
    }

    QMessageBox::information(this, "View Cart", summary);
}

void User_page::increaseQTY()
{
    int val = ui->quantityInput->value();
    if(val < 100)
        ui->quantityInput->setValue(val + 1);
}

void User_page::decreaseQTY()
{
    int val = ui->quantityInput->value();
    if (val > 1)
        ui->quantityInput->setValue(val - 1);
}

void User_page::on_logoutBtn_clicked()
{
    emit logoutRequested();
}

// ----- SEARCH FUNCTION -----
void User_page::on_searchItemBtn_clicked()
{
    QString searchText = ui->searchItemInput->text().trimmed();

    // Empty search resets tabs and shows current tab normally
    if (searchText.isEmpty()) {
        for (int i = 0; i < ui->categoryTabs->count(); ++i) {
            ui->categoryTabs->setTabVisible(i, true);
        }
        int currentIndex = ui->categoryTabs->currentIndex();
        if (tabIndexToCategory.contains(currentIndex)) {
            loadItemsForCategory(tabIndexToCategory[currentIndex]);
            selectedItemId = -1;
            ui->quantityInput->setValue(1);
        }
        return;
    }

    // Hide all tabs and clear tables
    for (int i = 0; i < ui->categoryTabs->count(); ++i) {
        QTableWidget* table = getTableForCategory(tabIndexToCategory[i]);
        if (table) {
            table->clearContents();
            table->setRowCount(0);
        }
        ui->categoryTabs->setTabVisible(i, false);
    }

    // Map category to matched items
    QMap<QString, QVector<InventoryItem>> matchedByCategory;

    for (const auto& item : inventory.getAllItems()) {
        QString itemName = QString::fromStdString(item.name);
        if (itemName.contains(searchText, Qt::CaseInsensitive)) {
            QString category = QString::fromStdString(item.category);
            matchedByCategory[category].push_back(item);
        }
    }

    // Show tabs with matches and populate their tables
    for (auto it = matchedByCategory.constBegin(); it != matchedByCategory.constEnd(); ++it) {
        QString category = it.key();
        const QVector<InventoryItem>& items = it.value();

        int tabIndex = tabIndexToCategory.key(category, -1);
        if (tabIndex == -1) continue;

        ui->categoryTabs->setTabVisible(tabIndex, true);

        QTableWidget* table = getTableForCategory(category);
        if (!table) continue;

        table->setRowCount(items.size());

        for (int row = 0; row < items.size(); ++row) {
            const auto& item = items[row];
            table->setItem(row, 0, new QTableWidgetItem(QString::number(item.id)));
            table->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(item.name)));
            table->setItem(row, 2, new QTableWidgetItem(QString::number(item.quantity)));
            table->setItem(row, 3, new QTableWidgetItem(QString::number(item.price, 'f', 2)));

            QPushButton* buyBtn = new QPushButton("Buy");
            buyBtn->setProperty("itemId", item.id);
            buyBtn->setProperty("itemName", QString::fromStdString(item.name));
            buyBtn->setProperty("category", category);

            connect(buyBtn, &QPushButton::clicked, this, [this, buyBtn]() {
                int itemId = buyBtn->property("itemId").toInt();
                QString category = buyBtn->property("category").toString();

                bool ok;
                int qty = QInputDialog::getInt(this, "Quantity", "Enter quantity:", 1, 1, 100, 1, &ok);
                if (!ok) return;

                auto allItems = inventory.getAllItems();
                auto it = std::find_if(allItems.begin(), allItems.end(), [=](const InventoryItem& i) {
                    return i.id == itemId;
                });

                if (it == allItems.end()) {
                    QMessageBox::warning(this, "Error", "Item not found");
                    return;
                }

                if (it->quantity < qty) {
                    QMessageBox::warning(this, "Insufficient Stock", "Not enough stock available.");
                    return;
                }

                if (orderManager.placeOrder(username.toStdString(), inventory, itemId, qty)) {
                    QMessageBox::information(this, "Added to Cart", QString("Added %1 x %2 to your cart.")
                                                 .arg(qty)
                                                 .arg(buyBtn->property("itemName").toString()));
                    loadItemsForCategory(category);
                } else {
                    QMessageBox::warning(this, "Failed", "Could not add item to cart.");
                }
            });

            table->setCellWidget(row, 4, buyBtn);
        }
    }
}

// ----- REFRESH FUNCTION -----
// Resets search box, shows all tabs, reloads current tab normally
void User_page::on_refreshBtn_clicked()
{
    ui->searchItemInput->clear();

    for (int i = 0; i < ui->categoryTabs->count(); ++i) {
        ui->categoryTabs->setTabVisible(i, true);
    }

    int currentIndex = ui->categoryTabs->currentIndex();
    if (tabIndexToCategory.contains(currentIndex)) {
        loadItemsForCategory(tabIndexToCategory[currentIndex]);
    }

    selectedItemId = -1;
    ui->quantityInput->setValue(1);
}
