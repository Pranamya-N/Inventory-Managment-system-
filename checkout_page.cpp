#include "checkout_page.h"
#include "ui_checkout_page.h"

#include "backend_classes.h"
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QTimer>

CheckoutPage::CheckoutPage(const QString& user, OrderManager& orders, InventoryManager& inv, QWidget *parent)
    : QWidget(parent),
    ui(new Ui::CheckoutPage),
    username(user),
    orderManager(orders),
    inventory(inv)
{
    ui->setupUi(this);

    // Cart Table Setup
    ui->cartTable->setColumnCount(4);
    ui->cartTable->setHorizontalHeaderLabels({"Item Name", "Price (Rs.)", "Quantity", "Subtotal (Rs.)"});
    ui->cartTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->cartTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->cartTable->setSelectionMode(QAbstractItemView::SingleSelection);

    ui->cartTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->cartTable->setColumnWidth(1, 100);
    ui->cartTable->setColumnWidth(2, 80);
    ui->cartTable->setColumnWidth(3, 120);

    // Bill Table Setup
    ui->billTable->setColumnCount(3);
    ui->billTable->setHorizontalHeaderLabels({"Product", "Quantity", "Amount (Rs.)"});
    ui->billTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->billTable->setColumnWidth(1, 80);
    ui->billTable->setColumnWidth(2, 120);

    ui->billTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->billTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->billTable->setSelectionMode(QAbstractItemView::SingleSelection);



    populateCartTable();
    ui->stackedWidget->setCurrentWidget(ui->pageCartView);
}

CheckoutPage::~CheckoutPage()
{
    delete ui;
}

void CheckoutPage::populateCartTable()
{
    ui->cartTable->setRowCount(0);

    auto cartItems = orderManager.getCartItems(username.toStdString());
    ui->cartTable->setRowCount(cartItems.size());

    double total = 0.0;
    for (int i = 0; i < static_cast<int>(cartItems.size()); ++i) {
        const auto& cartItem = cartItems[i];

        QString itemName = QString::fromStdString(cartItem.itemName);
        double price = cartItem.price;
        int quantity = cartItem.quantity;
        double subtotal = price * quantity;
        total += subtotal;

        ui->cartTable->setItem(i, 0, new QTableWidgetItem(itemName));

        QTableWidgetItem* priceItem = new QTableWidgetItem(QString::number(price, 'f', 2));
        priceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->cartTable->setItem(i, 1, priceItem);

        QTableWidgetItem* quantityItem = new QTableWidgetItem(QString::number(quantity));
        quantityItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->cartTable->setItem(i, 2, quantityItem);

        QTableWidgetItem* subtotalItem = new QTableWidgetItem(QString::number(subtotal, 'f', 2));
        subtotalItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->cartTable->setItem(i, 3, subtotalItem);
    }

    ui->labelBill->setText(
        QString("Your bill is Rs. %1. Please pay at the counter or deposit to bank account number 123456.")
            .arg(QString::number(total, 'f', 2))
        );
}

void CheckoutPage::populateBillTable()
{
    ui->billTable->setRowCount(0);

    auto cartItems = orderManager.getCartItems(username.toStdString());
    ui->billTable->setRowCount(cartItems.size() + 1);

    double grandTotal = 0.0;

    for (int i = 0; i < static_cast<int>(cartItems.size()); ++i) {
        const auto& cartItem = cartItems[i];

        QString itemName = QString::fromStdString(cartItem.itemName);
        int quantity = cartItem.quantity;
        double price = cartItem.price;
        double amount = price * quantity;
        grandTotal += amount;

        ui->billTable->setItem(i, 0, new QTableWidgetItem(itemName));

        QTableWidgetItem* quantityItem = new QTableWidgetItem(QString::number(quantity));
        quantityItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->billTable->setItem(i, 1, quantityItem);

        QTableWidgetItem* amountItem = new QTableWidgetItem(QString::number(amount, 'f', 2));
        amountItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->billTable->setItem(i, 2, amountItem);
    }

    QTableWidgetItem* totalLabelItem = new QTableWidgetItem("Grand Total");
    totalLabelItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->billTable->setItem(cartItems.size(), 0, totalLabelItem);

    QTableWidgetItem* emptyItem = new QTableWidgetItem("");
    ui->billTable->setItem(cartItems.size(), 1, emptyItem);

    QTableWidgetItem* grandTotalItem = new QTableWidgetItem(QString::number(grandTotal, 'f', 2));
    grandTotalItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->billTable->setItem(cartItems.size(), 2, grandTotalItem);
}

void CheckoutPage::onRemoveItemClicked()
{
    int row = ui->cartTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Remove Item", "Please select an item in the cart to remove.");
        return;
    }

    auto cartItems = orderManager.getCartItems(username.toStdString());
    if (row >= static_cast<int>(cartItems.size())) return;

    int itemId = cartItems[row].itemId;
    orderManager.removeFromCart(username.toStdString(), itemId);

    populateCartTable();
}

void CheckoutPage::onDecreaseQuantityClicked()
{
    int row = ui->cartTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Decrease Quantity", "Please select an item in the cart.");
        return;
    }

    auto cartItems = orderManager.getCartItems(username.toStdString());
    if (row >= static_cast<int>(cartItems.size())) return;

    int itemId = cartItems[row].itemId;

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
    auto cartItems = orderManager.getCartItems(username.toStdString());
    if (cartItems.empty()) {
        QMessageBox::information(this, "Checkout", "Your cart is empty.");
        return;
    }

    populateBillTable();
    ui->stackedWidget->setCurrentWidget(ui->pageBillView);
}

void CheckoutPage::on_billBackButton_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->pageCartView);
}

void CheckoutPage::on_confirmPaymentButton_clicked()
{
    ui->confirmPaymentButton->setEnabled(false);
    ui->confirmPaymentButton->setText("Processing...");

    // Place order directly
    bool success = orderManager.placeOrder(username.toStdString(), inventory);

    if (success) {
        QMessageBox::information(this, "Payment Successful",
                                 "Payment done. Thanks for visiting!");
        populateCartTable();  // Cart should now be empty
        ui->stackedWidget->setCurrentWidget(ui->pageCartView);

        // Optionally return to user page after short delay
        QTimer::singleShot(2000, this, [this]() {
            emit backToUserPage();
        });
    } else {
        QMessageBox::critical(this, "Payment Failed",
                              "Something went wrong. Please try again.");
    }

    ui->confirmPaymentButton->setEnabled(true);
    ui->confirmPaymentButton->setText("Confirm Payment");
}
