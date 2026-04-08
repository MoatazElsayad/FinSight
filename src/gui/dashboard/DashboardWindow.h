#ifndef DASHBOARDWINDOW_H
#define DASHBOARDWINDOW_H

#include <QWidget>

class QLabel;
class QTableWidget;
class QListWidget;

class DashboardWindow : public QWidget {
    Q_OBJECT

public:
    explicit DashboardWindow(QWidget *parent = nullptr);

private:
    QLabel *incomeValueLabel;
    QLabel *expensesValueLabel;
    QLabel *savingsValueLabel;
    QLabel *savingsRateValueLabel;

    QTableWidget *recentTransactionsTable;
    QListWidget *topCategoriesList;
    QLabel *budgetHealthLabel;

    void setupUi();
    void loadDummyData();
    QWidget *createSummaryCard(const QString &title, QLabel *&valueLabel);
};

#endif
