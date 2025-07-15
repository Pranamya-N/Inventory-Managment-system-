#ifndef CHECKOUT_PAGE_H
#define CHECKOUT_PAGE_H

#include <QWidget>
#include <QString>

namespace Ui {
class CheckoutPage;
}

class OrderManager;
class InventoryManager;

class CheckoutPage : public QWidget
{
    Q_OBJECT

public:
    explicit CheckoutPage(const QString& user, OrderManager& orders, InventoryManager& inv, QWidget *parent = nullptr);
    ~CheckoutPage();

signals:
    void backToUserPage();
    void logoutRequested();

private slots:
    void onRemoveItemClicked();
    void onDecreaseQuantityClicked();
    void on_backButton_clicked();
    void on_confirmOrderButton_clicked();
    void on_billBackButton_clicked();
    void on_confirmPaymentButton_clicked();
    void on_logoutButton_clicked();

private:
    Ui::CheckoutPage *ui;
    QString username;
    OrderManager& orderManager;
    InventoryManager& inventory;

    void populateCartTable();
    double calculateTotal();
    void populateBillTable();
};

#endif // CHECKOUT_PAGE_H
