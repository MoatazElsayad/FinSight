#include "gui/ai/AIInsightsWindow.h"

#include <algorithm>
#include <map>

#include <QDate>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QComboBox>

using namespace finsight::core::models;

AIInsightsWindow::AIInsightsWindow(finsight::core::managers::FinanceTrackerBackend& backend,
                                   const std::string& userId,
                                   QWidget *parent)
    : QWidget(parent),
      backend_(backend),
      userId_(userId) {
    setupUi();
    refreshData();
}

void AIInsightsWindow::setUserId(const std::string& userId) {
    userId_ = userId;
}

void AIInsightsWindow::setupUi() {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(14);

    setStyleSheet(
        "AIInsightsWindow, QWidget { background-color: #0b1020; color: #e5e9f4; }"
        "QFrame { background-color: #141a27; border: 1px solid #2b3245; border-radius: 12px; }"
        "QLabel[muted='true'] { color: #9ca6bf; }"
        "QListWidget, QTextEdit, QComboBox {"
        "  background-color: #0f1527;"
        "  border: 1px solid #2b3245;"
        "  border-radius: 10px;"
        "  color: #e5e9f4;"
        "}"
        "QTabWidget::pane { border: 0; }"
        "QTabBar::tab {"
        "  background-color: #0f1527;"
        "  color: #98a4be;"
        "  border: 1px solid #2b3245;"
        "  border-top-left-radius: 8px;"
        "  border-top-right-radius: 8px;"
        "  padding: 8px 14px;"
        "  margin-right: 4px;"
        "}"
        "QTabBar::tab:selected { background-color: #243252; color: #eef3ff; }"
    );

    auto *title = new QLabel("AI Insights");
    title->setStyleSheet("font-size: 28px; font-weight: 700;");

    auto *subTitle = new QLabel("Summary intelligence and recommended actions.");
    subTitle->setProperty("muted", true);
    subTitle->setStyleSheet("font-size: 12px;");

    auto *headerLayout = new QHBoxLayout();
    auto *leftHead = new QVBoxLayout();
    leftHead->addWidget(title);
    leftHead->addWidget(subTitle);

    auto *controlsFrame = new QFrame();
    auto *controlsLayout = new QHBoxLayout(controlsFrame);
    controlsLayout->setContentsMargins(10, 8, 10, 8);

    periodSelector = new QComboBox();
    periodSelector->addItem("Monthly");
    periodSelector->addItem("Yearly");
    periodSelector->addItem("Overall");

    auto *refreshButton = new QPushButton("Refresh");
    refreshButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #253355;"
        "  border: 1px solid #4968a8;"
        "  border-radius: 8px;"
        "  color: #eef3ff;"
        "  padding: 6px 12px;"
        "}"
    );

    statusLabel = new QLabel("Analysis Complete");
    statusLabel->setStyleSheet(
        "background-color: #163625;"
        "color: #89f0b7;"
        "border-radius: 9px;"
        "padding: 5px 9px;"
        "font-weight: 600;"
    );

    controlsLayout->addWidget(new QLabel("Range:"));
    controlsLayout->addWidget(periodSelector);
    controlsLayout->addSpacing(10);
    controlsLayout->addWidget(statusLabel);
    controlsLayout->addSpacing(8);
    controlsLayout->addWidget(refreshButton);

    headerLayout->addLayout(leftHead);
    headerLayout->addStretch();
    headerLayout->addWidget(controlsFrame, 0, Qt::AlignTop);

    auto *tabs = new QTabWidget();

    auto *summaryTab = new QWidget();
    auto *summaryLayout = new QVBoxLayout(summaryTab);

    auto addMetric = [&](const QString& labelText, QLabel*& target) {
        auto *label = new QLabel(labelText);
        label->setProperty("muted", true);
        label->setStyleSheet("font-weight: 600;");
        target = new QLabel("-");
        target->setWordWrap(true);
        target->setStyleSheet("font-size: 14px;");
        summaryLayout->addWidget(label);
        summaryLayout->addWidget(target);
        summaryLayout->addSpacing(8);
    };

    addMetric("Key Win", keyWinValue);
    addMetric("Budget Performance", budgetPerformanceValue);

    auto *topActionsLabel = new QLabel("Top Actions");
    topActionsLabel->setProperty("muted", true);
    topActionsLabel->setStyleSheet("font-weight: 600;");
    topActionsList = new QListWidget();
    summaryLayout->addWidget(topActionsLabel);
    summaryLayout->addWidget(topActionsList);

    auto *chatTab = new QWidget();
    auto *chatLayout = new QVBoxLayout(chatTab);
    chatHistory = new QTextEdit();
    chatHistory->setReadOnly(true);
    chatHistory->setPlainText(
        "AI Chat\n"
        "• Ask: 'What changed most this month?'\n"
        "• Ask: 'How can I increase savings rate by 5%?'\n"
        "• Ask: 'Which budget is most at risk?'\n"
    );
    chatLayout->addWidget(chatHistory);

    tabs->addTab(summaryTab, "Summary");
    tabs->addTab(chatTab, "Chat");

    mainLayout->addLayout(headerLayout);
    mainLayout->addWidget(tabs, 1);

    connect(periodSelector, &QComboBox::currentIndexChanged, this, [this](int) { refreshData(); });
    connect(refreshButton, &QPushButton::clicked, this, [this]() {
        statusLabel->setText("Analysis Complete");
        refreshData();
    });
}

void AIInsightsWindow::applyAnalysis(double netSavings, double savingsRate, double largestCategorySpend) {
    if (netSavings >= 0.0) {
        keyWinValue->setText("Positive net cash flow maintained across the selected range.");
    } else {
        keyWinValue->setText("Expenses exceeded income. Reducing one major cost center can quickly recover balance.");
    }

    if (savingsRate >= 0.2) {
        budgetPerformanceValue->setText("Excellent trajectory: savings rate is above 20%.");
    } else if (savingsRate >= 0.1) {
        budgetPerformanceValue->setText("On track but improvable: move toward a 20% savings target.");
    } else {
        budgetPerformanceValue->setText("Budget pressure detected: raise savings by reducing the largest expense category.");
    }

    topActionsList->clear();
    topActionsList->addItem("Set a weekly cap on discretionary spending and track variance.");
    topActionsList->addItem("Automate a savings transfer immediately after each income event.");
    if (largestCategorySpend > 0.0) {
        topActionsList->addItem("Trim your top expense category by 10% next period.");
    } else {
        topActionsList->addItem("Start categorizing expenses to unlock personalized optimization tips.");
    }
}

void AIInsightsWindow::refreshData() {
    if (userId_.empty()) {
        statusLabel->setText("Awaiting Analysis");
        applyAnalysis(0.0, 0.0, 0.0);
        return;
    }

    const auto transactions = backend_.transactions().listTransactions(userId_);
    const QDate now = QDate::currentDate();

    double income = 0.0;
    double expenses = 0.0;
    std::map<std::string, double> categoryTotals;

    for (const auto& transaction : transactions) {
        bool inScope = true;
        if (periodSelector->currentText() == "Monthly") {
            inScope = transaction.date.year == now.year() && transaction.date.month == now.month();
        } else if (periodSelector->currentText() == "Yearly") {
            inScope = transaction.date.year == now.year();
        }

        if (!inScope) {
            continue;
        }

        if (transaction.type == TransactionType::Income) {
            income += transaction.amount;
        } else {
            expenses += transaction.amount;
            categoryTotals[transaction.categoryId] += transaction.amount;
        }
    }

    double largestCategorySpend = 0.0;
    for (const auto& [_, amount] : categoryTotals) {
        largestCategorySpend = std::max(largestCategorySpend, amount);
    }

    const double netSavings = income - expenses;
    const double savingsRate = income <= 0.0 ? 0.0 : netSavings / income;
    statusLabel->setText("Analysis Complete");
    applyAnalysis(netSavings, savingsRate, largestCategorySpend);
}
