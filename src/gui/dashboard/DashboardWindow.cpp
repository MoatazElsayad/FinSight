#include "gui/dashboard/DashboardWindow.h"

#include "core/models/AI.h"

#include <algorithm>
#include <exception>
#include <stdexcept>
#include <string>
#include <vector>

#include <QBrush>
#include <QColor>
#include <QtWidgets>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtConcurrent/QtConcurrentRun>
#else
#include <QtConcurrentRun>
#endif

#include <QAbstractItemView>
#include <QEasingCurve>
#include <QButtonGroup>
#include <QComboBox>
#include <QDate>
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QListWidget>
#include <QMetaObject>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QVBoxLayout>

using namespace finsight::core::models;

namespace {

QString displayModelName(const std::string& modelId) {
    QString q = QString::fromStdString(modelId);
    const int slash = q.lastIndexOf(QLatin1Char('/'));
    if (slash >= 0) {
        q = q.mid(slash + 1);
    }
    q.replace(QLatin1Char('-'), QLatin1Char(' '));
    q.replace(QLatin1Char(':'), QLatin1Char(' '));
    return q.trimmed();
}

}  // namespace

DashboardWindow::DashboardWindow(finsight::core::managers::FinanceTrackerBackend& backend,
                                 const std::string& userId,
                                 QWidget *parent)
    : QWidget(parent),
      backend_(backend),
      userId_(userId) {
    setupUi();
    configureMonthSelector();

    aiProgressWatchdog_.setParent(this);
    aiProgressWatchdog_.setSingleShot(true);
    connect(&aiProgressWatchdog_, &QTimer::timeout, this, &DashboardWindow::onAiProgressWatchdogTimeout);
    if (aiSparklesLabel != nullptr) {
        aiSparklesOpacity_ = new QGraphicsOpacityEffect(aiSparklesLabel);
        aiSparklesLabel->setGraphicsEffect(aiSparklesOpacity_);
        aiSparklesPulse_ = new QPropertyAnimation(aiSparklesOpacity_, "opacity", this);
        aiSparklesPulse_->setDuration(900);
        aiSparklesPulse_->setStartValue(0.35);
        aiSparklesPulse_->setEndValue(1.0);
        aiSparklesPulse_->setEasingCurve(QEasingCurve::InOutQuad);
        aiSparklesPulse_->setLoopCount(-1);
    }

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

    generateAISummaryButton = new QPushButton(QStringLiteral("✨ Generate Insights"));
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

    aiModelLabel = new QLabel("");
    aiModelLabel->setStyleSheet("font-size: 10px; color: #6b7a94; background-color: #1a2f5a; padding: 2px 8px; border-radius: 4px;");
    aiModelLabel->setVisible(false);

    aiHeaderLayout->addWidget(aiSummaryTitle);
    aiHeaderLayout->addStretch();
    aiHeaderLayout->addWidget(aiModelLabel);
    aiHeaderLayout->addWidget(aiStatusLabel);
    aiHeaderLayout->addWidget(generateAISummaryButton);

    aiSummaryLayout->addLayout(aiHeaderLayout);

    aiLoadingFrame = new QFrame();
    aiLoadingFrame->setVisible(false);
    aiLoadingFrame->setStyleSheet("QFrame { background-color: transparent; border: none; }");
    auto *loadingLayout = new QVBoxLayout(aiLoadingFrame);
    loadingLayout->setContentsMargins(8, 16, 8, 16);
    loadingLayout->setSpacing(10);
    aiSparklesLabel = new QLabel(QStringLiteral("✨"));
    aiSparklesLabel->setAlignment(Qt::AlignCenter);
    aiSparklesLabel->setStyleSheet("font-size: 42px; background: transparent;");
    aiLoadingSubLabel = new QLabel();
    aiLoadingSubLabel->setAlignment(Qt::AlignCenter);
    aiLoadingSubLabel->setWordWrap(true);
    aiLoadingSubLabel->setStyleSheet("font-size: 12px; color: #9bb7e8; padding: 0 12px;");
    loadingLayout->addStretch(1);
    loadingLayout->addWidget(aiSparklesLabel);
    loadingLayout->addWidget(aiLoadingSubLabel);
    loadingLayout->addStretch(1);
    aiSummaryLayout->addWidget(aiLoadingFrame);

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
    aiSummaryText->setPlaceholderText(
        QStringLiteral("Unlock AI insights for this period—spending patterns, budget health, and practical next steps."));

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

    auto *recentTitle = new QLabel(QStringLiteral("Latest transactions"));
    recentTitle->setStyleSheet(
        "font-size: 13px; font-weight: 600; color: #5b8cff; margin-top: 12px;");

    recentTransactionsTable = new QTableWidget();
    recentTransactionsTable->setColumnCount(5);
    recentTransactionsTable->setHorizontalHeaderLabels(
        {QStringLiteral("Date"),
         QStringLiteral("Title"),
         QStringLiteral("Type"),
         QStringLiteral("Category"),
         QStringLiteral("Amount")});
    recentTransactionsTable->verticalHeader()->setVisible(false);
    recentTransactionsTable->setShowGrid(false);
    recentTransactionsTable->setAlternatingRowColors(true);
    recentTransactionsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    recentTransactionsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    recentTransactionsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    recentTransactionsTable->setMinimumHeight(200);
    recentTransactionsTable->setMaximumHeight(320);
    {
        auto *header = recentTransactionsTable->horizontalHeader();
        header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        header->setSectionResizeMode(1, QHeaderView::Stretch);
        header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        header->setSectionResizeMode(3, QHeaderView::Stretch);
        header->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    }
    recentTransactionsTable->setStyleSheet(
        "QTableWidget {"
        "  background-color: #1a2f5a;"
        "  border: 1px solid #2a4080;"
        "  border-radius: 10px;"
        "  color: #e5e9f4;"
        "  selection-background-color: #2e5aa6;"
        "  alternate-background-color: #15284a;"
        "}"
        "QTableWidget::item {"
        "  padding: 8px 6px;"
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

    aiSummaryLayout->addWidget(recentTitle);
    aiSummaryLayout->addWidget(recentTransactionsTable);

    mainLayout->addWidget(aiSummaryBox, 1);

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
        cancelPendingAiSummaryUi();

        incomeValueLabel->setText("$0.00");
        expensesValueLabel->setText("$0.00");
        liquidCashValueLabel->setText("$0.00");
        savingsRateValueLabel->setText("0.00%");
        recentTransactionsTable->setRowCount(0);

        // Clear AI summary for logged out user
        aiSummaryText->clear();
        aiRecommendationsList->clear();
        aiStatusLabel->setText("");
        aiModelLabel->setVisible(false);
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

    const auto transactions = backend_.transactions().listTransactions(userId_);
    for (const auto& transaction : transactions) {
        if (!isTransactionInScope(transaction, period)) {
            continue;
        }

        if (transaction.type == TransactionType::Income) {
            income += transaction.amount;
        } else {
            expenses += transaction.amount;
        }
    }

    const double netSavings = income - expenses;
    const double savingsRate = income <= 0.0 ? 0.0 : netSavings / income;

    incomeValueLabel->setText("$" + QString::number(income, 'f', 2));
    expensesValueLabel->setText("$" + QString::number(expenses, 'f', 2));
    liquidCashValueLabel->setText("$" + QString::number(netSavings, 'f', 2));
    savingsRateValueLabel->setText(QString::number(savingsRate * 100.0, 'f', 2) + "%");

    std::vector<Transaction> latestAll = backend_.transactions().listTransactions(userId_);
    std::sort(latestAll.begin(), latestAll.end(), [](const auto& left, const auto& right) {
        return left.date > right.date;
    });
    const int recentCount = std::min<int>(static_cast<int>(latestAll.size()), 10);
    recentTransactionsTable->setRowCount(recentCount);
    for (int index = 0; index < recentCount; ++index) {
        const auto& transaction = latestAll[static_cast<size_t>(index)];
        recentTransactionsTable->setItem(index, 0,
            new QTableWidgetItem(QString::fromStdString(transaction.date.toString())));
        recentTransactionsTable->setItem(index, 1,
            new QTableWidgetItem(QString::fromStdString(transaction.title)));
        recentTransactionsTable->setItem(index, 2, new QTableWidgetItem(
            transaction.type == TransactionType::Income ? QStringLiteral("Income") : QStringLiteral("Expense")));

        QString categoryLabel = QStringLiteral("—");
        try {
            const auto& category = backend_.transactions().requireCategory(transaction.categoryId);
            categoryLabel = QString::fromStdString(category.name);
        } catch (const std::out_of_range&) {
        }
        recentTransactionsTable->setItem(index, 3, new QTableWidgetItem(categoryLabel));

        auto *amountItem = new QTableWidgetItem(QStringLiteral("$") + QString::number(transaction.amount, 'f', 2));
        amountItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (transaction.type == TransactionType::Income) {
            amountItem->setForeground(QBrush(QColor(QStringLiteral("#8CF4B8"))));
        } else {
            amountItem->setForeground(QBrush(QColor(QStringLiteral("#FF9191"))));
        }
        recentTransactionsTable->setItem(index, 4, amountItem);
    }
}

void DashboardWindow::generateAISummary() {
    if (userId_.empty()) {
        aiStatusLabel->setText(QStringLiteral("No user signed in"));
        return;
    }

    YearMonth period {};
    if (!selectedYearMonth(period)) {
        const QDate today = QDate::currentDate();
        period = YearMonth {today.year(), today.month()};
    }

    beginAiSummaryGeneration(period);
}

void DashboardWindow::cancelPendingAiSummaryUi() {
    ++aiRunGeneration_;
    aiProgressWatchdog_.stop();
    if (aiSparklesPulse_ != nullptr) {
        aiSparklesPulse_->stop();
    }
    if (aiLoadingFrame != nullptr) {
        aiLoadingFrame->setVisible(false);
    }
    if (aiSummaryText != nullptr) {
        aiSummaryText->setVisible(true);
    }
    if (aiSummaryTitle != nullptr) {
        aiSummaryTitle->setText(QStringLiteral("AI-Powered Financial Analysis"));
    }
    if (aiLoadingSubLabel != nullptr) {
        aiLoadingSubLabel->clear();
    }
    resetAiSummaryButtonStyle();
    generateAISummaryButton->setText(QStringLiteral("✨ Generate Insights"));
    generateAISummaryButton->setEnabled(false);
}

void DashboardWindow::resetAiSummaryButtonStyle() {
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
        "}");
}

void DashboardWindow::beginAiSummaryGeneration(const YearMonth& period) {
    ++aiRunGeneration_;
    const int generation = aiRunGeneration_;
    aiWatchdogGeneration_ = generation;
    aiStreamProgressSeenForRun_ = false;
    aiInsightAppliedForRun_ = false;
    aiFallbackJobStarted_ = false;
    aiPendingYearMonth_ = period;

    generateAISummaryButton->setEnabled(false);
    generateAISummaryButton->setText(QStringLiteral("Analyzing…"));
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

    aiStatusLabel->setText(QStringLiteral("✨ Analyzing your finances…"));
    aiStatusLabel->setStyleSheet(QStringLiteral("font-size: 11px; color: #8cc6ff; font-style: italic; font-weight: 500;"));
    aiSummaryTitle->setText(QStringLiteral("Analyzing Your Finances…"));
    if (aiLoadingSubLabel != nullptr) {
        aiLoadingSubLabel->clear();
    }
    aiSummaryText->setPlaceholderText(QStringLiteral("Generating personalized insights…"));
    aiSummaryText->clear();
    aiRecommendationsList->clear();
    aiModelLabel->setVisible(false);
    aiSummaryText->setVisible(false);
    if (aiLoadingFrame != nullptr) {
        aiLoadingFrame->setVisible(true);
    }
    if (aiSparklesPulse_ != nullptr) {
        aiSparklesPulse_->start();
    }

    const std::string userCopy = userId_;
    finsight::core::managers::FinanceTrackerBackend *backend = &backend_;
    const YearMonth periodCopy = period;

    aiProgressWatchdog_.start(5000);

    (void)QtConcurrent::run([this, backend, userCopy, periodCopy, generation]() {
        try {
            const auto insight = backend->ai().generateDashboardInsight(
                userCopy,
                periodCopy,
                backend->analytics(),
                backend->transactions(),
                backend->budgets(),
                backend->savings(),
                backend->goals(),
                [this, generation](const std::string& event, const std::string& model, const std::string& detail) {
                    QMetaObject::invokeMethod(
                        this,
                        [this, generation, event, model, detail]() {
                            if (generation != aiRunGeneration_) {
                                return;
                            }
                            dispatchAiStreamEvent(QString::fromStdString(event),
                                                  QString::fromStdString(model),
                                                  QString::fromStdString(detail));
                        },
                        Qt::QueuedConnection);
                });

            QMetaObject::invokeMethod(
                this,
                [this, generation, insight]() {
                    if (generation != aiRunGeneration_) {
                        return;
                    }
                    if (aiInsightAppliedForRun_) {
                        return;
                    }
                    aiInsightAppliedForRun_ = true;
                    aiProgressWatchdog_.stop();
                    applyAiDashboardInsight(generation, insight);
                },
                Qt::QueuedConnection);
        } catch (const std::exception& ex) {
            QMetaObject::invokeMethod(
                this,
                [this, generation, msg = QString::fromUtf8(ex.what())]() {
                    if (generation != aiRunGeneration_) {
                        return;
                    }
                    if (aiInsightAppliedForRun_) {
                        return;
                    }
                    aiInsightAppliedForRun_ = true;
                    aiProgressWatchdog_.stop();
                    applyAiDashboardRequestFailure(generation, msg);
                },
                Qt::QueuedConnection);
        }
    });
}

void DashboardWindow::onAiProgressWatchdogTimeout() {
    if (aiWatchdogGeneration_ != aiRunGeneration_) {
        return;
    }
    if (aiStreamProgressSeenForRun_) {
        return;
    }
    if (aiInsightAppliedForRun_) {
        return;
    }
    if (aiFallbackJobStarted_) {
        return;
    }
    aiFallbackJobStarted_ = true;
    if (aiLoadingSubLabel != nullptr) {
        aiLoadingSubLabel->setText(
            QStringLiteral("No live progress yet. Running a direct summary request…"));
    }

    const int generation = aiRunGeneration_;
    const std::string userCopy = userId_;
    const YearMonth periodCopy = aiPendingYearMonth_;
    finsight::core::managers::FinanceTrackerBackend *backend = &backend_;

    (void)QtConcurrent::run([this, backend, userCopy, periodCopy, generation]() {
        try {
            const auto insight = backend->ai().generateDashboardInsight(userCopy,
                                                                         periodCopy,
                                                                         backend->analytics(),
                                                                         backend->transactions(),
                                                                         backend->budgets(),
                                                                         backend->savings(),
                                                                         backend->goals());

            QMetaObject::invokeMethod(
                this,
                [this, generation, insight]() {
                    if (generation != aiRunGeneration_) {
                        return;
                    }
                    if (aiInsightAppliedForRun_) {
                        return;
                    }
                    aiInsightAppliedForRun_ = true;
                    aiProgressWatchdog_.stop();
                    applyAiDashboardInsight(generation, insight);
                },
                Qt::QueuedConnection);
        } catch (const std::exception& ex) {
            QMetaObject::invokeMethod(
                this,
                [this, generation, msg = QString::fromUtf8(ex.what())]() {
                    if (generation != aiRunGeneration_) {
                        return;
                    }
                    if (aiInsightAppliedForRun_) {
                        return;
                    }
                    aiInsightAppliedForRun_ = true;
                    aiProgressWatchdog_.stop();
                    applyAiDashboardRequestFailure(generation, msg);
                },
                Qt::QueuedConnection);
        }
    });
}

void DashboardWindow::dispatchAiStreamEvent(const QString& event, const QString& model, const QString& detail) {
    if (!aiStreamProgressSeenForRun_) {
        aiStreamProgressSeenForRun_ = true;
        aiProgressWatchdog_.stop();
    }

    if (event == QStringLiteral("trying_model")) {
        aiSummaryTitle->setText(
            QStringLiteral("Analyzing with %1").arg(displayModelName(model.toStdString())));
        aiStatusLabel->setText(
            QStringLiteral("✨ Analyzing with %1").arg(displayModelName(model.toStdString())));
        if (aiLoadingSubLabel != nullptr) {
            aiLoadingSubLabel->clear();
        }
    } else if (event == QStringLiteral("model_failed")) {
        if (aiLoadingSubLabel != nullptr) {
            aiLoadingSubLabel->setText(
                QStringLiteral("Failed: %1. Trying next…").arg(displayModelName(model.toStdString())));
        }
    } else if (event == QStringLiteral("error")) {
        if (aiLoadingSubLabel != nullptr && !detail.isEmpty()) {
            aiLoadingSubLabel->setText(detail);
        }
    }
}

void DashboardWindow::applyAiDashboardInsight(int generation, const finsight::core::models::AIDashboardInsight& insight) {
    if (generation != aiRunGeneration_) {
        return;
    }

    aiProgressWatchdog_.stop();
    if (aiSparklesPulse_ != nullptr) {
        aiSparklesPulse_->stop();
    }
    if (aiLoadingFrame != nullptr) {
        aiLoadingFrame->setVisible(false);
    }
    aiSummaryText->setVisible(true);
    aiSummaryTitle->setText(QStringLiteral("AI-Powered Financial Analysis"));

    generateAISummaryButton->setEnabled(true);
    resetAiSummaryButtonStyle();
    generateAISummaryButton->setText(QStringLiteral("✨ Generate Insights"));

    if (insight.allModelsBusy) {
        const QString body = QString::fromStdString(insight.summary);
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        aiSummaryText->setMarkdown(QStringLiteral("## All models busy\n\n%1").arg(body));
#else
        aiSummaryText->setPlainText(QStringLiteral("All models busy\n\n") + body);
#endif
        aiRecommendationsList->clear();
        aiModelLabel->setVisible(false);
        aiStatusLabel->setText(QStringLiteral("All models busy"));
        aiStatusLabel->setStyleSheet(QStringLiteral("font-size: 11px; color: #ff9191; font-style: italic; font-weight: 500;"));
        return;
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    aiSummaryText->setMarkdown(QString::fromStdString(insight.summary));
#else
    aiSummaryText->setPlainText(QString::fromStdString(insight.summary));
#endif

    aiRecommendationsList->clear();
    for (const auto& recommendation : insight.recommendations) {
        aiRecommendationsList->addItem(QStringLiteral("💡 ") + QString::fromStdString(recommendation));
    }

    if (!insight.model.empty()) {
        aiModelLabel->setText(
            QStringLiteral("Answered by %1").arg(displayModelName(insight.model)));
        aiModelLabel->setVisible(true);
    }

    aiStatusLabel->setText(QStringLiteral("✅ Analysis complete"));
    aiStatusLabel->setStyleSheet(QStringLiteral("font-size: 11px; color: #8cf4b8; font-style: italic; font-weight: 500;"));
}

void DashboardWindow::applyAiDashboardRequestFailure(int generation, const QString& message) {
    if (generation != aiRunGeneration_) {
        return;
    }

    if (aiSparklesPulse_ != nullptr) {
        aiSparklesPulse_->stop();
    }
    if (aiLoadingFrame != nullptr) {
        aiLoadingFrame->setVisible(false);
    }
    aiSummaryText->setVisible(true);
    aiSummaryTitle->setText(QStringLiteral("AI-Powered Financial Analysis"));

    generateAISummaryButton->setEnabled(true);
    resetAiSummaryButtonStyle();
    generateAISummaryButton->setText(QStringLiteral("✨ Generate Insights"));

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    aiSummaryText->setMarkdown(
        QStringLiteral("## Request failed\n\n%1\n\nCheck your OpenRouter API key in `.env` and your network connection.")
            .arg(message));
#else
    aiSummaryText->setPlainText(QStringLiteral("Request failed\n\n") + message);
#endif
    aiRecommendationsList->clear();
    aiModelLabel->setVisible(false);
    aiStatusLabel->setText(QStringLiteral("Request failed"));
    aiStatusLabel->setStyleSheet(QStringLiteral("font-size: 11px; color: #ff9191; font-style: italic; font-weight: 500;"));
}
