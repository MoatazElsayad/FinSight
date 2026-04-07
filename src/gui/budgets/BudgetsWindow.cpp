#include "gui/budgets/BudgetsWindow.h"

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

BudgetsWindow::BudgetsWindow(QWidget *parent) : QWidget(parent) {
    setupUi();
    loadDummyData();
    refreshSummary();
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
    monthCombo->setCurrentText("April 2026");

    categoryCombo = new QComboBox();
    categoryCombo->addItems({"Food", "Transport", "Shopping", "Salary", "Bills", "Entertainment"});

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

void BudgetsWindow::loadDummyData() {
    budgetsTable->setRowCount(0);

    addBudgetRow("April 2026", "Food", 1500.0, 900.0);
    addBudgetRow("April 2026", "Transport", 800.0, 350.0);
    addBudgetRow("April 2026", "Bills", 2000.0, 1450.0);
    addBudgetRow("April 2026", "Entertainment", 700.0, 500.0);
}

void BudgetsWindow::addBudgetRow(const QString &month,
                                 const QString &category,
                                 double budgeted,
                                 double spent) {
    int row = budgetsTable->rowCount();
    budgetsTable->insertRow(row);

    double remaining = budgeted - spent;

    budgetsTable->setItem(row, 0, new QTableWidgetItem(month));
    budgetsTable->setItem(row, 1, new QTableWidgetItem(category));
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
    monthCombo->setCurrentText("April 2026");
    categoryCombo->setCurrentIndex(0);
    amountSpin->setValue(0.0);
    budgetsTable->clearSelection();
    editButton->setEnabled(false);
    deleteButton->setEnabled(false);
}

void BudgetsWindow::onAddBudget() {
    QString month = monthCombo->currentText();
    QString category = categoryCombo->currentText();
    double budgeted = amountSpin->value();

    if (budgeted <= 0.0) {
        QMessageBox::warning(this, "Validation Error", "Budget amount must be greater than 0.");
        return;
    }

    double spent = 0.0;

    if (category == "Food") spent = 900.0;
    else if (category == "Transport") spent = 350.0;
    else if (category == "Bills") spent = 1450.0;
    else if (category == "Entertainment") spent = 500.0;
    else if (category == "Shopping") spent = 600.0;
    else if (category == "Salary") spent = 0.0;

    addBudgetRow(month, category, budgeted, spent);
    refreshSummary();
    clearInputs();
}

void BudgetsWindow::onEditBudget() {
    int currentRow = budgetsTable->currentRow();

    if (currentRow < 0) {
        QMessageBox::information(this, "Edit Budget", "Please select a budget first.");
        return;
    }

    double budgeted = amountSpin->value();

    if (budgeted <= 0.0) {
        QMessageBox::warning(this, "Validation Error", "Budget amount must be greater than 0.");
        return;
    }

    double spent = budgetsTable->item(currentRow, 3)->text().toDouble();
    double remaining = budgeted - spent;

    budgetsTable->item(currentRow, 0)->setText(monthCombo->currentText());
    budgetsTable->item(currentRow, 1)->setText(categoryCombo->currentText());
    budgetsTable->item(currentRow, 2)->setText(QString::number(budgeted, 'f', 2));
    budgetsTable->item(currentRow, 4)->setText(QString::number(remaining, 'f', 2));

    refreshSummary();
    clearInputs();
}

void BudgetsWindow::onDeleteBudget() {
    int currentRow = budgetsTable->currentRow();

    if (currentRow < 0) {
        QMessageBox::information(this, "Delete Budget", "Please select a budget first.");
        return;
    }

    auto reply = QMessageBox::question(
        this,
        "Delete Budget",
        "Are you sure you want to delete the selected budget?"
    );

    if (reply == QMessageBox::Yes) {
        budgetsTable->removeRow(currentRow);
        refreshSummary();
        clearInputs();
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
    categoryCombo->setCurrentText(budgetsTable->item(currentRow, 1)->text());
    amountSpin->setValue(budgetsTable->item(currentRow, 2)->text().toDouble());
}
