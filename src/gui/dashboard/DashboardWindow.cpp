#include "gui/dashboard/DashboardWindow.h"

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

DashboardWindow::DashboardWindow(QWidget *parent) : QWidget(parent) {
    setupUi();
    loadDummyData();
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

void DashboardWindow::loadDummyData() {
    incomeValueLabel->setText("12,000.00");
    expensesValueLabel->setText("3,140.00");
    savingsValueLabel->setText("8,860.00");
    savingsRateValueLabel->setText("73.83%");

    recentTransactionsTable->setRowCount(4);

    recentTransactionsTable->setItem(0, 0, new QTableWidgetItem("2026-04-04"));
    recentTransactionsTable->setItem(0, 1, new QTableWidgetItem("Electricity Bill"));
    recentTransactionsTable->setItem(0, 2, new QTableWidgetItem("Expense"));
    recentTransactionsTable->setItem(0, 3, new QTableWidgetItem("400.00"));

    recentTransactionsTable->setItem(1, 0, new QTableWidgetItem("2026-04-03"));
    recentTransactionsTable->setItem(1, 1, new QTableWidgetItem("Monthly Salary"));
    recentTransactionsTable->setItem(1, 2, new QTableWidgetItem("Income"));
    recentTransactionsTable->setItem(1, 3, new QTableWidgetItem("12000.00"));

    recentTransactionsTable->setItem(2, 0, new QTableWidgetItem("2026-04-02"));
    recentTransactionsTable->setItem(2, 1, new QTableWidgetItem("Uber"));
    recentTransactionsTable->setItem(2, 2, new QTableWidgetItem("Expense"));
    recentTransactionsTable->setItem(2, 3, new QTableWidgetItem("90.00"));

    recentTransactionsTable->setItem(3, 0, new QTableWidgetItem("2026-04-01"));
    recentTransactionsTable->setItem(3, 1, new QTableWidgetItem("Grocery Store"));
    recentTransactionsTable->setItem(3, 2, new QTableWidgetItem("Expense"));
    recentTransactionsTable->setItem(3, 3, new QTableWidgetItem("250.00"));

    topCategoriesList->addItem("Bills - 1450.00");
    topCategoriesList->addItem("Food - 900.00");
    topCategoriesList->addItem("Entertainment - 500.00");
    topCategoriesList->addItem("Transport - 350.00");

    budgetHealthLabel->setText(
        "You are within budget in most categories. "
        "Bills and Food are currently the highest spending areas."
    );
}
