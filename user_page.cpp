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

        table->setColumnCount(6);
        table->setHorizontalHeaderLabels({"ID", "Name", "Available Stock", "Price", "Description", "Buy"});
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->setAlternatingRowColors(true);

        table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
        table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
        table->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);

        connect(table, &QTableWidget::cellClicked, this, [this, table](int row, int) {
            QTableWidgetItem* idItem = table->item(row, 0);
            if (idItem) {
                selectedItemId = idItem->text().toInt();
                ui->quantityInput->setValue(1);
            }
        });
    }

    loadItemsForCategory(tabIndexToCategory[0]);

    // Button Connections
    connect(ui->categoryTabs, &QTabWidget::currentChanged, this, &User_page::on_categoryTabs_currentChanged);
    connect(ui->increaseQtyBtn, &QPushButton::clicked, this, &User_page::increaseQTY);
    connect(ui->decreaseQtyBtn, &QPushButton::clicked, this, &User_page::decreaseQTY);
    connect(ui->logoutBtn, &QPushButton::clicked, this, &User_page::on_logoutBtn_clicked);
    connect(ui->searchItemBtn, &QPushButton::clicked, this, &User_page::on_searchItemBtn_clicked);
    connect(ui->refreshBtn, &QPushButton::clicked, this, &User_page::on_refreshBtn_clicked);
    connect(ui->settingsBtn, &QPushButton::clicked, this, &User_page::on_settingsBtn_clicked);
}

User_page::~User_page()
{
    delete ui;
}

QTableWidget* User_page::getTableForCategory(const QString& category) const
{
    if (category == "Groceries")
        return ui->groceriesTable;
    else if (category == "Electronics")
        return ui->electronicsTable;
    else if (category == "Liquors")
        return ui->liquorsTable;
    else if (category == "Smoking Products")
        return ui->smokingTable;
    else if (category == "Cosmetics")
        return ui->cosmeticsTable;
    else if (category == "Clothes")
        return ui->clothesTable;
    else if (category == "Furniture")
        return ui->furnitureTable;
    else
        return nullptr;
}

void User_page::loadItemsForCategory(const QString& category)
{
    QTableWidget* table = getTableForCategory(category);
    if (!table) return;

    table->disconnect();
    table->clearContents();
    table->setRowCount(0);
    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({"ID", "Name", "Available Stock", "Price", "Description", "Buy"});
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);

    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);

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

        // View Description Button
        QPushButton* viewDescBtn = new QPushButton("View");
        viewDescBtn->setFixedSize(80, 25);
        viewDescBtn->setStyleSheet("QPushButton { font-size: 10px; padding: 2px; }");
        connect(viewDescBtn, &QPushButton::clicked, this, [item]() {
            QMessageBox::information(nullptr, "Item Description", QString::fromStdString(item.description));
        });
        table->setCellWidget(row, 4, viewDescBtn);

        // Buy Button
        QPushButton* buyBtn = new QPushButton("Buy");
        buyBtn->setProperty("itemId", item.id);
        buyBtn->setProperty("itemName", QString::fromStdString(item.name));
        buyBtn->setProperty("category", category);
        connect(buyBtn, &QPushButton::clicked, this, [this, item, category]() {
            auto allItems = inventory.getAllItems();
            auto it = std::find_if(allItems.begin(), allItems.end(), [item](const InventoryItem& i) {
                return i.id == item.id;
            });

            if (it == allItems.end()) {
                QMessageBox::warning(this, "Error", "Item not found");
                return;
            }

            if (it->quantity <= 0) {
                QMessageBox::warning(this, "Out of Stock", "This item is currently out of stock.");
                return;
            }

            bool ok;
            int qty = QInputDialog::getInt(this, "Quantity",
                                           QString("Enter quantity (Max available: %1):").arg(it->quantity),
                                           1, 1, it->quantity, 1, &ok);
            if (!ok) return;

            if (orderManager.addToCart(username.toStdString(), item.id, qty)) {
                QMessageBox::information(this, "Added to Cart",
                                         QString("Added %1 x %2 to your cart.")
                                             .arg(qty)
                                             .arg(QString::fromStdString(item.name)));
                loadItemsForCategory(category);
            } else {
                QMessageBox::warning(this, "Failed", "Could not add item to cart.");
            }
        });

        table->setCellWidget(row, 5, buyBtn);
    }

    connect(table, &QTableWidget::cellClicked, this, [this, table](int row, int) {
        QTableWidgetItem* idItem = table->item(row, 0);
        if (idItem) {
            selectedItemId = idItem->text().toInt();
            ui->quantityInput->setValue(1);
        }
    });
}


void User_page::on_addToCartBtn_clicked()
{
    int qty = ui->quantityInput->value();

    if (selectedItemId == -1) {
        int currentTab = ui->categoryTabs->currentIndex();
        QString category = tabIndexToCategory.value(currentTab);
        QTableWidget* table = getTableForCategory(category);
        if (!table) {
            QMessageBox::warning(this, "Error", "Current category table not found.");
            return;
        }
        QModelIndexList selected = table->selectionModel()->selectedRows();
        if (selected.isEmpty()) {
            QMessageBox::warning(this, "Selection Required", "Please select an item to add.");
            return;
        }
        int row = selected.first().row();
        QTableWidgetItem* idItem = table->item(row, 0);
        if (!idItem) {
            QMessageBox::warning(this, "Error", "Selected item is invalid.");
            return;
        }
        selectedItemId = idItem->text().toInt();
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

    if (orderManager.addToCart(username.toStdString(), selectedItemId, qty)) {
        QMessageBox::information(this, "Added to Cart", QString("Added %1 x %2 to your cart.")
                                     .arg(qty)
                                     .arg(QString::fromStdString(it->name)));
        loadItemsForCategory(category);
        selectedItemId = -1;
        ui->quantityInput->setValue(1);
    } else {
        QMessageBox::warning(this, "Failed", "Could not add item to cart.");
    }
}

void User_page::on_checkoutBtn_clicked() {
    // Check if user has items in cart
    auto cartItems = orderManager.getCartItems(username.toStdString());
    if (cartItems.empty()) {
        QMessageBox::information(this, "Cart Empty", "Your cart is empty. Add some items first.");
        return;
    }

    // Just emit the signal to open checkout page - don't place order yet
    emit checkoutRequested();
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
    SettingsPage settingsDialog(username, this);

    std::vector<PurchaseRecord> purchaseHistory;
    auto orders = orderManager.getUserOrders(username.toStdString());
    for (const auto& order : orders) {
        PurchaseRecord rec;
        rec.productName = QString::fromStdString(inventory.getItemName(order.itemId));
        rec.quantity = order.quantity;
        rec.amount = inventory.getPrice(order.itemId) * order.quantity;
        purchaseHistory.push_back(rec);
    }
    settingsDialog.setPurchaseHistory(purchaseHistory);

    settingsDialog.exec();
}

void User_page::on_settingsBtn_clicked()
{
    showSettingsDialog();
}

void User_page::on_viewCartBtn_clicked() {
    auto cartItems = orderManager.getCartItems(username.toStdString());
    if (cartItems.empty()) {
        QMessageBox::information(this, "Cart", "Your cart is empty.");
        return;
    }

    QString summary = "🛒 Your Cart:\n";
    for (const auto& item : cartItems) {
        summary += QString("• %1 x %2 (Rs. %3)\n")
                       .arg(QString::fromStdString(item.itemName))
                       .arg(item.quantity)
                       .arg(item.price * item.quantity, 0, 'f', 2);
    }
    QMessageBox::information(this, "Cart Contents", summary);
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

void User_page::on_searchItemBtn_clicked()
{
    QString searchText = ui->searchItemInput->text().trimmed();

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

    for (int i = 0; i < ui->categoryTabs->count(); ++i) {
        QTableWidget* table = getTableForCategory(tabIndexToCategory[i]);
        if (table) {
            table->clearContents();
            table->setRowCount(0);
        }
        ui->categoryTabs->setTabVisible(i, false);
    }

    QMap<QString, QVector<InventoryItem>> matchedByCategory;

    for (const auto& item : inventory.getAllItems()) {
        QString itemName = QString::fromStdString(item.name);
        if (itemName.contains(searchText, Qt::CaseInsensitive)) {
            QString category = QString::fromStdString(item.category);
            matchedByCategory[category].push_back(item);
        }
    }

    for (auto it = matchedByCategory.constBegin(); it != matchedByCategory.constEnd(); ++it) {
        QString category = it.key();
        const QVector<InventoryItem>& items = it.value();

        int tabIndex = tabIndexToCategory.key(category, -1);
        if (tabIndex == -1) continue;

        ui->categoryTabs->setTabVisible(tabIndex, true);

        QTableWidget* table = getTableForCategory(category);
        if (!table) continue;

        table->setColumnCount(6);
        table->setHorizontalHeaderLabels({"ID", "Name", "Available Stock", "Price", "Description", "Buy"});

        table->setRowCount(items.size());

        for (int row = 0; row < items.size(); ++row) {
            const auto& item = items[row];
            table->setItem(row, 0, new QTableWidgetItem(QString::number(item.id)));
            table->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(item.name)));
            table->setItem(row, 2, new QTableWidgetItem(QString::number(item.quantity)));
            table->setItem(row, 3, new QTableWidgetItem(QString::number(item.price, 'f', 2)));
            table->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(item.description)));

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

                if (orderManager.addToCart(username.toStdString(), itemId, qty)) {
                    QMessageBox::information(this, "Added to Cart", QString("Added %1 x %2 to your cart.")
                                                 .arg(qty)
                                                 .arg(buyBtn->property("itemName").toString()));
                    loadItemsForCategory(category);
                } else {
                    QMessageBox::warning(this, "Failed", "Could not add item to cart.");
                }
            });

            table->setCellWidget(row, 5, buyBtn);
        }
    }
}

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

void User_page::on_categoryTabs_currentChanged(int index)
{
    if (tabIndexToCategory.contains(index)) {
        loadItemsForCategory(tabIndexToCategory[index]);
        selectedItemId = -1;
        ui->quantityInput->setValue(1);
    }
}
