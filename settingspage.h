#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QDialog>
#include <vector>
#include "backend_classes.h"  // ✅ Include for PurchaseRecord

namespace Ui {
class SettingsPage;
}

class SettingsPage : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsPage(const QString& username, QWidget *parent = nullptr);
    ~SettingsPage();

    void setPurchaseHistory(const std::vector<PurchaseRecord>& purchases);

private slots:
    void onChangePasswordClicked();
    void onViewPurchaseHistoryClicked();
    void onBackToMenuFromPassword();
    void onBackToMenuFromHistory();
    void onSubmitPasswordChange();
    void onShowPasswordToggled(bool checked);

private:
    Ui::SettingsPage *ui;
    QString username;

    std::vector<PurchaseRecord> purchaseHistory;

    void populatePurchaseHistoryTable();
};

#endif // SETTINGSPAGE_H
