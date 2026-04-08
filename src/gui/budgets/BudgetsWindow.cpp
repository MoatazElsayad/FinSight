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
#include <QGroupBox>
#include <QMessageBox>
#include <QAbstractItemView>

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

    auto *titleLabel = new QLabel("Budgets");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold;");
    mainLayout->addWidget(titleLabel);

    auto *inputBox = new QGroupBox("Budget Details");
    auto *inputLayout = new QGridLayout(inputBox);

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

    clearButton = new QPushButton("Clear");

    inputLayout->addWidget(new QLabel("Month:"), 0, 0);
    inputLayout->addWidget(monthCombo, 0, 1);
    inputLayout->addWidget(new QLabel("Category:"), 0, 2);
    inputLayout->addWidget(categoryCombo, 0, 3);
    inputLayout->addWidget(new QLabel("Budget Amount:"), 1, 0);
    inputLayout->addWidget(amountSpin, 1, 1);
    inputLayout->addWidget(clearButton, 1, 3);

    mainLayout->addWidget(inputBox);

    budgetsTable = new QTableWidget();
    budgetsTable->setColumnCount(5);
    budgetsTable->setHorizontalHeaderLabels({"Month", "Category", "Budgeted", "Spent", "Remaining"});
    budgetsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    budgetsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    budgetsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    budgetsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    mainLayout->addWidget(budgetsTable);

    auto *summaryBox = new QGroupBox("Summary");
    auto *summaryLayout = new QHBoxLayout(summaryBox);

    totalBudgetLabel = new QLabel("Total Budgeted: 0.00");
    totalSpentLabel = new QLabel("Total Spent: 0.00");
    totalRemainingLabel = new QLabel("Total Remaining: 0.00");

    summaryLayout->addWidget(totalBudgetLabel);
    summaryLayout->addWidget(totalSpentLabel);
    summaryLayout->addWidget(totalRemainingLabel);

    mainLayout->addWidget(summaryBox);

    auto *buttonsLayout = new QHBoxLayout();
    addButton = new QPushButton("Add");
    editButton = new QPushButton("Edit");
    deleteButton = new QPushButton("Delete");

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

    totalBudgetLabel->setText("Total Budgeted: " + QString::number(totalBudget, 'f', 2));
    totalSpentLabel->setText("Total Spent: " + QString::number(totalSpent, 'f', 2));
    totalRemainingLabel->setText("Total Remaining: " + QString::number(totalRemaining, 'f', 2));
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

    backend_.budgets().createBudget(
        userId_,
        categoryCombo->currentData().toString().toStdString(),
        selectedPeriod(),
        budgeted);
    refreshData();
    clearInputs();
    emit dataChanged();
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
