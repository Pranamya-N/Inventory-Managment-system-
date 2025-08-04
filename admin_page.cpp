#include "admin_page.h"
#include "ui_admin_page.h"

#include <QMessageBox>
#include <QDebug>
#include <QString>
#include <QTableWidgetItem>
#include <QHeaderView>

Admin_page::Admin_page(UserDataStore& userStore, InventoryManager& inventoryMgr, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Admin_page),
    userDataStore(userStore),
    inventoryManager(inventoryMgr)
{
    ui->setupUi(this);

    // Setup tables with headers
    setupUserTable();
    setupInventoryTables();

    // Connect buttons to slots
    connect(ui->searchUserBtn, &QPushButton::clicked, this, &Admin_page::onSearchUser);
    connect(ui->deleteUserBtn, &QPushButton::clicked, this, &Admin_page::onDeleteUser);
    connect(ui->addItemBtn, &QPushButton::clicked, this, &Admin_page::onAddItem);
    connect(ui->updateItemBtn, &QPushButton::clicked, this, &Admin_page::onUpdateItem);
    connect(ui->searchItemBtn, &QPushButton::clicked, this, &Admin_page::onSearchItem);
    connect(ui->removeItemBtn, &QPushButton::clicked, this, &Admin_page::onRemoveItem);
    connect(ui->refreshBtn, &QPushButton::clicked, this, &Admin_page::refreshAllTables);
    connect(ui->logoutBtn, &QPushButton::clicked, this, &Admin_page::logoutRequested);

    // **Connect Add Description button here** (replace addDescriptionBtn with actual button name)
    connect(ui->addDescriptionBtn, &QPushButton::clicked, this, &Admin_page::onAddDescription);

    refreshAllTables();
}


Admin_page::~Admin_page()
{
    delete ui;
}

// Setup User table headers
void Admin_page::setupUserTable()
{
    ui->tableUsers->setColumnCount(4);
    QStringList headers = {"Username", "Password", "Role", "Status"};
    ui->tableUsers->setHorizontalHeaderLabels(headers);
    ui->tableUsers->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

// Setup inventory tables headers for each category tab
void Admin_page::setupInventoryTables()
{
    // Common headers for inventory tables
    QStringList headers = {"ID", "Name", "Quantity", "Price (Rs)", "Category"};

    auto setupTable = [&](QTableWidget* table) {
        table->setColumnCount(5);
        table->setHorizontalHeaderLabels(headers);
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    };

    setupTable(ui->tableElectronics);
    setupTable(ui->tableGroceries);
    setupTable(ui->tableFurniture);
    setupTable(ui->tableCosmetics);
    setupTable(ui->tableClothes);
    setupTable(ui->tableSmoking);
    setupTable(ui->tableLiquors);
}

// Refresh all tables
void Admin_page::refreshAllTables()
{
    populateUserTable();
    populateInventoryTables();
}

// Populate User Table with all users
void Admin_page::populateUserTable()
{
    ui->tableUsers->setRowCount(0);
    const auto& users = userDataStore.getAllUsers();

    for (const auto& user : users)
    {
        int row = ui->tableUsers->rowCount();
        ui->tableUsers->insertRow(row);

        ui->tableUsers->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(user.username)));
        ui->tableUsers->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(user.password)));
        ui->tableUsers->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(user.role)));
        ui->tableUsers->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(user.status)));
    }
}

// Populate inventory tables by category
void Admin_page::populateInventoryTables()
{
    // Clear all tables first
    ui->tableElectronics->setRowCount(0);
    ui->tableGroceries->setRowCount(0);
    ui->tableFurniture->setRowCount(0);
    ui->tableCosmetics->setRowCount(0);
    ui->tableClothes->setRowCount(0);
    ui->tableSmoking->setRowCount(0);
    ui->tableLiquors->setRowCount(0);

    const auto& items = inventoryManager.getAllItems();
    for (const auto& item : items)
    {
        QTableWidget* targetTable = nullptr;
        QString category = QString::fromStdString(item.category).toLower();

        if (category == "electronics") targetTable = ui->tableElectronics;
        else if (category == "groceries") targetTable = ui->tableGroceries;
        else if (category == "furniture") targetTable = ui->tableFurniture;
        else if (category == "cosmetics") targetTable = ui->tableCosmetics;
        else if (category == "clothes") targetTable = ui->tableClothes;
        else if (category == "smoking products") targetTable = ui->tableSmoking;
        else if (category == "liquors") targetTable = ui->tableLiquors;

        if (targetTable)
        {
            int row = targetTable->rowCount();
            targetTable->insertRow(row);
            targetTable->setItem(row, 0, new QTableWidgetItem(QString::number(item.id)));
            targetTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(item.name)));
            targetTable->setItem(row, 2, new QTableWidgetItem(QString::number(item.quantity)));
            targetTable->setItem(row, 3, new QTableWidgetItem(QString("Rs %1").arg(item.price, 0, 'f', 2)));
            targetTable->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(item.category)));
        }
    }
}

// Search user by username and show result in user table (filtered)
void Admin_page::onSearchUser()
{
    QString searchText = ui->searchUserInput->text().trimmed();
    if (searchText.isEmpty())
    {
        QMessageBox::information(this, "Input Required", "Please enter a username to search.");
        return;
    }

    ui->tableUsers->setRowCount(0);
    const auto& users = userDataStore.getAllUsers();
    for (const auto& user : users)
    {
        if (QString::fromStdString(user.username).contains(searchText, Qt::CaseInsensitive))
        {
            int row = ui->tableUsers->rowCount();
            ui->tableUsers->insertRow(row);
            ui->tableUsers->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(user.username)));
            ui->tableUsers->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(user.password)));
            ui->tableUsers->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(user.role)));
            ui->tableUsers->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(user.status)));
        }
    }
}

// Delete user by username (cannot delete admin)
void Admin_page::onDeleteUser()
{
    QString username = ui->deleteUserInput->text().trimmed();
    if (username.isEmpty())
    {
        QMessageBox::warning(this, "Input Required", "Please enter username to delete.");
        return;
    }

    // Check user exists
    if (!userDataStore.userExists(username.toStdString()))
    {
        QMessageBox::warning(this, "User Not Found", "No user found with that username.");
        return;
    }

    // Prevent deleting admin users
    std::string role = userDataStore.getUserRole(username.toStdString());
    if (role == "admin")
    {
        QMessageBox::warning(this, "Action Denied", "Cannot delete an admin user.");
        return;
    }

    bool success = userDataStore.deleteUser(username.toStdString());
    if (success)
    {
        QMessageBox::information(this, "Success", "User deleted successfully.");
        ui->deleteUserInput->clear();
        populateUserTable();
    }
    else
    {
        QMessageBox::critical(this, "Failed", "Failed to delete user.");
    }
}

// Add new item to inventory
void Admin_page::onAddItem()
{
    bool okId;
    int id = ui->idInput->text().toInt(&okId);
    if (!okId || id <= 0)
    {
        QMessageBox::warning(this, "Invalid Input", "Please enter a valid positive integer ID.");
        return;
    }

    QString name = ui->nameInput->text().trimmed();
    if (name.isEmpty())
    {
        QMessageBox::warning(this, "Invalid Input", "Please enter item name.");
        return;
    }

    int quantity = ui->quantityInput->value();
    double price = ui->priceInput->value();
    QString category = ui->categoryComboBox->currentText();

    InventoryItem item;
    item.id = id;
    item.name = name.toStdString();
    item.quantity = quantity;
    item.price = price;
    item.category = category.toStdString();

    inventoryManager.addItem(item);
    QMessageBox::information(this, "Success", "Item added/updated successfully.");
    clearAddItemInputs();
    populateInventoryTables();
}

// Clear add item inputs after adding
void Admin_page::clearAddItemInputs()
{
    ui->idInput->clear();
    ui->nameInput->clear();
    ui->quantityInput->setValue(0);
    ui->priceInput->setValue(0.0);
    ui->categoryComboBox->setCurrentIndex(0);
}

// Update existing item by ID or Name - set price directly and increase quantity
void Admin_page::onUpdateItem()
{
    QString identifier = ui->updateIdOrNameInput->text().trimmed();
    if (identifier.isEmpty())
    {
        QMessageBox::warning(this, "Input Required", "Please enter Item ID or Name to update.");
        return;
    }

    bool okId;
    int id = identifier.toInt(&okId);

    double newPrice = ui->changePriceInput->value();
    int quantityIncrease = ui->increaseQuantityInput->value();

    if (newPrice == 0.0 && quantityIncrease == 0)
    {
        QMessageBox::information(this, "No Changes", "Please specify a new price or quantity to increase.");
        return;
    }

    // Try to find the item by ID or Name
    InventoryItem item;
    bool found = false;
    if (okId)
    {
        found = inventoryManager.getItem(id, item);
    }
    else
    {
        // Search by name (case-insensitive)
        const auto& items = inventoryManager.getAllItems();
        for (const auto& i : items)
        {
            if (QString::fromStdString(i.name).compare(identifier, Qt::CaseInsensitive) == 0)
            {
                item = i;
                found = true;
                break;
            }
        }
    }

    if (!found)
    {
        QMessageBox::warning(this, "Not Found", "Item not found with given ID or Name.");
        return;
    }

    // Update price and quantity
    if (newPrice > 0.0)
        item.price = newPrice; // set new price directly
    if (quantityIncrease > 0)
        item.quantity += quantityIncrease; // increase quantity

    inventoryManager.addItem(item); // addItem uses INSERT OR REPLACE
    QMessageBox::information(this, "Success", "Item updated successfully.");
    clearUpdateInputs();
    populateInventoryTables();
}

// Clear update inputs
void Admin_page::clearUpdateInputs()
{
    ui->updateIdOrNameInput->clear();
    ui->changePriceInput->setValue(0.0);
    ui->increaseQuantityInput->setValue(0);
}

// Update existing item by ID or Name - set price directly and increase quantity


// Add description to an existing item by ID
void Admin_page::onAddDescription()
{
    bool ok;
    int id = ui->descItemIdInput->text().toInt(&ok);  // ✅ Fixed name from descIdInput
    if (!ok || id <= 0) {
        QMessageBox::warning(this, "Invalid Input", "Please enter a valid Item ID.");
        return;
    }

    QString description = ui->descInput->toPlainText().trimmed();  // ✅ Use toPlainText for QTextEdit
    if (description.isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Description cannot be empty.");
        return;
    }

    InventoryItem item;
    if (!inventoryManager.getItem(id, item)) {
        QMessageBox::warning(this, "Not Found", "No item found with that ID.");
        return;
    }

    item.description = description.toStdString();  // ✅ description field must exist in InventoryItem
    inventoryManager.addItem(item);  // replace item

    QMessageBox::information(this, "Success", "Description updated successfully.");

    ui->descItemIdInput->clear();  // ✅ clear ID input
    ui->descInput->clear();        // ✅ clear QTextEdit

    populateInventoryTables();
}


void Admin_page::onSearchItem()
{
    QString searchText = ui->searchItemInput->text().trimmed();
    if (searchText.isEmpty())
    {
        QMessageBox::information(this, "Input Required", "Please enter ID or Name to search.");
        return;
    }

    bool okId;
    int id = searchText.toInt(&okId);

    ui->tableElectronics->setRowCount(0);
    ui->tableGroceries->setRowCount(0);
    ui->tableFurniture->setRowCount(0);
    ui->tableCosmetics->setRowCount(0);
    ui->tableClothes->setRowCount(0);
    ui->tableSmoking->setRowCount(0);
    ui->tableLiquors->setRowCount(0);

    const auto& items = inventoryManager.getAllItems();
    for (const auto& item : items)
    {
        bool matches = false;
        if (okId)
            matches = (item.id == id);
        else
            matches = (QString::fromStdString(item.name).contains(searchText, Qt::CaseInsensitive));

        if (matches)
        {
            QTableWidget* targetTable = nullptr;
            QString category = QString::fromStdString(item.category).toLower();

            if (category == "electronics") targetTable = ui->tableElectronics;
            else if (category == "groceries") targetTable = ui->tableGroceries;
            else if (category == "furniture") targetTable = ui->tableFurniture;
            else if (category == "cosmetics") targetTable = ui->tableCosmetics;
            else if (category == "clothes") targetTable = ui->tableClothes;
            else if (category == "smoking products") targetTable = ui->tableSmoking;
            else if (category == "liquors") targetTable = ui->tableLiquors;

            if (targetTable)
            {
                int row = targetTable->rowCount();
                targetTable->insertRow(row);
                targetTable->setItem(row, 0, new QTableWidgetItem(QString::number(item.id)));
                targetTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(item.name)));
                targetTable->setItem(row, 2, new QTableWidgetItem(QString::number(item.quantity)));
                targetTable->setItem(row, 3, new QTableWidgetItem(QString("Rs %1").arg(item.price, 0, 'f', 2)));
                targetTable->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(item.category)));
            }
        }
    }
}

// Remove inventory item by ID
void Admin_page::onRemoveItem()
{
    bool okId;
    int id = ui->removeIdInput->text().toInt(&okId);
    if (!okId || id <= 0)
    {
        QMessageBox::warning(this, "Invalid Input", "Please enter a valid item ID to remove.");
        return;
    }

    bool success = inventoryManager.removeItemById(id);
    if (success)
    {
        QMessageBox::information(this, "Success", "Item removed successfully.");
        ui->removeIdInput->clear();
        populateInventoryTables();
    }
    else
    {
        QMessageBox::critical(this, "Failed", "Failed to remove item. Item ID may not exist.");
    }
}
