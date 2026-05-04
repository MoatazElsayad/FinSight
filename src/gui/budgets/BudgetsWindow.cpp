#include "gui/budgets/BudgetsWindow.h"

#include "gui/FinSightUi.h"

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
#include <QProgressBar>
#include <QBrush>
#include <QColor>

#include <algorithm>
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

    setStyleSheet(finsight::gui::ui::pageStyle(QStringLiteral("BudgetsWindow")));

    auto *headerColumn = new QVBoxLayout();
    auto *titleLabel = new QLabel("Budgets");
    titleLabel->setStyleSheet(finsight::gui::ui::titleStyle());
    auto *subtitleLabel = new QLabel("Set monthly limits by category and see spend vs. plan at a glance.");
    subtitleLabel->setStyleSheet(finsight::gui::ui::subtitleStyle());
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
        cap->setObjectName("metricCaption");
        auto *val = new QLabel(finsight::gui::ui::formatMoney(0.0));
        val->setObjectName("metricValue");
        val->setStyleSheet(
            QStringLiteral("font-size: 24px; font-weight: 800; color: %1; margin-top: 6px;").arg(accentColor));
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
    inputTitle->setStyleSheet(finsight::gui::ui::cardTitleStyle());
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
    clearButton->setStyleSheet(finsight::gui::ui::ghostButtonStyle());

    auto *labMonth = new QLabel(QStringLiteral("Month"));
    labMonth->setStyleSheet(finsight::gui::ui::labelStyle());
    auto *labCat = new QLabel(QStringLiteral("Category"));
    labCat->setStyleSheet(finsight::gui::ui::labelStyle());
    auto *labAmt = new QLabel(QStringLiteral("Budget amount"));
    labAmt->setStyleSheet(finsight::gui::ui::labelStyle());

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
    tableTitle->setStyleSheet(finsight::gui::ui::cardTitleStyle());
    tableLay->addWidget(tableTitle);
    tableLay->addSpacing(8);

    budgetsTable = new QTableWidget();
    budgetsTable->setColumnCount(6);
    budgetsTable->setHorizontalHeaderLabels({QStringLiteral("Month"), QStringLiteral("Category"),
        QStringLiteral("Budgeted"), QStringLiteral("Spent"), QStringLiteral("Remaining"), QStringLiteral("Progress")});
    finsight::gui::ui::prepareTable(budgetsTable);

    tableLay->addWidget(budgetsTable, 1);
    mainLayout->addWidget(tableCard, 1);

    auto *buttonsLayout = new QHBoxLayout();
    addButton = new QPushButton(QStringLiteral("Add budget"));
    editButton = new QPushButton(QStringLiteral("Save changes"));
    deleteButton = new QPushButton(QStringLiteral("Delete"));

    addButton->setStyleSheet(finsight::gui::ui::primaryButtonStyle());
    editButton->setStyleSheet(finsight::gui::ui::secondaryButtonStyle());
    deleteButton->setStyleSheet(finsight::gui::ui::dangerButtonStyle());

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

    auto *budgetedItem = new QTableWidgetItem(finsight::gui::ui::formatMoney(budgeted));
    budgetedItem->setData(Qt::UserRole, budgeted);
    budgetsTable->setItem(row, 2, budgetedItem);

    auto *spentItem = new QTableWidgetItem(finsight::gui::ui::formatMoney(spent));
    spentItem->setData(Qt::UserRole, spent);
    budgetsTable->setItem(row, 3, spentItem);

    auto *remainingItem = new QTableWidgetItem(finsight::gui::ui::formatMoney(remaining));
    remainingItem->setData(Qt::UserRole, remaining);
    if (remaining < 0.0) {
        remainingItem->setForeground(QBrush(QColor(finsight::gui::ui::dangerColor())));
    } else if (remaining <= budgeted * 0.1) {
        remainingItem->setForeground(QBrush(QColor(finsight::gui::ui::warningColor())));
    } else {
        remainingItem->setForeground(QBrush(QColor(finsight::gui::ui::successColor())));
    }
    budgetsTable->setItem(row, 4, remainingItem);

    auto *progress = new QProgressBar();
    progress->setRange(0, 100);
    const int percent = budgeted <= 0.0 ? 0 : static_cast<int>(std::min(100.0, (spent / budgeted) * 100.0));
    progress->setValue(percent);
    progress->setFormat(QStringLiteral("%1%").arg(QString::number(spent / std::max(1.0, budgeted) * 100.0, 'f', 0)));
    if (remaining < 0.0) {
        progress->setStyleSheet(QStringLiteral(
            "QProgressBar::chunk { background-color: %1; border-radius: 8px; }")
            .arg(finsight::gui::ui::dangerColor()));
    } else if (remaining <= budgeted * 0.1) {
        progress->setStyleSheet(QStringLiteral(
            "QProgressBar::chunk { background-color: %1; border-radius: 8px; }")
            .arg(finsight::gui::ui::warningColor()));
    }
    budgetsTable->setCellWidget(row, 5, progress);
}

void BudgetsWindow::refreshSummary() {
    double totalBudget = 0.0;
    double totalSpent = 0.0;
    double totalRemaining = 0.0;

    for (int row = 0; row < budgetsTable->rowCount(); ++row) {
        totalBudget += budgetsTable->item(row, 2)->data(Qt::UserRole).toDouble();
        totalSpent += budgetsTable->item(row, 3)->data(Qt::UserRole).toDouble();
        totalRemaining += budgetsTable->item(row, 4)->data(Qt::UserRole).toDouble();
    }

    if (totalBudgetValue != nullptr) {
        totalBudgetValue->setText(finsight::gui::ui::formatMoney(totalBudget));
    }
    if (totalSpentValue != nullptr) {
        totalSpentValue->setText(finsight::gui::ui::formatMoney(totalSpent));
    }
    if (totalRemainingValue != nullptr) {
        totalRemainingValue->setText(finsight::gui::ui::formatMoney(totalRemaining));
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
    amountSpin->setValue(budgetsTable->item(currentRow, 2)->data(Qt::UserRole).toDouble());
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
