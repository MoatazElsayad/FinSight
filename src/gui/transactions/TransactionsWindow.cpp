#include "gui/transactions/TransactionsWindow.h"
#include "gui/transactions/TransactionDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QGroupBox>
#include <QDate>
#include <QMessageBox>
#include <QAbstractItemView>
#include <QDialog>

TransactionsWindow::TransactionsWindow(QWidget *parent) : QWidget(parent) {
    setupUi();
    loadDummyData();
}

void TransactionsWindow::setupUi() {
    auto *mainLayout = new QVBoxLayout(this);

    auto *titleLabel = new QLabel("Transactions");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold;");
    mainLayout->addWidget(titleLabel);

    auto *filtersBox = new QGroupBox("Filters");
    auto *filtersLayout = new QGridLayout(filtersBox);

    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("Search by title or merchant");

    typeFilter = new QComboBox();
    typeFilter->addItems({"All", "Income", "Expense"});

    categoryFilter = new QComboBox();
    categoryFilter->addItems({"All", "Food", "Transport", "Shopping", "Salary", "Bills"});

    fromDateEdit = new QDateEdit();
    fromDateEdit->setCalendarPopup(true);
    fromDateEdit->setDate(QDate::currentDate().addMonths(-1));

    toDateEdit = new QDateEdit();
    toDateEdit->setCalendarPopup(true);
    toDateEdit->setDate(QDate::currentDate());

    clearFiltersButton = new QPushButton("Clear Filters");

    filtersLayout->addWidget(new QLabel("Search:"), 0, 0);
    filtersLayout->addWidget(searchEdit, 0, 1);

    filtersLayout->addWidget(new QLabel("Type:"), 0, 2);
    filtersLayout->addWidget(typeFilter, 0, 3);

    filtersLayout->addWidget(new QLabel("Category:"), 1, 0);
    filtersLayout->addWidget(categoryFilter, 1, 1);

    filtersLayout->addWidget(new QLabel("From:"), 1, 2);
    filtersLayout->addWidget(fromDateEdit, 1, 3);

    filtersLayout->addWidget(new QLabel("To:"), 1, 4);
    filtersLayout->addWidget(toDateEdit, 1, 5);

    filtersLayout->addWidget(clearFiltersButton, 0, 5);

    mainLayout->addWidget(filtersBox);

    transactionsTable = new QTableWidget();
    transactionsTable->setColumnCount(5);
    transactionsTable->setHorizontalHeaderLabels({"Date", "Title", "Type", "Category", "Amount"});
    transactionsTable->horizontalHeader()->setStretchLastSection(true);
    transactionsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    transactionsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    transactionsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    transactionsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    mainLayout->addWidget(transactionsTable);

    auto *buttonsLayout = new QHBoxLayout();
    addButton = new QPushButton("Add");
    editButton = new QPushButton("Edit");
    deleteButton = new QPushButton("Delete");

    buttonsLayout->addStretch();
    buttonsLayout->addWidget(addButton);
    buttonsLayout->addWidget(editButton);
    buttonsLayout->addWidget(deleteButton);

    mainLayout->addLayout(buttonsLayout);

    connect(addButton, &QPushButton::clicked, this, &TransactionsWindow::onAddTransaction);
    connect(editButton, &QPushButton::clicked, this, &TransactionsWindow::onEditTransaction);

    connect(deleteButton, &QPushButton::clicked, this, [this]() {
        int currentRow = transactionsTable->currentRow();
        if (currentRow < 0) {
            QMessageBox::information(this, "Delete Transaction", "Please select a transaction first.");
            return;
        }
        transactionsTable->removeRow(currentRow);
    });
}

void TransactionsWindow::loadDummyData() {
    transactionsTable->setRowCount(0);

    addTransactionRow("2026-04-01", "Grocery Store", "Expense", "Food", "250");
    addTransactionRow("2026-04-02", "Uber", "Expense", "Transport", "90");
    addTransactionRow("2026-04-03", "Monthly Salary", "Income", "Salary", "12000");
    addTransactionRow("2026-04-04", "Electricity Bill", "Expense", "Bills", "400");
}

void TransactionsWindow::addTransactionRow(const QString &date,
                                           const QString &title,
                                           const QString &type,
                                           const QString &category,
                                           const QString &amountText) {
    int row = transactionsTable->rowCount();
    transactionsTable->insertRow(row);

    transactionsTable->setItem(row, 0, new QTableWidgetItem(date));
    transactionsTable->setItem(row, 1, new QTableWidgetItem(title));
    transactionsTable->setItem(row, 2, new QTableWidgetItem(type));
    transactionsTable->setItem(row, 3, new QTableWidgetItem(category));
    transactionsTable->setItem(row, 4, new QTableWidgetItem(amountText));
}

void TransactionsWindow::onAddTransaction() {
    TransactionDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted) {
        addTransactionRow(
            dialog.date(),
            dialog.title(),
            dialog.type(),
            dialog.category(),
            QString::number(dialog.amount(), 'f', 2)
        );
    }
}

void TransactionsWindow::onEditTransaction() {
    int currentRow = transactionsTable->currentRow();

    if (currentRow < 0) {
        QMessageBox::information(this, "Edit Transaction", "Please select a transaction first.");
        return;
    }

    TransactionDialog dialog(this);
    dialog.setDialogTitle("Edit Transaction");

    dialog.setDate(transactionsTable->item(currentRow, 0)->text());
    dialog.setTitle(transactionsTable->item(currentRow, 1)->text());
    dialog.setType(transactionsTable->item(currentRow, 2)->text());
    dialog.setCategory(transactionsTable->item(currentRow, 3)->text());
    dialog.setAmount(transactionsTable->item(currentRow, 4)->text().toDouble());

    if (dialog.exec() == QDialog::Accepted) {
        transactionsTable->item(currentRow, 0)->setText(dialog.date());
        transactionsTable->item(currentRow, 1)->setText(dialog.title());
        transactionsTable->item(currentRow, 2)->setText(dialog.type());
        transactionsTable->item(currentRow, 3)->setText(dialog.category());
        transactionsTable->item(currentRow, 4)->setText(QString::number(dialog.amount(), 'f', 2));
    }
}
