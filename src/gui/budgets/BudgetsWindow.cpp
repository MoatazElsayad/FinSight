#include "gui/budgets/BudgetsWindow.h"

#include <QDate>
#include <QStringList>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QFrame>
#include <QMessageBox>
#include <QAbstractItemView>

#include <exception>
#include <utility>

using namespace finsight::core::models;

namespace {

QString formatMonth(const YearMonth& period) {
    static const QStringList names {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };
    return names.at(period.month - 1) + " " + QString::number(period.year);
}

int monthIndexFromText(const QString& text) {
    static const QStringList names {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };
    for (int index = 0; index < names.size(); ++index) {
        if (text.startsWith(names[index])) {
            return index + 1;
        }
    }
    return 1;
}

}

BudgetsWindow::BudgetsWindow(finsight::core::managers::FinanceTrackerBackend& backend,
                             const std::string& userId,
                             QWidget *parent)
    : QWidget(parent),
      backend_(backend),
      userId_(userId) {
    setupUi();
    populateCategories();
    refreshData();
    refreshSummary();
}

void BudgetsWindow::setUserId(const std::string& userId) {
    userId_ = userId;
    populateCategories();
}

void BudgetsWindow::setupUi() {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(20);

    setStyleSheet(
        "BudgetsWindow, BudgetsWindow > QWidget {"
        "  background-color: #0b1020;"
        "  color: #e5e9f4;"
        "}"
        "QFrame#finCard {"
        "  background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1a2135, stop:1 #141a27);"
        "  border: 1px solid #2b3245;"
        "  border-radius: 14px;"
        "}"
        "QLineEdit, QComboBox, QDoubleSpinBox {"
        "  background-color: #0f1527;"
        "  border: 1px solid #2b3245;"
        "  border-radius: 10px;"
        "  color: #e5e9f4;"
        "  padding: 8px 10px;"
        "  min-height: 22px;"
        "}"
        "QTableWidget {"
        "  background-color: #0f1527;"
        "  border: 1px solid #2a4080;"
        "  border-radius: 10px;"
        "  color: #e5e9f4;"
        "  gridline-color: #1e2436;"
        "  selection-background-color: #253355;"
        "}"
        "QTableWidget::item { padding: 8px; border-bottom: 1px solid #1e2436; }"
        "QTableWidget::item:selected { background-color: #253355; }"
        "QHeaderView::section {"
        "  background-color: #0f1a33;"
        "  color: #5b8cff;"
        "  border: 0;"
        "  padding: 10px 8px;"
        "  font-weight: 700;"
        "  border-bottom: 2px solid #2a4080;"
        "}"
        "QScrollBar:vertical { background-color: #0f1527; width: 12px; border-radius: 6px; }"
        "QScrollBar::handle:vertical { background-color: #2b3245; border-radius: 6px; min-height: 24px; }"
    );

    auto *headerColumn = new QVBoxLayout();
    auto *titleLabel = new QLabel("Budgets");
    titleLabel->setStyleSheet(
        "font-size: 32px; font-weight: 700; color: #ffffff; letter-spacing: -0.5px;");
    auto *subtitleLabel = new QLabel("Set monthly limits by category and see spend vs. plan at a glance.");
    subtitleLabel->setStyleSheet("font-size: 13px; color: #8d97ac;");
    subtitleLabel->setWordWrap(true);
    headerColumn->addWidget(titleLabel);
    headerColumn->addWidget(subtitleLabel);
    mainLayout->addLayout(headerColumn);

    auto *summaryRow = new QHBoxLayout();
    summaryRow->setSpacing(16);
    auto makeMetricCard = [](const QString& caption, const QString& accentColor) {
        auto *card = new QFrame();
        card->setObjectName("finCard");
        card->setMinimumHeight(104);
        auto *lay = new QVBoxLayout(card);
        lay->setContentsMargins(18, 16, 18, 16);
        auto *cap = new QLabel(caption);
        cap->setStyleSheet("font-size: 12px; font-weight: 500; color: #9ca6bf; letter-spacing: 0.4px;");
        auto *val = new QLabel(QStringLiteral("$0.00"));
        val->setStyleSheet(
            QStringLiteral("font-size: 26px; font-weight: 700; color: %1; margin-top: 6px;").arg(accentColor));
        lay->addWidget(cap);
        lay->addWidget(val);
        lay->addStretch();
        return std::pair<QFrame*, QLabel*> {card, val};
    };
    {
        auto p = makeMetricCard(QStringLiteral("Total budgeted"), QStringLiteral("#8cc6ff"));
        summaryRow->addWidget(p.first, 1);
        totalBudgetValue = p.second;
    }
    {
        auto p = makeMetricCard(QStringLiteral("Total spent"), QStringLiteral("#ff9191"));
        summaryRow->addWidget(p.first, 1);
        totalSpentValue = p.second;
    }
    {
        auto p = makeMetricCard(QStringLiteral("Remaining"), QStringLiteral("#8cf4b8"));
        summaryRow->addWidget(p.first, 1);
        totalRemainingValue = p.second;
    }
    mainLayout->addLayout(summaryRow);

    auto *inputCard = new QFrame();
    inputCard->setObjectName("finCard");
    auto *inputOuter = new QVBoxLayout(inputCard);
    inputOuter->setContentsMargins(20, 18, 20, 18);
    auto *inputTitle = new QLabel(QStringLiteral("Add or edit budget"));
    inputTitle->setStyleSheet("font-size: 14px; font-weight: 600; color: #e5e9f4;");
    inputOuter->addWidget(inputTitle);
    inputOuter->addSpacing(12);

    auto *inputLayout = new QGridLayout();
    inputLayout->setHorizontalSpacing(16);
    inputLayout->setVerticalSpacing(12);

    monthCombo = new QComboBox();
    monthCombo->addItems({
        "January 2026", "February 2026", "March 2026", "April 2026",
        "May 2026", "June 2026", "July 2026", "August 2026",
        "September 2026", "October 2026", "November 2026", "December 2026"
    });
    monthCombo->setCurrentText(formatMonth(YearMonth {QDate::currentDate().year(), QDate::currentDate().month()}));

    categoryCombo = new QComboBox();

    amountSpin = new QDoubleSpinBox();
    amountSpin->setRange(1.0, 100000000.0);
    amountSpin->setDecimals(2);
    amountSpin->setSingleStep(50.0);

    clearButton = new QPushButton(QStringLiteral("Clear"));
    clearButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #0f1527;"
        "  color: #aab2c5;"
        "  border: 1px solid #2b3245;"
        "  border-radius: 10px;"
        "  padding: 10px 18px;"
        "  font-weight: 600;"
        "}"
        "QPushButton:hover { background-color: #1a2135; border-color: #3a4155; }");

    auto *labMonth = new QLabel(QStringLiteral("Month"));
    labMonth->setStyleSheet(QStringLiteral("color: #9ca6bf; font-size: 12px; font-weight: 500;"));
    auto *labCat = new QLabel(QStringLiteral("Category"));
    labCat->setStyleSheet(QStringLiteral("color: #9ca6bf; font-size: 12px; font-weight: 500;"));
    auto *labAmt = new QLabel(QStringLiteral("Budget amount"));
    labAmt->setStyleSheet(QStringLiteral("color: #9ca6bf; font-size: 12px; font-weight: 500;"));

    inputLayout->addWidget(labMonth, 0, 0);
    inputLayout->addWidget(monthCombo, 0, 1);
    inputLayout->addWidget(labCat, 0, 2);
    inputLayout->addWidget(categoryCombo, 0, 3);
    inputLayout->addWidget(labAmt, 1, 0);
    inputLayout->addWidget(amountSpin, 1, 1);
    inputLayout->addWidget(clearButton, 1, 3, Qt::AlignRight);

    inputOuter->addLayout(inputLayout);
    mainLayout->addWidget(inputCard);

    auto *tableCard = new QFrame();
    tableCard->setObjectName("finCard");
    auto *tableLay = new QVBoxLayout(tableCard);
    tableLay->setContentsMargins(16, 16, 16, 16);
    auto *tableTitle = new QLabel(QStringLiteral("Your budgets"));
    tableTitle->setStyleSheet("font-size: 14px; font-weight: 600; color: #e5e9f4;");
    tableLay->addWidget(tableTitle);
    tableLay->addSpacing(8);

    budgetsTable = new QTableWidget();
    budgetsTable->setColumnCount(5);
    budgetsTable->setHorizontalHeaderLabels({QStringLiteral("Month"), QStringLiteral("Category"),
        QStringLiteral("Budgeted"), QStringLiteral("Spent"), QStringLiteral("Remaining")});
    budgetsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    budgetsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    budgetsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    budgetsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    budgetsTable->setShowGrid(false);
    budgetsTable->verticalHeader()->setVisible(false);

    tableLay->addWidget(budgetsTable, 1);
    mainLayout->addWidget(tableCard, 1);

    auto *buttonsLayout = new QHBoxLayout();
    addButton = new QPushButton(QStringLiteral("Add budget"));
    editButton = new QPushButton(QStringLiteral("Save changes"));
    deleteButton = new QPushButton(QStringLiteral("Delete"));

    addButton->setStyleSheet(
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #5b8cff, stop:1 #4a7ae6);"
        "  color: #ffffff; border: none; border-radius: 10px;"
        "  padding: 10px 22px; font-weight: 600; font-size: 13px;"
        "}"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #6b9cff, stop:1 #5a8af6); }");
    editButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #0f1527; color: #e5e9f4;"
        "  border: 1px solid #3a5490; border-radius: 10px;"
        "  padding: 10px 22px; font-weight: 600; font-size: 13px;"
        "}"
        "QPushButton:hover { background-color: #1a2742; }"
        "QPushButton:disabled { color: #5c6578; border-color: #2b3245; }");
    deleteButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #2a1a24; color: #ff9db0;"
        "  border: 1px solid #5a3040; border-radius: 10px;"
        "  padding: 10px 22px; font-weight: 600; font-size: 13px;"
        "}"
        "QPushButton:hover { background-color: #3a222e; }"
        "QPushButton:disabled { color: #6b4a55; border-color: #2b3245; }");

    editButton->setEnabled(false);
    deleteButton->setEnabled(false);

    buttonsLayout->addStretch();
    buttonsLayout->addWidget(addButton);
    buttonsLayout->addWidget(editButton);
    buttonsLayout->addWidget(deleteButton);

    mainLayout->addLayout(buttonsLayout);

    connect(addButton, &QPushButton::clicked, this, &BudgetsWindow::onAddBudget);
    connect(editButton, &QPushButton::clicked, this, &BudgetsWindow::onEditBudget);
    connect(deleteButton, &QPushButton::clicked, this, &BudgetsWindow::onDeleteBudget);
    connect(clearButton, &QPushButton::clicked, this, &BudgetsWindow::clearInputs);
    connect(budgetsTable, &QTableWidget::itemSelectionChanged, this, &BudgetsWindow::onTableSelectionChanged);
}

void BudgetsWindow::populateCategories() {
    categoryCombo->clear();
    for (const auto& category : backend_.transactions().getCategoriesForUser(userId_)) {
        if (category.kind == CategoryKind::Expense) {
            categoryCombo->addItem(QString::fromStdString(category.name), QString::fromStdString(category.id));
        }
    }
}

void BudgetsWindow::refreshData() {
    budgetsTable->setRowCount(0);
    if (userId_.empty()) {
        refreshSummary();
        return;
    }

    for (const auto& budget : backend_.budgets().allBudgets()) {
        if (budget.userId != userId_) {
            continue;
        }
        const auto& category = backend_.transactions().requireCategory(budget.categoryId);
        const double spent = backend_.transactions().spentForCategory(userId_, budget.categoryId, budget.period);
        addBudgetRow(formatMonth(budget.period),
                     QString::fromStdString(category.name),
                     QString::fromStdString(budget.id),
                     QString::fromStdString(budget.categoryId),
                     budget.limit,
                     spent);
    }
    refreshSummary();
}

void BudgetsWindow::addBudgetRow(const QString &month,
                                 const QString &category,
                                 const QString& budgetId,
                                 const QString& categoryId,
                                 double budgeted,
                                 double spent) {
    int row = budgetsTable->rowCount();
    budgetsTable->insertRow(row);

    double remaining = budgeted - spent;

    auto *monthItem = new QTableWidgetItem(month);
    monthItem->setData(Qt::UserRole, budgetId);
    budgetsTable->setItem(row, 0, monthItem);
    auto *categoryItem = new QTableWidgetItem(category);
    categoryItem->setData(Qt::UserRole, categoryId);
    budgetsTable->setItem(row, 1, categoryItem);
    budgetsTable->setItem(row, 2, new QTableWidgetItem(QString::number(budgeted, 'f', 2)));
    budgetsTable->setItem(row, 3, new QTableWidgetItem(QString::number(spent, 'f', 2)));
    budgetsTable->setItem(row, 4, new QTableWidgetItem(QString::number(remaining, 'f', 2)));
}

void BudgetsWindow::refreshSummary() {
    double totalBudget = 0.0;
    double totalSpent = 0.0;
    double totalRemaining = 0.0;

    for (int row = 0; row < budgetsTable->rowCount(); ++row) {
        totalBudget += budgetsTable->item(row, 2)->text().toDouble();
        totalSpent += budgetsTable->item(row, 3)->text().toDouble();
        totalRemaining += budgetsTable->item(row, 4)->text().toDouble();
    }

    if (totalBudgetValue != nullptr) {
        totalBudgetValue->setText(QStringLiteral("$") + QString::number(totalBudget, 'f', 2));
    }
    if (totalSpentValue != nullptr) {
        totalSpentValue->setText(QStringLiteral("$") + QString::number(totalSpent, 'f', 2));
    }
    if (totalRemainingValue != nullptr) {
        totalRemainingValue->setText(QStringLiteral("$") + QString::number(totalRemaining, 'f', 2));
    }
}

void BudgetsWindow::clearInputs() {
    monthCombo->setCurrentText(formatMonth(YearMonth {QDate::currentDate().year(), QDate::currentDate().month()}));
    categoryCombo->setCurrentIndex(0);
    amountSpin->setValue(0.0);
    budgetsTable->clearSelection();
    editButton->setEnabled(false);
    deleteButton->setEnabled(false);
}

void BudgetsWindow::onAddBudget() {
    double budgeted = amountSpin->value();

    if (budgeted <= 0.0) {
        QMessageBox::warning(this, "Validation Error", "Budget amount must be greater than 0.");
        return;
    }
    if (categoryCombo->currentIndex() < 0 || categoryCombo->currentData().toString().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Please choose a category.");
        return;
    }

    try {
        backend_.budgets().setBudget(
            userId_,
            categoryCombo->currentData().toString().toStdString(),
            selectedPeriod(),
            budgeted);
        refreshData();
        clearInputs();
        emit dataChanged();
    } catch (const std::exception& error) {
        QMessageBox::warning(this, "Budget Error", error.what());
    }
}

void BudgetsWindow::onEditBudget() {
    const auto budget = selectedBudget();
    if (!budget) {
        QMessageBox::information(this, "Edit Budget", "Please select a budget first.");
        return;
    }

    double budgeted = amountSpin->value();

    if (budgeted <= 0.0) {
        QMessageBox::warning(this, "Validation Error", "Budget amount must be greater than 0.");
        return;
    }
    if (categoryCombo->currentIndex() < 0 || categoryCombo->currentData().toString().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Please choose a category.");
        return;
    }

    try {
        backend_.budgets().updateBudget(
            userId_,
            budget->id,
            Budget {
                .id = budget->id,
                .userId = userId_,
                .categoryId = categoryCombo->currentData().toString().toStdString(),
                .period = selectedPeriod(),
                .limit = budgeted,
            });
        refreshData();
        clearInputs();
        emit dataChanged();
    } catch (const std::exception& error) {
        QMessageBox::warning(this, "Budget Error", error.what());
    }
}

void BudgetsWindow::onDeleteBudget() {
    const auto budget = selectedBudget();
    if (!budget) {
        QMessageBox::information(this, "Delete Budget", "Please select a budget first.");
        return;
    }

    auto reply = QMessageBox::question(
        this,
        "Delete Budget",
        "Are you sure you want to delete the selected budget?"
    );

    if (reply == QMessageBox::Yes) {
        backend_.budgets().deleteBudget(userId_, budget->id);
        refreshData();
        clearInputs();
        emit dataChanged();
    }
}

void BudgetsWindow::onTableSelectionChanged() {
    int currentRow = budgetsTable->currentRow();
    bool hasSelection = currentRow >= 0;

    editButton->setEnabled(hasSelection);
    deleteButton->setEnabled(hasSelection);

    if (!hasSelection) {
        return;
    }

    monthCombo->setCurrentText(budgetsTable->item(currentRow, 0)->text());
    const int categoryIndex = categoryCombo->findData(budgetsTable->item(currentRow, 1)->data(Qt::UserRole));
    if (categoryIndex >= 0) {
        categoryCombo->setCurrentIndex(categoryIndex);
    }
    amountSpin->setValue(budgetsTable->item(currentRow, 2)->text().toDouble());
}

std::optional<Budget> BudgetsWindow::selectedBudget() const {
    const int currentRow = budgetsTable->currentRow();
    if (currentRow < 0 || budgetsTable->item(currentRow, 0) == nullptr) {
        return std::nullopt;
    }

    const std::string budgetId = budgetsTable->item(currentRow, 0)->data(Qt::UserRole).toString().toStdString();
    for (const auto& budget : backend_.budgets().allBudgets()) {
        if (budget.id == budgetId && budget.userId == userId_) {
            return budget;
        }
    }
    return std::nullopt;
}

YearMonth BudgetsWindow::selectedPeriod() const {
    const QString monthText = monthCombo->currentText();
    const int year = monthText.right(4).toInt();
    return YearMonth {year, monthIndexFromText(monthText)};
}
