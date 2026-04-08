#include "gui/dashboard/DashboardWindow.h"

#include <algorithm>
#include <QDate>
#include <QStringList>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QFrame>
#include <QGroupBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QListWidget>
#include <QAbstractItemView>

using namespace finsight::core::models;

DashboardWindow::DashboardWindow(finsight::core::managers::FinanceTrackerBackend& backend,
                                 const std::string& userId,
                                 QWidget *parent)
    : QWidget(parent),
      backend_(backend),
      userId_(userId) {
    setupUi();
    refreshData();
}

void DashboardWindow::setUserId(const std::string& userId) {
    userId_ = userId;
}

QWidget *DashboardWindow::createSummaryCard(const QString &title, QLabel *&valueLabel) {
    auto *card = new QFrame();
    card->setFrameShape(QFrame::StyledPanel);
    card->setStyleSheet(
        "QFrame {"
        "  background-color: #f5f5f5;"
        "  border: 1px solid #dcdcdc;"
        "  border-radius: 8px;"
        "}"
    );

    auto *layout = new QVBoxLayout(card);

    auto *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #444;");

    valueLabel = new QLabel("0.00");
    valueLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: #111;");

    layout->addWidget(titleLabel);
    layout->addWidget(valueLabel);
    layout->addStretch();

    return card;
}

void DashboardWindow::setupUi() {
    auto *mainLayout = new QVBoxLayout(this);

    auto *titleLabel = new QLabel("Dashboard");
    titleLabel->setStyleSheet("font-size: 22px; font-weight: bold;");
    mainLayout->addWidget(titleLabel);

    auto *summaryLayout = new QGridLayout();

    summaryLayout->addWidget(createSummaryCard("Total Income", incomeValueLabel), 0, 0);
    summaryLayout->addWidget(createSummaryCard("Total Expenses", expensesValueLabel), 0, 1);
    summaryLayout->addWidget(createSummaryCard("Net Savings", savingsValueLabel), 1, 0);
    summaryLayout->addWidget(createSummaryCard("Savings Rate", savingsRateValueLabel), 1, 1);

    mainLayout->addLayout(summaryLayout);

    auto *bottomLayout = new QHBoxLayout();

    auto *recentBox = new QGroupBox("Recent Transactions");
    auto *recentLayout = new QVBoxLayout(recentBox);

    recentTransactionsTable = new QTableWidget();
    recentTransactionsTable->setColumnCount(4);
    recentTransactionsTable->setHorizontalHeaderLabels({"Date", "Title", "Type", "Amount"});
    recentTransactionsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    recentTransactionsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    recentTransactionsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    recentTransactionsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    recentLayout->addWidget(recentTransactionsTable);

    auto *insightsBox = new QGroupBox("Insights");
    auto *insightsLayout = new QVBoxLayout(insightsBox);

    auto *categoriesTitle = new QLabel("Top Spending Categories");
    categoriesTitle->setStyleSheet("font-weight: bold;");

    topCategoriesList = new QListWidget();

    auto *budgetHealthTitle = new QLabel("Budget Health");
    budgetHealthTitle->setStyleSheet("font-weight: bold; margin-top: 10px;");

    budgetHealthLabel = new QLabel("No data");
    budgetHealthLabel->setWordWrap(true);
    budgetHealthLabel->setStyleSheet(
        "background-color: #f5f5f5;"
        "border: 1px solid #dcdcdc;"
        "border-radius: 6px;"
        "padding: 8px;"
    );

    insightsLayout->addWidget(categoriesTitle);
    insightsLayout->addWidget(topCategoriesList);
    insightsLayout->addWidget(budgetHealthTitle);
    insightsLayout->addWidget(budgetHealthLabel);

    bottomLayout->addWidget(recentBox, 2);
    bottomLayout->addWidget(insightsBox, 1);

    mainLayout->addLayout(bottomLayout);
}

void DashboardWindow::refreshData() {
    if (userId_.empty()) {
        incomeValueLabel->setText("0.00");
        expensesValueLabel->setText("0.00");
        savingsValueLabel->setText("0.00");
        savingsRateValueLabel->setText("0.00%");
        recentTransactionsTable->setRowCount(0);
        topCategoriesList->clear();
        budgetHealthLabel->setText("No user is signed in.");
        return;
    }

    const QDate today = QDate::currentDate();
    const YearMonth period {today.year(), today.month()};
    const auto dashboard = backend_.analytics().buildDashboard(
        userId_,
        period,
        backend_.transactions(),
        backend_.budgets(),
        backend_.savings(),
        backend_.goals());

    incomeValueLabel->setText(QString::number(dashboard.overview.income, 'f', 2));
    expensesValueLabel->setText(QString::number(dashboard.overview.expenses, 'f', 2));
    savingsValueLabel->setText(QString::number(dashboard.overview.netSavings, 'f', 2));
    savingsRateValueLabel->setText(QString::number(dashboard.overview.savingsRate * 100.0, 'f', 2) + "%");

    recentTransactionsTable->setRowCount(0);
    const auto transactions = backend_.transactions().listTransactions(userId_);
    const int recentCount = std::min<int>(static_cast<int>(transactions.size()), 5);
    recentTransactionsTable->setRowCount(recentCount);
    for (int index = 0; index < recentCount; ++index) {
        const auto& transaction = transactions[static_cast<size_t>(index)];
        recentTransactionsTable->setItem(index, 0, new QTableWidgetItem(QString::fromStdString(transaction.date.toString())));
        recentTransactionsTable->setItem(index, 1, new QTableWidgetItem(QString::fromStdString(transaction.title)));
        recentTransactionsTable->setItem(index, 2, new QTableWidgetItem(
            transaction.type == TransactionType::Income ? "Income" : "Expense"));
        recentTransactionsTable->setItem(index, 3, new QTableWidgetItem(QString::number(transaction.amount, 'f', 2)));
    }

    topCategoriesList->clear();
    for (const auto& category : dashboard.topExpenseCategories) {
        topCategoriesList->addItem(QString::fromStdString(category.categoryName) +
                                   " - " +
                                   QString::number(category.amount, 'f', 2));
    }
    if (dashboard.topExpenseCategories.empty()) {
        topCategoriesList->addItem("No expense activity for this month.");
    }

    QStringList budgetLines;
    for (const auto& status : dashboard.budgetHealth) {
        const auto& category = backend_.transactions().requireCategory(status.budget.categoryId);
        QString line = QString("%1: spent %2 / %3")
                           .arg(QString::fromStdString(category.name))
                           .arg(QString::number(status.spent, 'f', 2))
                           .arg(QString::number(status.budget.limit, 'f', 2));
        if (status.overspent) {
            line += " (Over budget)";
        }
        budgetLines << line;
    }
    budgetHealthLabel->setText(budgetLines.isEmpty() ? "No budget data available." : budgetLines.join("\n"));
}
