#ifndef PROFILEWINDOW_H
#define PROFILEWINDOW_H

#include "core/managers/FinanceTrackerBackend.h"

#include <QWidget>

class QLabel;
class QLineEdit;
class QComboBox;
class QPushButton;

class ProfileWindow : public QWidget {
    Q_OBJECT

public:
    explicit ProfileWindow(finsight::core::managers::FinanceTrackerBackend& backend,
                           const std::string& userId,
                           QWidget *parent = nullptr);

    void setUserId(const std::string& userId);
    void refreshData();

signals:
    void profileUpdated();

private:
    finsight::core::managers::FinanceTrackerBackend& backend_;
    std::string userId_;

    QLabel *emailValueLabel;
    QLabel *createdAtValueLabel;
    QLineEdit *fullNameEdit;
    QLineEdit *phoneEdit;
    QComboBox *genderCombo;
    QPushButton *saveButton;

    void setupUi();
    void onSaveProfile();
};

#endif
