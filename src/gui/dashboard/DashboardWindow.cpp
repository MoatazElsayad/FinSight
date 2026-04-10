#include "gui/dashboard/DashboardWindow.h"

#include <algorithm>
#include <exception>
#include <map>
#include <vector>

#include <QtWidgets>

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
        "  background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "                                    stop:0 #1a2135, stop:1 #141a27);"
        "  border: 1px solid #2b3245;"
        "  border-radius: 14px;"
        "  border-top: 3px solid " + accentColor + ";"
        "}"
        "QFrame#summaryCard:hover {"
        "  background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "                                    stop:0 #1e2436, stop:1 #171f34);"
        "  border-color: #3a4155;"
        "}"
    );

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(18, 16, 18, 16);

    auto *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("font-size: 12px; font-weight: 500; color: #9ca6bf; letter-spacing: 0.5px;");

    valueLabel = new QLabel("$0.00");
    valueLabel->setStyleSheet(QString("font-size: 28px; font-weight: 700; color: %1; margin-top: 4px;").arg(accentColor));

    layout->addWidget(titleLabel);
    layout->addSpacing(8);
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
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(20);

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
        "  padding: 0 8px;"
        "  color: #a6afc2;"
        "}"
        "QTableWidget, QListWidget, QComboBox {"
        "  background-color: #0f1527;"
        "  border: 1px solid #2b3245;"
        "  border-radius: 10px;"
        "  color: #e5e9f4;"
        "  selection-background-color: #253355;"
        "}"
        "QTableWidget::item {"
        "  padding: 4px;"
        "  border-bottom: 1px solid #1e2436;"
        "}"
        "QTableWidget::item:selected {"
        "  background-color: #253355;"
        "}"
        "QHeaderView::section {"
        "  background-color: #171f34;"
        "  color: #aab2c5;"
        "  border: 0;"
        "  padding: 8px 6px;"
        "  font-weight: 600;"
        "  border-bottom: 2px solid #2b3245;"
        "}"
        "QScrollBar:vertical {"
        "  background-color: #0f1527;"
        "  width: 12px;"
        "  border-radius: 6px;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background-color: #2b3245;"
        "  border-radius: 6px;"
        "  min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "  background-color: #3a4155;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "  border: none;"
        "  background: none;"
        "}"
    );

    auto *topBar = new QHBoxLayout();
    auto *titleLabel = new QLabel("Dashboard");
    titleLabel->setStyleSheet(
        "font-size: 32px; "
        "font-weight: 700; "
        "color: #ffffff; "
        "text-shadow: 0 2px 4px rgba(0,0,0,0.3);"
    );

    auto *subtitleLabel = new QLabel("Track outcomes, monitor trends, and act with confidence.");
    subtitleLabel->setStyleSheet(
        "font-size: 13px; "
        "color: #8d97ac; "
        "font-weight: 400; "
        "margin-top: 2px;"
    );

    auto *headerColumn = new QVBoxLayout();
    headerColumn->addWidget(titleLabel);
    headerColumn->addWidget(subtitleLabel);

    auto *filterContainer = new QFrame();
    filterContainer->setStyleSheet(
        "QFrame {"
        "  background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "                                    stop:0 #1a2135, stop:1 #141a27);"
        "  border: 1px solid #2b3245;"
        "  border-radius: 12px;"
        "  box-shadow: 0 2px 8px rgba(0,0,0,0.15);"
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
            "  padding: 8px 12px;"
            "  font-weight: 500;"
            "  font-size: 12px;"
            "  transition: all 0.2s ease;"
            "}"
            "QPushButton:hover {"
            "  background-color: #1a2135;"
            "  border-color: #3a4155;"
            "  color: #aab2c5;"
            "}"
            "QPushButton:checked {"
            "  background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
            "                                    stop:0 #4968a8, stop:1 #3a5490);"
            "  color: #ffffff;"
            "  border-color: #4968a8;"
            "  font-weight: 600;"
            "}"
            "QPushButton:checked:hover {"
            "  background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
            "                                    stop:0 #5a7bc0, stop:1 #4968a8);"
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
    summaryLayout->setHorizontalSpacing(16);
    summaryLayout->setVerticalSpacing(16);
    summaryLayout->addWidget(createSummaryCard("Total Income", incomeValueLabel, "#8CF4B8"), 0, 0);
    summaryLayout->addWidget(createSummaryCard("Total Expenses", expensesValueLabel, "#FF9191"), 0, 1);
    summaryLayout->addWidget(createSummaryCard("Liquid Cash", liquidCashValueLabel, "#8CC6FF"), 0, 2);
    summaryLayout->addWidget(createSummaryCard("Savings Rate", savingsRateValueLabel, "#D5B4FF"), 0, 3);
    mainLayout->addLayout(summaryLayout);

    // AI Summary Section
    auto *aiSummaryBox = new QGroupBox("AI Financial Insights");
    aiSummaryBox->setStyleSheet(
        "QGroupBox {"
        "  border: 2px solid #5b8cff;"
        "  border-radius: 16px;"
        "  margin-top: 12px;"
        "  padding: 16px;"
        "  background-color: #0f1a33;"
        "  color: #e5e9f4;"
        "  font-weight: 600;"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  left: 16px;"
        "  padding: 0 8px;"
        "  color: #5b8cff;"
        "  font-size: 16px;"
        "}"
    );

    auto *aiSummaryLayout = new QVBoxLayout(aiSummaryBox);
    aiSummaryLayout->setSpacing(12);

    // AI Summary Header with Generate Button
    auto *aiHeaderLayout = new QHBoxLayout();
    aiSummaryTitle = new QLabel("AI-Powered Financial Analysis");
    aiSummaryTitle->setStyleSheet("font-size: 14px; font-weight: 600; color: #e5e9f4;");

    generateAISummaryButton = new QPushButton("Generate AI Summary");
    generateAISummaryButton->setStyleSheet(
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "               stop:0 #5b8cff, stop:1 #4a7ae6);"
        "  color: #ffffff;"
        "  border: none;"
        "  border-radius: 8px;"
        "  padding: 8px 16px;"
        "  font-weight: 600;"
        "  font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "               stop:0 #6b9cff, stop:1 #5a8af6);"
        "}"
        "QPushButton:pressed {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "               stop:0 #4a7ae6, stop:1 #3a6ad6);"
        "}"
        "QPushButton:disabled {"
        "  background-color: #2a4080;"
        "  color: #6b7a94;"
        "}"
    );
    generateAISummaryButton->setMinimumWidth(140);

    aiStatusLabel = new QLabel("");
    aiStatusLabel->setStyleSheet("font-size: 11px; color: #8fa3bf; font-style: italic;");

    aiHeaderLayout->addWidget(aiSummaryTitle);
    aiHeaderLayout->addStretch();
    aiHeaderLayout->addWidget(aiStatusLabel);
    aiHeaderLayout->addWidget(generateAISummaryButton);

    aiSummaryLayout->addLayout(aiHeaderLayout);

    // AI Summary Content
    aiSummaryText = new QTextEdit();
    aiSummaryText->setReadOnly(true);
    aiSummaryText->setMaximumHeight(120);
    aiSummaryText->setStyleSheet(
        "QTextEdit {"
        "  background-color: #1a2f5a;"
        "  border: 1px solid #2a4080;"
        "  border-radius: 10px;"
        "  color: #e1e8fa;"
        "  padding: 12px;"
        "  font-size: 12px;"
        "  line-height: 1.5;"
        "  selection-background-color: #5b8cff;"
        "}"
    );
    aiSummaryText->setPlaceholderText("Click 'Generate AI Summary' to get personalized financial insights and recommendations based on your spending patterns, budget performance, and financial goals.");

    aiSummaryLayout->addWidget(aiSummaryText);

    // AI Recommendations
    auto *recommendationsTitle = new QLabel("💡 AI Recommendations");
    recommendationsTitle->setStyleSheet("font-size: 13px; font-weight: 600; color: #5b8cff; margin-top: 8px;");

    aiRecommendationsList = new QListWidget();
    aiRecommendationsList->setMaximumHeight(120);
    aiRecommendationsList->setStyleSheet(
        "QListWidget {"
        "  background-color: #1a2f5a;"
        "  border: 1px solid #2a4080;"
        "  border-radius: 10px;"
        "  color: #e1e8fa;"
        "  font-size: 12px;"
        "  padding: 8px;"
        "}"
        "QListWidget::item {"
        "  padding: 8px;"
        "  border-bottom: 1px solid #233d72;"
        "  background-color: transparent;"
        "}"
        "QListWidget::item:hover {"
        "  background-color: #233d72;"
        "}"
        "QListWidget::item:selected {"
        "  background-color: #2e5aa6;"
        "  color: #ffffff;"
        "}"
    );

    aiSummaryLayout->addWidget(recommendationsTitle);
    aiSummaryLayout->addWidget(aiRecommendationsList);

    mainLayout->addWidget(aiSummaryBox);

    auto *reportsBox = new QGroupBox("Reports");
    reportsBox->setStyleSheet(
        "QGroupBox {"
        "  border: 1px solid #2a4080;"
        "  border-radius: 14px;"
        "  margin-top: 12px;"
        "  padding: 16px;"
        "  background-color: #0f1a33;"
        "  color: #e5e9f4;"
        "  font-weight: 600;"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  left: 12px;"
        "  padding: 0 8px;"
        "  color: #5b8cff;"
        "}"
    );
    auto *reportsLayout = new QVBoxLayout(reportsBox);

    auto *recentTitle = new QLabel("📊 Recent Transactions");
    recentTitle->setStyleSheet("font-weight: 700; color: #5b8cff; font-size: 14px; margin-bottom: 8px;");

    recentTransactionsTable = new QTableWidget();
    recentTransactionsTable->setColumnCount(4);
    recentTransactionsTable->setHorizontalHeaderLabels({"Date", "Title", "Type", "Amount"});
    recentTransactionsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    recentTransactionsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    recentTransactionsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    recentTransactionsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    recentTransactionsTable->setStyleSheet(
        "QTableWidget, QListWidget {"
        "  background-color: #1a2f5a;"
        "  border: 1px solid #2a4080;"
        "  border-radius: 10px;"
        "  color: #e5e9f4;"
        "  selection-background-color: #2e5aa6;"
        "}"
        "QTableWidget::item {"
        "  padding: 8px;"
        "  border-bottom: 1px solid #233d72;"
        "}"
        "QTableWidget::item:selected {"
        "  background-color: #2e5aa6;"
        "}"
        "QHeaderView::section {"
        "  background-color: #0f1a33;"
        "  color: #5b8cff;"
        "  border: 0;"
        "  padding: 8px 6px;"
        "  font-weight: 700;"
        "  border-bottom: 2px solid #2a4080;"
        "}"
    );

    auto *categoriesTitle = new QLabel("🏷️ Top Spending Categories");
    categoriesTitle->setStyleSheet("font-weight: 700; color: #5b8cff; font-size: 14px; margin-top: 16px; margin-bottom: 8px;");
    topCategoriesList = new QListWidget();
    topCategoriesList->setStyleSheet(
        "QListWidget {"
        "  background-color: #1a2f5a;"
        "  border: 1px solid #2a4080;"
        "  border-radius: 10px;"
        "  color: #e5e9f4;"
        "  font-size: 12px;"
        "  padding: 8px;"
        "}"
        "QListWidget::item {"
        "  padding: 8px;"
        "  border-bottom: 1px solid #233d72;"
        "  background-color: transparent;"
        "}"
        "QListWidget::item:hover {"
        "  background-color: #233d72;"
        "}"
        "QListWidget::item:selected {"
        "  background-color: #2e5aa6;"
        "}"
    );

    auto *budgetHealthTitle = new QLabel("💰 Budget Performance");
    budgetHealthTitle->setStyleSheet("font-weight: 700; color: #5b8cff; font-size: 14px; margin-top: 16px; margin-bottom: 8px;");

    budgetHealthLabel = new QLabel("No data");
    budgetHealthLabel->setWordWrap(true);
    budgetHealthLabel->setStyleSheet(
        "background-color: #1a2f5a;"
        "border: 1px solid #2a4080;"
        "border-radius: 10px;"
        "padding: 12px;"
        "color: #e1e8fa;"
        "line-height: 1.5;"
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

    // AI Summary connections
    connect(generateAISummaryButton, &QPushButton::clicked, this, &DashboardWindow::generateAISummary);
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

        // Clear AI summary for logged out user
        aiSummaryText->clear();
        aiRecommendationsList->clear();
        aiStatusLabel->setText("");
        generateAISummaryButton->setEnabled(false);

        return;
    }

    generateAISummaryButton->setEnabled(true);

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

void DashboardWindow::generateAISummary() {
    if (userId_.empty()) {
        aiStatusLabel->setText("No user signed in");
        return;
    }

    // Disable button and show loading state
    generateAISummaryButton->setEnabled(false);
    generateAISummaryButton->setText("Analyzing...");
    generateAISummaryButton->setStyleSheet(
        "QPushButton {"
        "  background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "                                    stop:0 #2d4270, stop:1 #1e2436);"
        "  color: #6b7688;"
        "  border: none;"
        "  border-radius: 8px;"
        "  padding: 8px 16px;"
        "  font-weight: 600;"
        "  font-size: 12px;"
        "}"
    );

    aiStatusLabel->setText("🔄 Analyzing your financial data...");
    aiStatusLabel->setStyleSheet("font-size: 11px; color: #8cc6ff; font-style: italic; font-weight: 500;");
    aiSummaryText->setPlaceholderText("Generating personalized insights...");
    aiSummaryText->clear();
    aiRecommendationsList->clear();

    // Get current period
    YearMonth period {};
    if (!selectedYearMonth(period)) {
        const QDate today = QDate::currentDate();
        period = YearMonth {today.year(), today.month()};
    }

    try {
        // Generate AI insight using the backend
        const auto insight = backend_.ai().generateDashboardInsight(
            userId_,
            period,
            backend_.analytics(),
            backend_.transactions(),
            backend_.budgets(),
            backend_.savings(),
            backend_.goals()
        );

        // Update UI with results
        aiSummaryText->setPlainText(QString::fromStdString(insight.summary));

        aiRecommendationsList->clear();
        for (const auto& recommendation : insight.recommendations) {
            aiRecommendationsList->addItem("💡 " + QString::fromStdString(recommendation));
        }

        aiStatusLabel->setText("✅ Analysis complete");
        aiStatusLabel->setStyleSheet("font-size: 11px; color: #8cf4b8; font-style: italic; font-weight: 500;");

    } catch (const std::exception& e) {
        aiSummaryText->setPlainText("Unable to generate AI summary. Please check your API configuration and ensure you have set up your OpenRouter API key in the .env file.");
        aiStatusLabel->setText("❌ Error: " + QString::fromStdString(e.what()));
        aiStatusLabel->setStyleSheet("font-size: 11px; color: #ff9191; font-style: italic; font-weight: 500;");
    }

    // Re-enable button and restore style
    generateAISummaryButton->setEnabled(true);
    generateAISummaryButton->setText("Generate AI Summary");
    generateAISummaryButton->setStyleSheet(
        "QPushButton {"
        "  background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "                                    stop:0 #4968a8, stop:1 #3a5490);"
        "  color: #ffffff;"
        "  border: none;"
        "  border-radius: 8px;"
        "  padding: 8px 16px;"
        "  font-weight: 600;"
        "  font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "  background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "                                    stop:0 #5a7bc0, stop:1 #4968a8);"
        "}"
        "QPushButton:pressed {"
        "  background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "                                    stop:0 #3a5490, stop:1 #2d4270);"
        "}"
    );
}
