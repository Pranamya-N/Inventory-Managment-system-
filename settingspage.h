#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QDialog>
#include <QString>
#include <QVector>
#include <QPair>

namespace Ui {
class SettingsPage;
}

class SettingsPage : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsPage(const QString& username, const QString& password, const QVector<QPair<QString, double>>& history, QWidget *parent = nullptr);
    ~SettingsPage();
    void setUsername(const QString& username);

signals:
    void changePasswordRequested(const QString& currentPassword, const QString& newPassword);

private slots:
    void on_pushButtonChangePassword_clicked();
    void on_pushButtonBack_clicked();
    void on_checkBoxShowPasswords_toggled(bool checked);
    void on_pushButtonViewHistory_clicked();

private:
    Ui::SettingsPage *ui;
    QString currentUsername;
    QString currentPassword;
    QVector<QPair<QString, double>> purchaseHistory;

    void clearPasswordFields();
    void populatePurchaseHistory();
};

#endif // SETTINGSPAGE_H
