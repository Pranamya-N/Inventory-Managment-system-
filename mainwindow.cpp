#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QVBoxLayout>
#include <QLayoutItem>
#include <QMessageBox>
#include "admin_page.h"
#include "checkout_page.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , userPageWidget(nullptr)
    , adminPageWidget(nullptr)
    , checkoutPage(nullptr)
{
    ui->setupUi(this);

    this->setStyleSheet(R"(
        QWidget {
            background-color: #0D0D1A;
            color: #E6F0FF;
            font-family: "Segoe UI", Tahoma, Geneva, Verdana, sans-serif;
            font-size: 13px;
        }
        QStackedWidget {
            background-color: #121224;
            border-radius: 8px;
        }
        QLabel {
            color: #A0BFE0;
            font-weight: 600;
        }
        QGroupBox {
            border: 1px solid #223A66;
            border-radius: 6px;
            margin-top: 10px;
            font-weight: 700;
            color: #8FAADC;
            background-color: #101020;
            padding: 10px;
        }
        QPushButton {
            background-color: #223A66;
            color: #FFFFFF;
            border: none;
            padding: 8px 16px;
            border-radius: 6px;
            font-weight: 600;
            min-width: 90px;
        }
        QPushButton:hover {
            background-color: #2D4D8B;
        }
        QPushButton:pressed {
            background-color: #1E335A;
        }
        QLineEdit {
            background-color: #1C1C2E;
            color: #E6F0FF;
            border: 1px solid #2A2A4A;
            border-radius: 6px;
            padding: 6px 8px;
        }
        QLineEdit:focus {
            border-color: #5FAFFF;
            outline: none;
        }
        QSpinBox {
            background-color: #1C1C2E;
            color: #E6F0FF;
            border: 1px solid #2A2A4A;
            border-radius: 6px;
            padding: 4px 8px;
            min-width: 80px;
        }
        QSpinBox:focus {
            border-color: #5FAFFF;
            outline: none;
        }
        QTabWidget::pane {
            border: 1px solid #1C1C2E;
            background: #121224;
            border-radius: 8px;
        }
        QTabBar::tab {
            background: #1A1A2E;
            color: #8FAADC;
            padding: 8px 20px;
            border-top-left-radius: 8px;
            border-top-right-radius: 8px;
            margin-right: 2px;
            min-width: 100px;
        }
        QTabBar::tab:selected {
            background: #23395B;
            color: #FFFFFF;
            font-weight: bold;
        }
        QTableWidget {
            background-color: #101020;
            gridline-color: #2A2A3D;
            alternate-background-color: #1A1A2E;
            color: #DDEBFF;
            border-radius: 8px;
        }
        QHeaderView::section {
            background-color: #1C1C3C;
            color: #A0BFE0;
            padding: 4px;
            border: 1px solid #2A2A4A;
            font-weight: bold;
        }
        QTableWidget::item:selected {
            background-color: #3A56A1;
            color: #FFFFFF;
        }
        QScrollBar:vertical {
            background: #121224;
            width: 12px;
            margin: 15px 3px 15px 3px;
            border-radius: 6px;
        }
        QScrollBar::handle:vertical {
            background: #223A66;
            min-height: 30px;
            border-radius: 6px;
        }
        QScrollBar::handle:vertical:hover {
            background: #2D4D8B;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            border: none;
            background: none;
        }
    )");

    ui->loginGroup->hide();
    ui->registerBtn->hide();
    ui->backBtn->hide();
    ui->messageLabel->clear();
    ui->roleLabel->show();
    ui->adminBtn->show();
    ui->userBtn->show();
    ui->stackedWidget->setCurrentIndex(0);
    ui->userPageContainer->setVisible(false);

    // --- Demo Inventory Items ---
    inventory.addItem({101, "Wireless Mouse", 25, 19.99, "Electronics"});
    inventory.addItem({102, "Bluetooth Keyboard", 15, 45.50, "Electronics"});
    inventory.addItem({103, "USB-C Monitor", 8, 150.00, "Electronics"});

    inventory.addItem({201, "Bananas", 100, 0.50, "Groceries"});
    inventory.addItem({202, "Organic Apples", 80, 1.20, "Groceries"});
    inventory.addItem({203, "Whole Wheat Bread", 60, 1.50, "Groceries"});

    inventory.addItem({301, "Modern Sofa", 5, 450.00, "Furniture"});
    inventory.addItem({302, "Dining Table", 3, 350.00, "Furniture"});
    inventory.addItem({303, "Office Chair", 10, 120.00, "Furniture"});

    inventory.addItem({401, "Red Lipstick", 30, 14.99, "Cosmetics"});
    inventory.addItem({402, "Eyeliner", 25, 9.99, "Cosmetics"});
    inventory.addItem({403, "Moisturizer Cream", 40, 18.50, "Cosmetics"});

    inventory.addItem({501, "Cotton T-Shirt", 50, 12.99, "Clothes"});
    inventory.addItem({502, "Jeans Pants", 40, 25.00, "Clothes"});
    inventory.addItem({503, "Leather Jacket", 15, 75.00, "Clothes"});

    inventory.addItem({601, "Cigarettes Pack", 120, 5.00, "Smoking Products"});
    inventory.addItem({602, "Hookah Set", 8, 60.00, "Smoking Products"});
    inventory.addItem({603, "Vape Pen", 20, 35.00, "Smoking Products"});

    inventory.addItem({701, "Red Wine", 50, 1200.00, "Liquors"});
    inventory.addItem({702, "Whiskey", 30, 2500.00, "Liquors"});
    inventory.addItem({703, "Vodka", 40, 1800.00, "Liquors"});
    inventory.addItem({704, "Beer Pack", 100, 900.00, "Liquors"});
    inventory.addItem({705, "Gin", 25, 2200.00, "Liquors"});

    // --- Demo Users ---
    userData.addUser("Pranamya Niroula", "niroula", "user");
    userData.addUser("Aaditya Thakuri", "thakuri", "user");
    userData.addUser("Aayush Timalsina", "timalsina", "user");
    userData.addUser("Santosh Tolange", "tolange", "user");
    userData.addUser("Pranish Shrestha", "shrestha", "user");

    // Connect Show Password checkbox
    connect(ui->showPasswordCheckbox, &QCheckBox::toggled, this, &MainWindow::on_showPasswordCheckbox_toggled);

    // Connect signal from user page to show checkout page
    connect(this, &MainWindow::checkoutRequested, this, &MainWindow::showCheckoutPage);
}

MainWindow::~MainWindow()
{
    delete ui;
    if (userPageWidget) delete userPageWidget;
    if (adminPageWidget) delete adminPageWidget;
    if (checkoutPage) delete checkoutPage;
}

void MainWindow::resetLoginUI()
{
    ui->loginGroup->hide();
    ui->backBtn->hide();
    ui->messageLabel->clear();
    ui->roleLabel->show();
    ui->adminBtn->show();
    ui->userBtn->show();
    ui->idInput->clear();
    ui->passwordInput->clear();
    ui->passwordInput->setEchoMode(QLineEdit::Password);
    ui->showPasswordCheckbox->setChecked(false);
    ui->stackedWidget->setCurrentIndex(0);
    ui->userPageContainer->setVisible(false);
    currentUser.clear();
}

void MainWindow::on_adminBtn_clicked()
{
    selectedRole = "admin";
    ui->loginGroup->show();
    ui->registerBtn->hide();
    ui->backBtn->show();
    ui->welcomeLabel->setText("Admin Login");
    ui->idInput->clear();
    ui->passwordInput->clear();
    ui->idInput->setPlaceholderText("Enter Admin ID");
    ui->passwordInput->setPlaceholderText("Enter Admin Password");
    ui->messageLabel->clear();
    ui->roleLabel->hide();
    ui->adminBtn->hide();
    ui->userBtn->hide();
}

void MainWindow::on_userBtn_clicked()
{
    selectedRole = "user";
    ui->loginGroup->show();
    ui->registerBtn->show();
    ui->backBtn->show();
    ui->welcomeLabel->setText("User Login");
    ui->idInput->clear();
    ui->passwordInput->clear();
    ui->idInput->setPlaceholderText("Enter User ID");
    ui->passwordInput->setPlaceholderText("Enter User Password");
    ui->messageLabel->clear();
    ui->roleLabel->hide();
    ui->adminBtn->hide();
    ui->userBtn->hide();
}

void MainWindow::on_backBtn_clicked()
{
    resetLoginUI();

    if (userPageWidget) {
        delete userPageWidget;
        userPageWidget = nullptr;
    }

    if (adminPageWidget) {
        delete adminPageWidget;
        adminPageWidget = nullptr;
    }

    if (checkoutPage) {
        delete checkoutPage;
        checkoutPage = nullptr;
    }
}

void MainWindow::on_loginBtn_clicked()
{
    QString id = ui->idInput->text();
    QString password = ui->passwordInput->text();

    if (selectedRole == "admin") {
        if (id == "admin" && password == "admin123") {
            if (adminPageWidget) {
                ui->stackedWidget->removeWidget(adminPageWidget);
                delete adminPageWidget;
                adminPageWidget = nullptr;
            }

            adminPageWidget = new Admin_page(inventory, userData, this);
            ui->stackedWidget->addWidget(adminPageWidget);
            ui->stackedWidget->setCurrentWidget(adminPageWidget);

            connect(adminPageWidget, &Admin_page::logoutRequested, this, [this]() {
                if (adminPageWidget) {
                    ui->stackedWidget->removeWidget(adminPageWidget);
                    delete adminPageWidget;
                    adminPageWidget = nullptr;
                }
                resetLoginUI();
            });

        } else {
            ui->messageLabel->setText("Invalid Admin ID or Password");
        }

    } else if (selectedRole == "user") {
        if (userData.isValidUser(id.toStdString(), password.toStdString())) {
            if (userPageWidget) {
                delete userPageWidget;
                userPageWidget = nullptr;
            }

            if (ui->userPageContainer->layout()) {
                QLayoutItem* item;
                while ((item = ui->userPageContainer->layout()->takeAt(0)) != nullptr) {
                    if (item->widget()) delete item->widget();
                    delete item;
                }
                delete ui->userPageContainer->layout();
            }

            userPageWidget = new User_page(id, inventory, orderManager, ui->userPageContainer);
            QVBoxLayout* newLayout = new QVBoxLayout(ui->userPageContainer);
            newLayout->addWidget(userPageWidget);
            ui->userPageContainer->setLayout(newLayout);
            ui->userPageContainer->setVisible(true);

            ui->stackedWidget->setCurrentIndex(1);

            currentUser = id;

            connect(userPageWidget, &User_page::logoutRequested, this, [this]() {
                if (userPageWidget) {
                    delete userPageWidget;
                    userPageWidget = nullptr;
                }

                if (ui->userPageContainer->layout()) {
                    QLayoutItem* item;
                    while ((item = ui->userPageContainer->layout()->takeAt(0)) != nullptr) {
                        if (item->widget()) delete item->widget();
                        delete item;
                    }
                    delete ui->userPageContainer->layout();
                }

                ui->userPageContainer->setVisible(false);
                currentUser.clear();
                resetLoginUI();
            });

            connect(userPageWidget, &User_page::checkoutRequested, this, &MainWindow::showCheckoutPage);

        } else {
            ui->messageLabel->setText("Invalid User ID or Password");
        }
    }
}


void MainWindow::on_registerBtn_clicked()
{
    QString id = ui->idInput->text();
    QString password = ui->passwordInput->text();

    if (id.isEmpty() || password.isEmpty()) {
        ui->messageLabel->setText("User ID and Password cannot be empty");
        return;
    }

    if (userData.userExists(id.toStdString())) {
        ui->messageLabel->setText("User ID already exists");
        return;
    }

    userData.addUser(id.toStdString(), password.toStdString(), "user");
    ui->messageLabel->setText("Registration successful! Please log in.");

    ui->idInput->clear();
    ui->passwordInput->clear();
}

void MainWindow::on_showPasswordCheckbox_toggled(bool checked)
{
    if (checked) {
        ui->passwordInput->setEchoMode(QLineEdit::Normal);
    } else {
        ui->passwordInput->setEchoMode(QLineEdit::Password);
    }
}

void MainWindow::showCheckoutPage()
{
    QString username;

    // ✅ Safely retrieve username before cleanup
    if (userPageWidget) {
        username = userPageWidget->getUsername();
    }

    if (username.isEmpty()) {
        QMessageBox::critical(this, "Error", "No valid user session found.");
        resetLoginUI();
        return;
    }

    // ✅ Cleanup previous CheckoutPage if it exists
    if (checkoutPage) {
        ui->stackedWidget->removeWidget(checkoutPage);
        checkoutPage->deleteLater();
        checkoutPage = nullptr;
    }

    // ✅ Remove old UserPage widget from layout & memory
    if (userPageWidget) {
        QLayout* layout = ui->userPageContainer->layout();
        if (layout) {
            QLayoutItem* item;
            while ((item = layout->takeAt(0)) != nullptr) {
                if (item->widget()) {
                    item->widget()->deleteLater();
                }
                delete item;
            }
            delete layout;
        }

        userPageWidget->deleteLater();
        userPageWidget = nullptr;
    }

    // ✅ Create new CheckoutPage
    checkoutPage = new CheckoutPage(username, orderManager, inventory, this);
    ui->stackedWidget->addWidget(checkoutPage);
    ui->stackedWidget->setCurrentWidget(checkoutPage);

    // ✅ Connect backToUserPage signal
    connect(checkoutPage, &CheckoutPage::backToUserPage, this, [this, username]() {
        if (checkoutPage) {
            ui->stackedWidget->removeWidget(checkoutPage);
            checkoutPage->deleteLater();
            checkoutPage = nullptr;
        }

        // Create and show user page again
        userPageWidget = new User_page(username, inventory, orderManager, ui->userPageContainer);
        QVBoxLayout* layout = new QVBoxLayout(ui->userPageContainer);
        layout->addWidget(userPageWidget);
        ui->userPageContainer->setLayout(layout);
        ui->userPageContainer->setVisible(true);
        ui->stackedWidget->setCurrentIndex(1);

        connect(userPageWidget, &User_page::logoutRequested, this, [this]() {
            if (userPageWidget) {
                userPageWidget->deleteLater();
                userPageWidget = nullptr;
            }

            QLayout* layout = ui->userPageContainer->layout();
            if (layout) {
                QLayoutItem* item;
                while ((item = layout->takeAt(0)) != nullptr) {
                    if (item->widget()) item->widget()->deleteLater();
                    delete item;
                }
                delete layout;
            }

            ui->userPageContainer->setVisible(false);
            currentUser.clear();
            resetLoginUI();
        });

        connect(userPageWidget, &User_page::checkoutRequested, this, &MainWindow::showCheckoutPage);
    });

    connect(checkoutPage, &CheckoutPage::logoutRequested, this, [this]() {
        if (checkoutPage) {
            ui->stackedWidget->removeWidget(checkoutPage);
            checkoutPage->deleteLater();
            checkoutPage = nullptr;
        }
        currentUser.clear();
        resetLoginUI();
    });
}
