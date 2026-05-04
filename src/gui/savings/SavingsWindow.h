#ifndef SAVINGSWINDOW_H
#define SAVINGSWINDOW_H

#include "core/managers/FinanceTrackerBackend.h"

#include <QWidget>
#include <string>

class QDateEdit;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QProgressBar;
class QPushButton;
class QTableWidget;

class SavingsWindow : public QWidget {
    Q_OBJECT

public:
    explicit SavingsWindow(finsight::core::managers::FinanceTrackerBackend& backend,
                           const std::string& userId,
                           QWidget* parent = nullptr);

    void setUserId(const std::string& userId);
    void refreshData();

signals:
    void dataChanged();

private:
    finsight::core::managers::FinanceTrackerBackend& backend_;
    std::string userId_;

    QLabel* balanceValue {nullptr};
    QLabel* monthlySavedValue {nullptr};
    QLabel* monthlyTargetValue {nullptr};
    QLabel* longTermTargetValue {nullptr};
    QLabel* targetDateValue {nullptr};
    QProgressBar* monthlyProgress {nullptr};

    QDoubleSpinBox* amountSpin {nullptr};
    QLineEdit* noteEdit {nullptr};
    QDateEdit* entryDateEdit {nullptr};
    QDoubleSpinBox* monthlyTargetSpin {nullptr};
    QDoubleSpinBox* longTermTargetSpin {nullptr};
    QDateEdit* targetDateEdit {nullptr};
    QTableWidget* entriesTable {nullptr};
    QPushButton* deleteEntryButton {nullptr};

    void setupUi();
    void addDeposit();
    void addWithdrawal();
    void saveTargets();
    void deleteSelectedEntry();
    void clearEntryForm();
    std::string selectedEntryId() const;
};

#endif
