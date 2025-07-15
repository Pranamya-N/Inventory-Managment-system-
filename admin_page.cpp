#include "admin_page.h"
#include "ui_admin_page.h"
#include "backend_classes.h"

#include <QMessageBox>
#include <QTableWidgetItem>
#include <QHeaderView>

Admin_page::Admin_page(InventoryManager& inv, UserDataStore& users, QWidget *parent)
    : QWidget(parent),
    ui(new Ui::Admin_page),
    inventory(inv),
    userData(users)
{
    ui->setupUi(this);

    ui->priceInput->setMaximum(1000000.00); // Max Rs 10 lakhs

    // Setup users table
    ui->tableUsers->setColumnCount(4);
    ui->tableUsers->setHorizontalHeaderLabels({"Username", "Password", "Role", "Status"});
    ui->tableUsers->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Setup item tables column headers
    QStringList itemHeaders = {"ID", "Name", "Quantity", "Price (Rs)"};
    auto setupItemTable = [&](QTableWidget* table) {
        table->setColumnCount(itemHeaders.size());
        table->setHorizontalHeaderLabels(itemHeaders);
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    };

    setupItemTable(ui->tableElectronics);
    setupItemTable(ui->tableGroceries);
    setupItemTable(ui->tableFurniture);
    setupItemTable(ui->tableCosmetics);
    setupItemTable(ui->tableClothes);
    setupItemTable(ui->tableSmoking);
    setupItemTable(ui->tableLiquors);  // Added Liquors table setup

    refreshAllItemTables();
    refreshUsersTable();
}

Admin_page::~Admin_page()
{
    delete ui;
}

void Admin_page::loadItemsToTable(const QString &category, QTableWidget *table, const QString &search)
{
    if (!table) return;

    table->setRowCount(0);

    const auto& items = inventory.getAllItems();
    int row = 0;

    for (const auto& item : items) {
        QString itemCategory = QString::fromStdString(item.category);
        if (itemCategory != category) continue;

        QString idStr = QString::number(item.id);
        QString nameStr = QString::fromStdString(item.name);

        if (!search.isEmpty() &&
            !nameStr.contains(search, Qt::CaseInsensitive) &&
            idStr != search)
            continue;

        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(idStr));
        table->setItem(row, 1, new QTableWidgetItem(nameStr));
        table->setItem(row, 2, new QTableWidgetItem(QString::number(item.quantity)));
        table->setItem(row, 3, new QTableWidgetItem(QString("Rs %1").arg(item.price, 0, 'f', 2)));
        ++row;
    }
}

void Admin_page::refreshAllItemTables(const QString &search)
{
    loadItemsToTable("Electronics", ui->tableElectronics, search);
    loadItemsToTable("Groceries", ui->tableGroceries, search);
    loadItemsToTable("Furniture", ui->tableFurniture, search);
    loadItemsToTable("Cosmetics", ui->tableCosmetics, search);
    loadItemsToTable("Clothes", ui->tableClothes, search);
    loadItemsToTable("Smoking Products", ui->tableSmoking, search);
    loadItemsToTable("Liquors", ui->tableLiquors, search);  // Refresh Liquors tab
}

void Admin_page::loadUsersToTable(const QString &search)
{
    QTableWidget *table = ui->tableUsers;

    table->setRowCount(0);

    const auto& users = userData.getAllUsers();
    int row = 0;

    for (const auto& user : users) {
        QString uname = QString::fromStdString(user.username);
        if (search.isEmpty() || uname.contains(search, Qt::CaseInsensitive)) {
            table->insertRow(row);
            table->setItem(row, 0, new QTableWidgetItem(uname));
            table->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(user.password)));
            table->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(user.role)));
            table->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(user.status)));
            ++row;
        }
    }
}

void Admin_page::refreshUsersTable()
{
    loadUsersToTable(ui->searchUserInput->text());
}

void Admin_page::on_addItemBtn_clicked()
{
    int id = ui->idInput->text().toInt();
    QString name = ui->nameInput->text();
    int qty = ui->quantityInput->value();
    double price = ui->priceInput->value();
    QString category = ui->categoryComboBox->currentText();

    if (id <= 0 || name.isEmpty() || qty <= 0 || price <= 0.0) {
        QMessageBox::warning(this, "Invalid Input", "Please fill all fields with valid data.");
        return;
    }

    if (inventory.hasItem(id)) {
        QMessageBox::warning(this, "Duplicate ID", "An item with this ID already exists.");
        return;
    }

    inventory.addItem({id, name.toStdString(), qty, price, category.toStdString()});
    QMessageBox::information(this, "Success", "Item added successfully.");
    refreshAllItemTables();
}

void Admin_page::on_removeItemBtn_clicked()
{
    int id = ui->removeIdInput->text().toInt();
    if (id <= 0) {
        QMessageBox::warning(this, "Invalid ID", "Please enter a valid item ID.");
        return;
    }

    if (inventory.removeItemById(id)) {
        QMessageBox::information(this, "Removed", "Item removed successfully.");
    } else {
        QMessageBox::warning(this, "Error", "Item not found.");
    }

    refreshAllItemTables();
}

void Admin_page::on_searchItemBtn_clicked()
{
    QString searchTerm = ui->searchItemInput->text();
    refreshAllItemTables(searchTerm);
}

void Admin_page::on_searchUserBtn_clicked()
{
    refreshUsersTable();
}

void Admin_page::on_deleteUserBtn_clicked()
{
    QString username = ui->deleteUserInput->text();

    if (username.isEmpty()) {
        QMessageBox::warning(this, "Missing Input", "Enter a username to delete.");
        return;
    }

    if (userData.deleteUser(username.toStdString())) {
        QMessageBox::information(this, "Success", "User deleted successfully.");
    } else {
        QMessageBox::warning(this, "Failed", "User not found or is an admin.");
    }

    refreshUsersTable();
}

void Admin_page::on_refreshBtn_clicked()
{
    refreshAllItemTables();
    refreshUsersTable();
}

void Admin_page::on_logoutBtn_clicked()
{
    emit logoutRequested();
}
