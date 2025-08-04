#include "checkout_page.h"
#include "ui_checkout_page.h"

#include "backend_classes.h"
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QTimer>
#include <QHeaderView>
#include <QPushButton>

CheckoutPage::CheckoutPage(const QString& user, OrderManager& orders, InventoryManager& inv, QWidget *parent)
    : QWidget(parent),
    ui(new Ui::CheckoutPage),
    username(user),
    orderManager(orders),
    inventory(inv)
{
    ui->setupUi(this);

    // Set up cart table with 6 columns including buttons
    ui->cartTable->setColumnCount(6);
    ui->cartTable->setHorizontalHeaderLabels({"Item Name", "Price (Rs.)", "Quantity", "Subtotal (Rs.)", "-", "Remove"});
    ui->cartTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->cartTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->cartTable->setSelectionMode(QAbstractItemView::SingleSelection);

    // Resize columns for better layout
    ui->cartTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);  // Item Name stretches
    ui->cartTable->setColumnWidth(1, 100);  // Price
    ui->cartTable->setColumnWidth(2, 80);   // Quantity
    ui->cartTable->setColumnWidth(3, 120);  // Subtotal
    ui->cartTable->setColumnWidth(4, 40);   // Decrease button (a bit wider for '-')
    ui->cartTable->setColumnWidth(5, 90);   // Remove button

    // Bill Table Setup (no changes here)
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

        // Set table items
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

        // Create decrease quantity button ('-')
        QPushButton* decButton = new QPushButton("-");
        decButton->setFixedSize(30, 24);

        QWidget* decButtonWidget = new QWidget();
        QHBoxLayout* decLayout = new QHBoxLayout(decButtonWidget);
        decLayout->addWidget(decButton);
        decLayout->setContentsMargins(0, 0, 0, 0);
        decLayout->setAlignment(Qt::AlignCenter);
        ui->cartTable->setCellWidget(i, 4, decButtonWidget);

        connect(decButton, &QPushButton::clicked, this, [this, i]() {
            auto cartItems = orderManager.getCartItems(username.toStdString());
            if (i >= 0 && i < static_cast<int>(cartItems.size())) {
                int itemId = cartItems[i].itemId;
                if (!orderManager.decreaseOrderQuantity(username.toStdString(), itemId)) {
                    QMessageBox::information(this, "Quantity", "Cannot decrease quantity below 1.");
                } else {
                    populateCartTable();
                }
            }
        });

        // Create remove item button
        QPushButton* remButton = new QPushButton("Remove");
        remButton->setFixedSize(60, 24);

        QWidget* remButtonWidget = new QWidget();
        QHBoxLayout* remLayout = new QHBoxLayout(remButtonWidget);
        remLayout->addWidget(remButton);
        remLayout->setContentsMargins(0, 0, 0, 0);
        remLayout->setAlignment(Qt::AlignCenter);
        ui->cartTable->setCellWidget(i, 5, remButtonWidget);

        connect(remButton, &QPushButton::clicked, this, [this, i]() {
            auto cartItems = orderManager.getCartItems(username.toStdString());
            if (i >= 0 && i < static_cast<int>(cartItems.size())) {
                int itemId = cartItems[i].itemId;
                orderManager.removeFromCart(username.toStdString(), itemId);
                populateCartTable();
            }
        });
    }

    ui->labelBill->setText(
        QString("Your bill is Rs. %1. Please pay at the counter or deposit to bank account number 123456.")
            .arg(QString::number(total, 'f', 2))
        );
}

void CheckoutPage::onRemoveButtonClicked(int row)
{
    auto cartItems = orderManager.getCartItems(username.toStdString());
    if (row >= 0 && row < static_cast<int>(cartItems.size())) {
        int itemId = cartItems[row].itemId;
        orderManager.removeFromCart(username.toStdString(), itemId);
        populateCartTable();
    }
}

void CheckoutPage::onDecreaseButtonClicked(int row)
{
    auto cartItems = orderManager.getCartItems(username.toStdString());
    if (row >= 0 && row < static_cast<int>(cartItems.size())) {
        int itemId = cartItems[row].itemId;
        if (!orderManager.decreaseOrderQuantity(username.toStdString(), itemId)) {
            QMessageBox::information(this, "Quantity", "Cannot decrease quantity below 1.");
        } else {
            populateCartTable();
        }
    }
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
