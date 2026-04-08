#ifndef DASHBOARDWINDOW_H
#define DASHBOARDWINDOW_H

#include "core/managers/FinanceTrackerBackend.h"

#include <QWidget>

class QLabel;
class QTableWidget;
class QListWidget;

class DashboardWindow : public QWidget {
    Q_OBJECT

public:
    explicit DashboardWindow(finsight::core::managers::FinanceTrackerBackend& backend,
                             const std::string& userId,
                             QWidget *parent = nullptr);

    void setUserId(const std::string& userId);
    void refreshData();

private:
    finsight::core::managers::FinanceTrackerBackend& backend_;
    std::string userId_;

    QLabel *incomeValueLabel;
    QLabel *expensesValueLabel;
    QLabel *savingsValueLabel;
    QLabel *savingsRateValueLabel;

    QTableWidget *recentTransactionsTable;
    QListWidget *topCategoriesList;
    QLabel *budgetHealthLabel;

    void setupUi();
    QWidget *createSummaryCard(const QString &title, QLabel *&valueLabel);
};

#endif
