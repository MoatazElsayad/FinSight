#include "gui/transactions/TransactionsWindow.h"
#include "gui/transactions/TransactionDialog.h"

#include <QDate>
#include <QStringList>
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

using namespace finsight::core::models;

TransactionsWindow::TransactionsWindow(finsight::core::managers::FinanceTrackerBackend& backend,
                                       const std::string& userId,
                                       QWidget *parent)
    : QWidget(parent),
      backend_(backend),
      userId_(userId) {
    setupUi();
    populateCategoryFilter();
    refreshData();
}

void TransactionsWindow::setUserId(const std::string& userId) {
    userId_ = userId;
    populateCategoryFilter();
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
    connect(deleteButton, &QPushButton::clicked, this, &TransactionsWindow::onDeleteTransaction);
    connect(clearFiltersButton, &QPushButton::clicked, this, [this]() {
        searchEdit->clear();
        typeFilter->setCurrentIndex(0);
        categoryFilter->setCurrentIndex(0);
        fromDateEdit->setDate(QDate::currentDate().addMonths(-1));
        toDateEdit->setDate(QDate::currentDate());
        refreshData();
    });
    connect(searchEdit, &QLineEdit::textChanged, this, [this]() { refreshData(); });
    connect(typeFilter, &QComboBox::currentTextChanged, this, [this]() { refreshData(); });
    connect(categoryFilter, &QComboBox::currentTextChanged, this, [this]() { refreshData(); });
    connect(fromDateEdit, &QDateEdit::dateChanged, this, [this]() { refreshData(); });
    connect(toDateEdit, &QDateEdit::dateChanged, this, [this]() { refreshData(); });
}

void TransactionsWindow::populateCategoryFilter() {
    categoryFilter->clear();
    categoryFilter->addItem("All", "");
    for (const auto& category : backend_.transactions().getCategoriesForUser(userId_)) {
        if (category.kind == CategoryKind::Income || category.kind == CategoryKind::Expense) {
            categoryFilter->addItem(QString::fromStdString(category.name), QString::fromStdString(category.id));
        }
    }
}

void TransactionsWindow::refreshData() {
    transactionsTable->setRowCount(0);
    if (userId_.empty()) {
        return;
    }

    TransactionFilter filter {};
    if (!searchEdit->text().trimmed().isEmpty()) {
        filter.searchText = searchEdit->text().trimmed().toStdString();
    }
    if (typeFilter->currentText() == "Income") {
        filter.type = TransactionType::Income;
    } else if (typeFilter->currentText() == "Expense") {
        filter.type = TransactionType::Expense;
    }
    if (!categoryFilter->currentData().toString().isEmpty()) {
        filter.categoryId = categoryFilter->currentData().toString().toStdString();
    }
    filter.from = Date::fromString(fromDateEdit->date().toString("yyyy-MM-dd").toStdString());
    filter.to = Date::fromString(toDateEdit->date().toString("yyyy-MM-dd").toStdString());

    const auto transactions = backend_.transactions().filterTransactions(userId_, filter);
    for (const auto& transaction : transactions) {
        const auto& category = backend_.transactions().requireCategory(transaction.categoryId);
        const int row = transactionsTable->rowCount();
        transactionsTable->insertRow(row);

        auto *dateItem = new QTableWidgetItem(QString::fromStdString(transaction.date.toString()));
        dateItem->setData(Qt::UserRole, QString::fromStdString(transaction.id));
        transactionsTable->setItem(row, 0, dateItem);
        transactionsTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(transaction.title)));
        transactionsTable->setItem(row, 2, new QTableWidgetItem(
            transaction.type == TransactionType::Income ? "Income" : "Expense"));
        auto *categoryItem = new QTableWidgetItem(QString::fromStdString(category.name));
        categoryItem->setData(Qt::UserRole, QString::fromStdString(category.id));
        transactionsTable->setItem(row, 3, categoryItem);
        transactionsTable->setItem(row, 4, new QTableWidgetItem(QString::number(transaction.amount, 'f', 2)));
    }
}

void TransactionsWindow::onAddTransaction() {
    TransactionDialog dialog(this);
    dialog.setAvailableCategories(backend_.transactions().getCategoriesForUser(userId_));

    if (dialog.exec() == QDialog::Accepted) {
        const auto transaction = backend_.transactions().addTransaction(Transaction {
            .userId = userId_,
            .title = dialog.title().toStdString(),
            .description = dialog.description().toStdString(),
            .categoryId = dialog.categoryId(),
            .type = dialog.transactionType(),
            .amount = dialog.amount(),
            .date = Date::fromString(dialog.date().toStdString()),
            .merchant = dialog.merchant().toStdString(),
        });

        const auto alerts = backend_.budgetAlerts().notifyBudgetExceededByTransaction(
            transaction,
            backend_.auth(),
            backend_.transactions(),
            backend_.budgets(),
            backend_.email());
        if (!alerts.empty()) {
            QStringList messages;
            for (const auto& alert : alerts) {
                messages << QString("Budget alert for %1: %2")
                                .arg(QString::fromStdString(alert.recipient))
                                .arg(alert.success ? "email sent" : "email failed");
            }
            QMessageBox::information(this, "Budget Alerts", messages.join("\n"));
        }
        refreshData();
        emit dataChanged();
    }
}

void TransactionsWindow::onEditTransaction() {
    const auto transaction = selectedTransaction();
    if (!transaction) {
        QMessageBox::information(this, "Edit Transaction", "Please select a transaction first.");
        return;
    }

    TransactionDialog dialog(this);
    dialog.setDialogTitle("Edit Transaction");
    dialog.setAvailableCategories(backend_.transactions().getCategoriesForUser(userId_));
    dialog.setDate(QString::fromStdString(transaction->date.toString()));
    dialog.setTitle(QString::fromStdString(transaction->title));
    dialog.setType(transaction->type == TransactionType::Income ? "Income" : "Expense");
    dialog.setCategoryId(transaction->categoryId);
    dialog.setAmount(transaction->amount);
    dialog.setDescription(QString::fromStdString(transaction->description));
    dialog.setMerchant(QString::fromStdString(transaction->merchant));

    if (dialog.exec() == QDialog::Accepted) {
        backend_.transactions().updateTransaction(
            userId_,
            transaction->id,
            Transaction {
                .id = transaction->id,
                .userId = userId_,
                .title = dialog.title().toStdString(),
                .description = dialog.description().toStdString(),
                .categoryId = dialog.categoryId(),
                .type = dialog.transactionType(),
                .amount = dialog.amount(),
                .date = Date::fromString(dialog.date().toStdString()),
                .merchant = dialog.merchant().toStdString(),
            });
        refreshData();
        emit dataChanged();
    }
}

void TransactionsWindow::onDeleteTransaction() {
    const auto transaction = selectedTransaction();
    if (!transaction) {
        QMessageBox::information(this, "Delete Transaction", "Please select a transaction first.");
        return;
    }
    backend_.transactions().deleteTransaction(userId_, transaction->id);
    refreshData();
    emit dataChanged();
}

std::optional<Transaction> TransactionsWindow::selectedTransaction() const {
    const int currentRow = transactionsTable->currentRow();
    if (currentRow < 0 || transactionsTable->item(currentRow, 0) == nullptr) {
        return std::nullopt;
    }

    const std::string transactionId =
        transactionsTable->item(currentRow, 0)->data(Qt::UserRole).toString().toStdString();
    for (const auto& transaction : backend_.transactions().listTransactions(userId_)) {
        if (transaction.id == transactionId) {
            return transaction;
        }
    }
    return std::nullopt;
}
