#include "checkout_page.h"
#include "ui_checkout_page.h"

#include "backend_classes.h"
#include <QTableWidgetItem>
#include <QMessageBox>
#include <algorithm>

CheckoutPage::CheckoutPage(const QString& user, OrderManager& orders, InventoryManager& inv, QWidget *parent)
    : QWidget(parent),
    ui(new Ui::CheckoutPage),
    username(user),
    orderManager(orders),
    inventory(inv)
{
    ui->setupUi(this);

    ui->cartTable->setColumnCount(4);
    ui->cartTable->setHorizontalHeaderLabels({"Item Name", "Price (Rs.)", "Quantity", "Subtotal (Rs.)"});
    ui->cartTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->cartTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->cartTable->setSelectionMode(QAbstractItemView::SingleSelection);

    // Set column resize modes:
    // Item Name column fixed wider, others share remaining space equally
    ui->cartTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->cartTable->setColumnWidth(0, 300); // Item Name column width fixed to 300 px
    ui->cartTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->cartTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    ui->cartTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);

    ui->billTable->setColumnCount(4);
    ui->billTable->setHorizontalHeaderLabels({"Product", "Quantity", "Amount (Rs.)", " "});
    ui->billTable->horizontalHeader()->setStretchLastSection(true);
    ui->billTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->billSection->setVisible(false); // Hide bill section initially

    connect(ui->backButton, &QPushButton::clicked, this, &CheckoutPage::on_backButton_clicked);
    connect(ui->confirmOrderButton, &QPushButton::clicked, this, &CheckoutPage::on_confirmOrderButton_clicked);
    connect(ui->billBackButton, &QPushButton::clicked, this, &CheckoutPage::on_billBackButton_clicked);
    connect(ui->confirmPaymentButton, &QPushButton::clicked, this, &CheckoutPage::on_confirmPaymentButton_clicked);

    populateCartTable();
}

CheckoutPage::~CheckoutPage()
{
    delete ui;
}

void CheckoutPage::populateCartTable()
{
    ui->cartTable->setRowCount(0);

    auto orders = orderManager.getUserOrders(username.toStdString());
    ui->cartTable->setRowCount(orders.size());

    double total = 0.0;
    for (int i = 0; i < static_cast<int>(orders.size()); ++i) {
        const auto& orderItem = orders[i];
        int itemId = orderItem.itemId;

        QString itemName = QString::fromStdString(inventory.getItemName(itemId));
        double price = inventory.getPrice(itemId);
        int quantity = orderItem.quantity;
        double subtotal = price * quantity;
        total += subtotal;

        ui->cartTable->setItem(i, 0, new QTableWidgetItem(itemName));
        ui->cartTable->setItem(i, 1, new QTableWidgetItem(QString::number(price, 'f', 2)));
        ui->cartTable->setItem(i, 2, new QTableWidgetItem(QString::number(quantity)));
        ui->cartTable->setItem(i, 3, new QTableWidgetItem(QString::number(subtotal, 'f', 2)));
    }

    ui->labelBill->setText(
        QString("Your bill is Rs. %1. Please pay at the counter or deposit to bank account number 123456.")
            .arg(QString::number(total, 'f', 2))
        );
}

double CheckoutPage::calculateTotal()
{
    double total = 0.0;
    auto orders = orderManager.getUserOrders(username.toStdString());

    for (const auto& orderItem : orders) {
        int itemId = orderItem.itemId;
        double price = inventory.getPrice(itemId);
        int quantity = orderItem.quantity;
        total += price * quantity;
    }

    return total;
}

void CheckoutPage::populateBillTable()
{
    ui->billTable->setRowCount(0);

    auto orders = orderManager.getUserOrders(username.toStdString());
    ui->billTable->setRowCount(orders.size() + 1);  // +1 for grand total row

    double grandTotal = 0.0;

    for (int i = 0; i < static_cast<int>(orders.size()); ++i) {
        const auto& orderItem = orders[i];
        int itemId = orderItem.itemId;

        QString itemName = QString::fromStdString(inventory.getItemName(itemId));
        int quantity = orderItem.quantity;
        double price = inventory.getPrice(itemId);
        double amount = price * quantity;
        grandTotal += amount;

        ui->billTable->setItem(i, 0, new QTableWidgetItem(itemName));
        ui->billTable->setItem(i, 1, new QTableWidgetItem(QString::number(quantity)));
        ui->billTable->setItem(i, 2, new QTableWidgetItem(QString::number(amount, 'f', 2)));
    }

    // Add grand total row
    QTableWidgetItem* totalLabelItem = new QTableWidgetItem("Grand Total");
    totalLabelItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->billTable->setItem(orders.size(), 0, totalLabelItem);

    ui->billTable->setSpan(orders.size(), 0, 1, 1); // spans only column 0

    QTableWidgetItem* emptyItem = new QTableWidgetItem("");
    ui->billTable->setItem(orders.size(), 1, emptyItem);

    QTableWidgetItem* grandTotalItem = new QTableWidgetItem(QString::number(grandTotal, 'f', 2));
    grandTotalItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->billTable->setItem(orders.size(), 2, grandTotalItem);
}

void CheckoutPage::onRemoveItemClicked()
{
    int row = ui->cartTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Remove Item", "Please select an item in the cart to remove.");
        return;
    }

    auto orders = orderManager.getUserOrders(username.toStdString());
    if (row >= static_cast<int>(orders.size())) return;

    int itemId = orders[row].itemId;
    orderManager.removeOrder(username.toStdString(), itemId);

    populateCartTable();
}

void CheckoutPage::onDecreaseQuantityClicked()
{
    int row = ui->cartTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Decrease Quantity", "Please select an item in the cart.");
        return;
    }

    auto orders = orderManager.getUserOrders(username.toStdString());
    if (row >= static_cast<int>(orders.size())) return;

    int itemId = orders[row].itemId;

    if (!orderManager.decreaseOrderQuantity(username.toStdString(), itemId)) {
        QMessageBox::information(this, "Quantity", "Cannot decrease quantity below 1.");
    } else {
        populateCartTable();
    }
}

void CheckoutPage::on_backButton_clicked()
{
    emit backToUserPage();
}

void CheckoutPage::on_confirmOrderButton_clicked()
{
    auto orders = orderManager.getUserOrders(username.toStdString());
    if (orders.empty()) {
        QMessageBox::information(this, "Checkout", "Your cart is empty.");
        return;
    }

    populateBillTable();
    ui->billSection->setVisible(true);
}

void CheckoutPage::on_billBackButton_clicked()
{
    ui->billSection->setVisible(false);
}

void CheckoutPage::on_confirmPaymentButton_clicked()
{
    auto orders = orderManager.getUserOrders(username.toStdString());
    if (orders.empty()) {
        QMessageBox::information(this, "Payment", "No orders to pay.");
        return;
    }

    QMessageBox::information(this, "Payment", "Payment successful! Thank you for your purchase.");

    orderManager.clearUserOrders(username.toStdString());  // Must be implemented in backend

    ui->billSection->setVisible(false);
    populateCartTable();
}
