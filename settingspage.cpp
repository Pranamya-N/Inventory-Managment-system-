#include "settingspage.h"
#include "ui_settingspage.h"
#include <QMessageBox>

SettingsPage::SettingsPage(const QString& username, const QString& password, const QVector<QPair<QString, double>>& history, QWidget *parent)
    : QDialog(parent),
    ui(new Ui::SettingsPage),
    currentUsername(username),
    currentPassword(password),
    purchaseHistory(history)
{
    ui->setupUi(this);
    ui->labelUsername->setText(currentUsername);

    ui->lineEditCurrentPassword->setEchoMode(QLineEdit::Password);
    ui->lineEditNewPassword->setEchoMode(QLineEdit::Password);
    ui->lineEditConfirmPassword->setEchoMode(QLineEdit::Password);

    connect(ui->checkBoxShowPasswords, &QCheckBox::toggled, this, &SettingsPage::on_checkBoxShowPasswords_toggled);
    connect(ui->pushButtonChangePassword, &QPushButton::clicked, this, &SettingsPage::on_pushButtonChangePassword_clicked);
    connect(ui->pushButtonViewHistory, &QPushButton::clicked, this, &SettingsPage::on_pushButtonViewHistory_clicked);
    connect(ui->pushButtonBack, &QPushButton::clicked, this, &SettingsPage::on_pushButtonBack_clicked);
}

SettingsPage::~SettingsPage()
{
    delete ui;
}

void SettingsPage::setUsername(const QString& username)
{
    currentUsername = username;
    ui->labelUsername->setText(currentUsername);
}

void SettingsPage::clearPasswordFields()
{
    ui->lineEditCurrentPassword->clear();
    ui->lineEditNewPassword->clear();
    ui->lineEditConfirmPassword->clear();
}

void SettingsPage::on_checkBoxShowPasswords_toggled(bool checked)
{
    QLineEdit::EchoMode mode = checked ? QLineEdit::Normal : QLineEdit::Password;
    ui->lineEditCurrentPassword->setEchoMode(mode);
    ui->lineEditNewPassword->setEchoMode(mode);
    ui->lineEditConfirmPassword->setEchoMode(mode);
}

void SettingsPage::on_pushButtonChangePassword_clicked()
{
    QString currentPassInput = ui->lineEditCurrentPassword->text();
    QString newPass = ui->lineEditNewPassword->text();
    QString confirmPass = ui->lineEditConfirmPassword->text();

    if (currentPassInput != currentPassword) {
        QMessageBox::warning(this, "Error", "Current password is incorrect.");
        return;
    }

    if (newPass.isEmpty() || confirmPass.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter and confirm the new password.");
        return;
    }

    if (newPass != confirmPass) {
        QMessageBox::warning(this, "Mismatch", "New password and confirmation do not match.");
        return;
    }

    emit changePasswordRequested(currentPassInput, newPass);
    QMessageBox::information(this, "Success", "Password changed successfully.");
    clearPasswordFields();
}

void SettingsPage::on_pushButtonBack_clicked()
{
    this->close();
}

void SettingsPage::on_pushButtonViewHistory_clicked()
{
    populatePurchaseHistory();
}

void SettingsPage::populatePurchaseHistory()
{
    QString historyText = "Purchase History:\n\n";
    double total = 0.0;
    for (const auto& item : purchaseHistory) {
        historyText += QString("- %1 : ₹%2\n").arg(item.first).arg(item.second, 0, 'f', 2);
        total += item.second;
    }
    historyText += QString("\nTotal: ₹%1").arg(total, 0, 'f', 2);
    QMessageBox::information(this, "Purchase History", historyText);
}
