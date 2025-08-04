#include "settingspage.h"
#include "ui_settingspage.h"

#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QStackedWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QAbstractItemView>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

SettingsPage::SettingsPage(const QString& username, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsPage),
    username(username)
{
    ui->setupUi(this);

    ui->labelUsername->setText("Username: " + username);

    // Adjust vertical spacing of layouts to 5
    if (QVBoxLayout* menuLayout = qobject_cast<QVBoxLayout*>(ui->pageMenu->layout())) {
        menuLayout->setSpacing(5);
    }

    if (QVBoxLayout* passwordLayout = qobject_cast<QVBoxLayout*>(ui->pageChangePassword->layout())) {
        passwordLayout->setSpacing(5);
    }

    if (QVBoxLayout* historyLayout = qobject_cast<QVBoxLayout*>(ui->pagePurchaseHistory->layout())) {
        historyLayout->setSpacing(5);
    }

    // Connect buttons with slots
    connect(ui->btnChangePassword, &QPushButton::clicked, this, &SettingsPage::onChangePasswordClicked);
    connect(ui->btnViewHistory, &QPushButton::clicked, this, &SettingsPage::onViewPurchaseHistoryClicked);
    connect(ui->btnBackPassword, &QPushButton::clicked, this, &SettingsPage::onBackToMenuFromPassword);
    connect(ui->btnBackHistory, &QPushButton::clicked, this, &SettingsPage::onBackToMenuFromHistory);
    connect(ui->btnSubmitPassword, &QPushButton::clicked, this, &SettingsPage::onSubmitPasswordChange);
    connect(ui->checkShowPassword, &QCheckBox::toggled, this, &SettingsPage::onShowPasswordToggled);
}

SettingsPage::~SettingsPage()
{
    delete ui;
}

void SettingsPage::onChangePasswordClicked()
{
    ui->lineOldPassword->clear();
    ui->lineNewPassword->clear();
    ui->lineConfirmPassword->clear();
    ui->checkShowPassword->setChecked(false);
    ui->stackedWidget->setCurrentWidget(ui->pageChangePassword);
}

void SettingsPage::onViewPurchaseHistoryClicked()
{
    qDebug() << "View Purchase History clicked for user:" << username;
    populatePurchaseHistoryTable();
    ui->stackedWidget->setCurrentWidget(ui->pagePurchaseHistory);
}

void SettingsPage::onBackToMenuFromPassword()
{
    ui->stackedWidget->setCurrentWidget(ui->pageMenu);
}

void SettingsPage::onBackToMenuFromHistory()
{
    ui->stackedWidget->setCurrentWidget(ui->pageMenu);
}

void SettingsPage::onSubmitPasswordChange()
{
    QString oldPass = ui->lineOldPassword->text();
    QString newPass = ui->lineNewPassword->text();
    QString confirmPass = ui->lineConfirmPassword->text();

    if (oldPass.isEmpty() || newPass.isEmpty() || confirmPass.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please fill all password fields.");
        return;
    }

    if (newPass != confirmPass) {
        QMessageBox::warning(this, "Input Error", "New password and confirm password do not match.");
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT password FROM users WHERE username = ?");
    query.addBindValue(username);
    if (!query.exec() || !query.next()) {
        QMessageBox::warning(this, "Error", "Failed to access user data.");
        return;
    }

    QString currentPassword = query.value(0).toString();
    if (currentPassword != oldPass) {
        QMessageBox::warning(this, "Error", "Old password is incorrect.");
        return;
    }

    QSqlQuery updateQuery;
    updateQuery.prepare("UPDATE users SET password = ? WHERE username = ?");
    updateQuery.addBindValue(newPass);
    updateQuery.addBindValue(username);

    if (!updateQuery.exec()) {
        QMessageBox::warning(this, "Error", "Failed to update password.");
        return;
    }

    QMessageBox::information(this, "Success", "Password updated successfully.");
    ui->stackedWidget->setCurrentWidget(ui->pageMenu);
}

void SettingsPage::onShowPasswordToggled(bool checked)
{
    QLineEdit::EchoMode mode = checked ? QLineEdit::Normal : QLineEdit::Password;
    ui->lineOldPassword->setEchoMode(mode);
    ui->lineNewPassword->setEchoMode(mode);
    ui->lineConfirmPassword->setEchoMode(mode);
}

void SettingsPage::setPurchaseHistory(const std::vector<PurchaseRecord>& purchases)
{
    purchaseHistory = purchases;
    if (ui->stackedWidget->currentWidget() == ui->pagePurchaseHistory) {
        populatePurchaseHistoryTable();
    }
}

void SettingsPage::populatePurchaseHistoryTable() {
    QSqlQuery q;
    q.prepare(R"(
        SELECT i.name, o.quantity, (i.price * o.quantity) as total, o.order_date
        FROM orders o
        JOIN inventory i ON o.itemId = i.id
        WHERE o.username = ? AND o.status = 'completed'
        ORDER BY o.order_date DESC
    )");
    q.addBindValue(username);
    if (!q.exec()) {
        QMessageBox::warning(this, "Error", "Failed to load purchase history.");
        qDebug() << "SQL Error:" << q.lastError().text();
        return;
    }

    int count = 0;
    while (q.next()) {
        ++count;
        PurchaseRecord rec;
        rec.productName = q.value(0).toString();
        rec.quantity = q.value(1).toInt();
        rec.amount = q.value(2).toDouble();
        purchaseHistory.push_back(rec);
    }
    qDebug() << "Purchase history records found:" << count;

    ui->tablePurchaseHistory->clearContents();
    ui->tablePurchaseHistory->setRowCount(static_cast<int>(purchaseHistory.size()));
    ui->tablePurchaseHistory->setColumnCount(3);
    ui->tablePurchaseHistory->setHorizontalHeaderLabels(QStringList() << "Product" << "Quantity" << "Amount (Rs.)");

    ui->tablePurchaseHistory->horizontalHeader()->setStretchLastSection(true);
    ui->tablePurchaseHistory->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tablePurchaseHistory->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablePurchaseHistory->setSelectionMode(QAbstractItemView::NoSelection);

    for (int i = 0; i < static_cast<int>(purchaseHistory.size()); ++i) {
        const PurchaseRecord& rec = purchaseHistory[i];
        ui->tablePurchaseHistory->setItem(i, 0, new QTableWidgetItem(rec.productName));
        ui->tablePurchaseHistory->setItem(i, 1, new QTableWidgetItem(QString::number(rec.quantity)));
        ui->tablePurchaseHistory->setItem(i, 2, new QTableWidgetItem(QString::number(rec.amount, 'f', 2)));
    }

    ui->tablePurchaseHistory->resizeColumnsToContents();
}
