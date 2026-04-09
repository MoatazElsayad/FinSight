#ifndef DASHBOARDWINDOW_H
#define DASHBOARDWINDOW_H

#include "core/managers/FinanceTrackerBackend.h"

#include <QWidget>

class QLabel;
class QTableWidget;
class QListWidget;
class QComboBox;
class QPushButton;
class QButtonGroup;

class DashboardWindow : public QWidget {
    Q_OBJECT

public:
    explicit DashboardWindow(finsight::core::managers::FinanceTrackerBackend& backend,
                             const std::string& userId,
                             QWidget *parent = nullptr);

    void setUserId(const std::string& userId);
    void refreshData();

private:
    enum class TimeRange {
        Monthly,
        Yearly,
        Overall
    };

    finsight::core::managers::FinanceTrackerBackend& backend_;
    std::string userId_;

    TimeRange activeTimeRange_ {TimeRange::Monthly};

    QLabel *incomeValueLabel;
    QLabel *expensesValueLabel;
    QLabel *liquidCashValueLabel;
    QLabel *savingsRateValueLabel;

    QPushButton *monthlyFilterButton;
    QPushButton *yearlyFilterButton;
    QPushButton *overallFilterButton;
    QButtonGroup *timeFilterGroup;
    QComboBox *monthSelector;

    QTableWidget *recentTransactionsTable;
    QListWidget *topCategoriesList;
    QLabel *budgetHealthLabel;

    void setupUi();
    QWidget *createSummaryCard(const QString &title, QLabel *&valueLabel, const QString& accentColor);
    void configureMonthSelector();
    bool selectedYearMonth(finsight::core::models::YearMonth& period) const;
    bool isTransactionInScope(const finsight::core::models::Transaction& transaction,
                              const finsight::core::models::YearMonth& period) const;
};

#endif
