#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include "backend_classes.h"
#include "user_page.h"
#include "admin_page.h"
#include "checkout_page.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_adminBtn_clicked();
    void on_userBtn_clicked();
    void on_backBtn_clicked();
    void on_loginBtn_clicked();
    void on_registerBtn_clicked();

    void on_showPasswordCheckbox_toggled(bool checked);

    void showCheckoutPage();  // <-- Declare the slot here

private:
    Ui::MainWindow *ui;
    QString selectedRole;



    // Track the currently logged-in user (needed for CheckoutPage)
    QString currentUser;

    InventoryManager inventory;
    UserDataStore userData;
    OrderManager orderManager;

    User_page* userPageWidget = nullptr;
    Admin_page* adminPageWidget = nullptr;
    CheckoutPage* checkoutPage = nullptr;

    void resetLoginUI();

signals:
    void checkoutRequested();
};

#endif // MAINWINDOW_H
