#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql>
#include <QDebug>

// Forward declarations
class User_page;
class Admin_page;
class CheckoutPage;

#include "backend_classes.h"  // includes UserDataStore, InventoryManager, OrderManager

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
    void on_loginBtn_clicked();
    void on_registerBtn_clicked();
    void on_adminBtn_clicked();
    void on_userBtn_clicked();
    void on_backBtn_clicked();
    void on_showPasswordCheckbox_toggled(bool checked);
    void showCheckoutPage();
    void resetLoginUI();

private:
    Ui::MainWindow *ui;

    User_page* userPageWidget = nullptr;
    Admin_page* adminPageWidget = nullptr;
    CheckoutPage* checkoutPage = nullptr;

    QString currentUser;
    QString selectedRole;

    UserDataStore userData;              // ✅ Fixed: using proper UserDataStore
    InventoryManager inventory;
    OrderManager orderManager;

signals:
    void checkoutRequested();
};

#endif // MAINWINDOW_H
