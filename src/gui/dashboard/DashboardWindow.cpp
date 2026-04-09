#include "gui/dashboard/DashboardWindow.h"

#include <algorithm>
#include <map>
#include <vector>

#include <QAbstractItemView>
#include <QButtonGroup>
#include <QComboBox>
#include <QDate>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

using namespace finsight::core::models;

DashboardWindow::DashboardWindow(finsight::core::managers::FinanceTrackerBackend& backend,
                                 const std::string& userId,
                                 QWidget *parent)
    : QWidget(parent),
      backend_(backend),
      userId_(userId) {
    setupUi();
    configureMonthSelector();
    refreshData();
}

void DashboardWindow::setUserId(const std::string& userId) {
    userId_ = userId;
}

QWidget *DashboardWindow::createSummaryCard(const QString &title,
                                            QLabel *&valueLabel,
                                            const QString& accentColor) {
    auto *card = new QFrame();
    card->setObjectName("summaryCard");
    card->setStyleSheet(
        "QFrame#summaryCard {"
        "  background-color: #141a27;"
        "  border: 1px solid #2b3245;"
        "  border-radius: 14px;"
        "}"
    );

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(18, 16, 18, 16);

    auto *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #9ca6bf;");

    valueLabel = new QLabel("$0.00");
    valueLabel->setStyleSheet(QString("font-size: 24px; font-weight: 700; color: %1;").arg(accentColor));

    layout->addWidget(titleLabel);
    layout->addSpacing(10);
    layout->addWidget(valueLabel);
    layout->addStretch();

    return card;
}

void DashboardWindow::configureMonthSelector() {
    monthSelector->clear();
    QDate current = QDate::currentDate();
    current = QDate(current.year(), current.month(), 1);

    for (int index = 0; index < 36; ++index) {
        const QDate month = current.addMonths(-index);
        monthSelector->addItem(month.toString("MMM yyyy"), month);
    }
}

bool DashboardWindow::selectedYearMonth(YearMonth& period) const {
    const QVariant data = monthSelector->currentData();
    const QDate month = data.toDate();
    if (!month.isValid()) {
        return false;
    }
    period = YearMonth {month.year(), month.month()};
    return true;
}

bool DashboardWindow::isTransactionInScope(const Transaction& transaction,
                                           const YearMonth& period) const {
    if (activeTimeRange_ == TimeRange::Overall) {
        return true;
    }
    if (activeTimeRange_ == TimeRange::Yearly) {
        return transaction.date.year == period.year;
    }
    return inMonth(transaction.date, period);
}

void DashboardWindow::setupUi() {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(14);

    setStyleSheet(
        "DashboardWindow, QWidget {"
        "  background-color: #0b1020;"
        "  color: #e5e9f4;"
        "}"
        "QGroupBox {"
        "  border: 1px solid #2b3245;"
        "  border-radius: 14px;"
        "  margin-top: 12px;"
        "  padding: 12px;"
        "  background-color: #141a27;"
        "  color: #e5e9f4;"
        "  font-weight: 600;"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  left: 12px;"
        "  padding: 0 6px;"
        "  color: #a6afc2;"
        "}"
        "QTableWidget, QListWidget, QComboBox {"
        "  background-color: #0f1527;"
        "  border: 1px solid #2b3245;"
        "  border-radius: 10px;"
        "  color: #e5e9f4;"
        "}"
        "QHeaderView::section {"
        "  background-color: #171f34;"
        "  color: #aab2c5;"
        "  border: 0;"
        "  padding: 6px;"
        "}"
    );

    auto *topBar = new QHBoxLayout();
    auto *titleLabel = new QLabel("Dashboard");
    titleLabel->setStyleSheet("font-size: 28px; font-weight: 700;");

    auto *subtitleLabel = new QLabel("Track outcomes, monitor trends, and act with confidence.");
    subtitleLabel->setStyleSheet("font-size: 12px; color: #8d97ac;");

    auto *headerColumn = new QVBoxLayout();
    headerColumn->addWidget(titleLabel);
    headerColumn->addWidget(subtitleLabel);

    auto *filterContainer = new QFrame();
    filterContainer->setStyleSheet(
        "QFrame {"
        "  background-color: #141a27;"
        "  border: 1px solid #2b3245;"
        "  border-radius: 12px;"
        "}"
    );
    auto *filterLayout = new QHBoxLayout(filterContainer);
    filterLayout->setContentsMargins(10, 8, 10, 8);
    filterLayout->setSpacing(8);

    auto makeFilterButton = [](const QString& text) {
        auto *button = new QPushButton(text);
        button->setCheckable(true);
        button->setStyleSheet(
            "QPushButton {"
            "  background-color: #0f1527;"
            "  color: #9ca6bf;"
            "  border: 1px solid #2b3245;"
            "  border-radius: 8px;"
            "  padding: 6px 10px;"
            "}"
            "QPushButton:checked {"
            "  background-color: #253355;"
            "  color: #e8eeff;"
            "  border-color: #4968a8;"
            "}"
        );
        return button;
    };

    monthlyFilterButton = makeFilterButton("Monthly");
    yearlyFilterButton = makeFilterButton("Yearly");
    overallFilterButton = makeFilterButton("Overall");

    timeFilterGroup = new QButtonGroup(this);
    timeFilterGroup->setExclusive(true);
    timeFilterGroup->addButton(monthlyFilterButton);
    timeFilterGroup->addButton(yearlyFilterButton);
    timeFilterGroup->addButton(overallFilterButton);
    monthlyFilterButton->setChecked(true);

    monthSelector = new QComboBox();
    monthSelector->setMinimumWidth(130);

    filterLayout->addWidget(monthlyFilterButton);
    filterLayout->addWidget(yearlyFilterButton);
    filterLayout->addWidget(overallFilterButton);
    filterLayout->addSpacing(8);
    filterLayout->addWidget(new QLabel("Period:"));
    filterLayout->addWidget(monthSelector);

    topBar->addLayout(headerColumn);
    topBar->addStretch();
    topBar->addWidget(filterContainer, 0, Qt::AlignTop);
    mainLayout->addLayout(topBar);

    auto *summaryLayout = new QGridLayout();
    summaryLayout->setHorizontalSpacing(12);
    summaryLayout->setVerticalSpacing(12);
    summaryLayout->addWidget(createSummaryCard("Total Income", incomeValueLabel, "#8CF4B8"), 0, 0);
    summaryLayout->addWidget(createSummaryCard("Total Expenses", expensesValueLabel, "#FF9191"), 0, 1);
    summaryLayout->addWidget(createSummaryCard("Liquid Cash", liquidCashValueLabel, "#8CC6FF"), 0, 2);
    summaryLayout->addWidget(createSummaryCard("Savings Rate", savingsRateValueLabel, "#D5B4FF"), 0, 3);
    mainLayout->addLayout(summaryLayout);

    auto *reportsBox = new QGroupBox("Reports");
    auto *reportsLayout = new QVBoxLayout(reportsBox);

    auto *recentTitle = new QLabel("Recent Transactions");
    recentTitle->setStyleSheet("font-weight: 600; color: #c8d0e3;");

    recentTransactionsTable = new QTableWidget();
    recentTransactionsTable->setColumnCount(4);
    recentTransactionsTable->setHorizontalHeaderLabels({"Date", "Title", "Type", "Amount"});
    recentTransactionsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    recentTransactionsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    recentTransactionsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    recentTransactionsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    auto *categoriesTitle = new QLabel("Top Spending Categories");
    categoriesTitle->setStyleSheet("font-weight: 600; color: #c8d0e3;");
    topCategoriesList = new QListWidget();

    auto *budgetHealthTitle = new QLabel("Budget Performance");
    budgetHealthTitle->setStyleSheet("font-weight: 600; color: #c8d0e3;");

    budgetHealthLabel = new QLabel("No data");
    budgetHealthLabel->setWordWrap(true);
    budgetHealthLabel->setStyleSheet(
        "background-color: #0f1527;"
        "border: 1px solid #2b3245;"
        "border-radius: 10px;"
        "padding: 10px;"
        "color: #e1e8fa;"
    );

    reportsLayout->addWidget(recentTitle);
    reportsLayout->addWidget(recentTransactionsTable, 2);
    reportsLayout->addWidget(categoriesTitle);
    reportsLayout->addWidget(topCategoriesList, 1);
    reportsLayout->addWidget(budgetHealthTitle);
    reportsLayout->addWidget(budgetHealthLabel);

    mainLayout->addWidget(reportsBox, 1);

    connect(monthlyFilterButton, &QPushButton::clicked, this, [this]() {
        activeTimeRange_ = TimeRange::Monthly;
        monthSelector->setEnabled(true);
        refreshData();
    });
    connect(yearlyFilterButton, &QPushButton::clicked, this, [this]() {
        activeTimeRange_ = TimeRange::Yearly;
        monthSelector->setEnabled(true);
        refreshData();
    });
    connect(overallFilterButton, &QPushButton::clicked, this, [this]() {
        activeTimeRange_ = TimeRange::Overall;
        monthSelector->setEnabled(false);
        refreshData();
    });
    connect(monthSelector, &QComboBox::currentIndexChanged, this, [this](int) {
        refreshData();
    });
}

void DashboardWindow::refreshData() {
    if (userId_.empty()) {
        incomeValueLabel->setText("$0.00");
        expensesValueLabel->setText("$0.00");
        liquidCashValueLabel->setText("$0.00");
        savingsRateValueLabel->setText("0.00%");
        recentTransactionsTable->setRowCount(0);
        topCategoriesList->clear();
        budgetHealthLabel->setText("No user is signed in.");
        return;
    }

    YearMonth period {};
    if (!selectedYearMonth(period)) {
        const QDate today = QDate::currentDate();
        period = YearMonth {today.year(), today.month()};
    }

    double income = 0.0;
    double expenses = 0.0;
    std::map<std::string, double> expenseByCategory;
    std::vector<Transaction> scopedTransactions;

    const auto transactions = backend_.transactions().listTransactions(userId_);
    for (const auto& transaction : transactions) {
        if (!isTransactionInScope(transaction, period)) {
            continue;
        }

        scopedTransactions.push_back(transaction);
        if (transaction.type == TransactionType::Income) {
            income += transaction.amount;
        } else {
            expenses += transaction.amount;
            expenseByCategory[transaction.categoryId] += transaction.amount;
        }
    }

    std::sort(scopedTransactions.begin(), scopedTransactions.end(), [](const auto& left, const auto& right) {
        return left.date > right.date;
    });

    const double netSavings = income - expenses;
    const double savingsRate = income <= 0.0 ? 0.0 : netSavings / income;

    incomeValueLabel->setText("$" + QString::number(income, 'f', 2));
    expensesValueLabel->setText("$" + QString::number(expenses, 'f', 2));
    liquidCashValueLabel->setText("$" + QString::number(netSavings, 'f', 2));
    savingsRateValueLabel->setText(QString::number(savingsRate * 100.0, 'f', 2) + "%");

    recentTransactionsTable->setRowCount(0);
    const int recentCount = std::min<int>(static_cast<int>(scopedTransactions.size()), 6);
    recentTransactionsTable->setRowCount(recentCount);
    for (int index = 0; index < recentCount; ++index) {
        const auto& transaction = scopedTransactions[static_cast<size_t>(index)];
        recentTransactionsTable->setItem(index, 0, new QTableWidgetItem(QString::fromStdString(transaction.date.toString())));
        recentTransactionsTable->setItem(index, 1, new QTableWidgetItem(QString::fromStdString(transaction.title)));
        recentTransactionsTable->setItem(index, 2, new QTableWidgetItem(
            transaction.type == TransactionType::Income ? "Income" : "Expense"));
        recentTransactionsTable->setItem(index, 3, new QTableWidgetItem("$" + QString::number(transaction.amount, 'f', 2)));
    }

    std::vector<std::pair<std::string, double>> sortedCategories(expenseByCategory.begin(), expenseByCategory.end());
    std::sort(sortedCategories.begin(), sortedCategories.end(), [](const auto& left, const auto& right) {
        return left.second > right.second;
    });

    topCategoriesList->clear();
    for (const auto& [categoryId, amount] : sortedCategories) {
        const auto& category = backend_.transactions().requireCategory(categoryId);
        topCategoriesList->addItem(QString::fromStdString(category.name) + " • $" + QString::number(amount, 'f', 2));
    }
    if (sortedCategories.empty()) {
        topCategoriesList->addItem("No expense activity for selected period.");
    }

    QStringList budgetLines;
    if (activeTimeRange_ == TimeRange::Monthly) {
        const auto statuses = backend_.budgets().summarizeBudgets(userId_, period, backend_.transactions());
        for (const auto& status : statuses) {
            const auto& category = backend_.transactions().requireCategory(status.budget.categoryId);
            QString line = QString("%1: $%2 / $%3")
                               .arg(QString::fromStdString(category.name))
                               .arg(QString::number(status.spent, 'f', 2))
                               .arg(QString::number(status.budget.limit, 'f', 2));
            if (status.overspent) {
                line += " (Over budget)";
            }
            budgetLines << line;
        }
    } else {
        budgetLines << "Budget breakdown is available in Monthly view.";
    }
    budgetHealthLabel->setText(budgetLines.isEmpty() ? "No budget data available." : budgetLines.join("\n"));
}
