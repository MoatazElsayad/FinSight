#ifndef BUDGETSWINDOW_H
#define BUDGETSWINDOW_H

#include "core/managers/FinanceTrackerBackend.h"

#include <QWidget>
#include <optional>

class QComboBox;
class QDoubleSpinBox;
class QTableWidget;
class QPushButton;
class QLabel;

class BudgetsWindow : public QWidget {
    Q_OBJECT

public:
    explicit BudgetsWindow(finsight::core::managers::FinanceTrackerBackend& backend,
                           const std::string& userId,
                           QWidget *parent = nullptr);

    void setUserId(const std::string& userId);
    void refreshData();

signals:
    void dataChanged();

private:
    finsight::core::managers::FinanceTrackerBackend& backend_;
    std::string userId_;

    QComboBox *monthCombo;
    QComboBox *categoryCombo;
    QDoubleSpinBox *amountSpin;

    QTableWidget *budgetsTable;

    QPushButton *addButton;
    QPushButton *editButton;
    QPushButton *deleteButton;
    QPushButton *clearButton;

    QLabel *totalBudgetLabel;
    QLabel *totalSpentLabel;
    QLabel *totalRemainingLabel;

    void setupUi();
    void populateCategories();
    void addBudgetRow(const QString &month,
                      const QString &category,
                      const QString& budgetId,
                      const QString& categoryId,
                      double budgeted,
                      double spent);
    void refreshSummary();
    void clearInputs();
    std::optional<finsight::core::models::Budget> selectedBudget() const;
    finsight::core::models::YearMonth selectedPeriod() const;

private slots:
    void onAddBudget();
    void onEditBudget();
    void onDeleteBudget();
    void onTableSelectionChanged();
};

#endif
