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
#include <QFrame>
#include <QMessageBox>
#include <QAbstractItemView>
#include <QDialog>
#include <QBrush>
#include <QColor>

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
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(20);

    setStyleSheet(
        "TransactionsWindow, TransactionsWindow > QWidget {"
        "  background-color: #0b1020;"
        "  color: #e5e9f4;"
        "}"
        "QFrame#finCard {"
        "  background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1a2135, stop:1 #141a27);"
        "  border: 1px solid #2b3245;"
        "  border-radius: 14px;"
        "}"
        "QLineEdit, QComboBox, QDateEdit {"
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
    auto *titleLabel = new QLabel(QStringLiteral("Transactions"));
    titleLabel->setStyleSheet(
        "font-size: 32px; font-weight: 700; color: #ffffff; letter-spacing: -0.5px;");
    auto *subtitleLabel = new QLabel(
        QStringLiteral("Search and filter your activity, then add or adjust entries in one place."));
    subtitleLabel->setStyleSheet("font-size: 13px; color: #8d97ac;");
    subtitleLabel->setWordWrap(true);
    headerColumn->addWidget(titleLabel);
    headerColumn->addWidget(subtitleLabel);
    mainLayout->addLayout(headerColumn);

    auto *filtersCard = new QFrame();
    filtersCard->setObjectName("finCard");
    auto *filtersOuter = new QVBoxLayout(filtersCard);
    filtersOuter->setContentsMargins(20, 18, 20, 18);
    auto *filtersTitle = new QLabel(QStringLiteral("Filters"));
    filtersTitle->setStyleSheet("font-size: 14px; font-weight: 600; color: #e5e9f4;");
    filtersOuter->addWidget(filtersTitle);
    filtersOuter->addSpacing(12);

    auto *filtersLayout = new QGridLayout();
    filtersLayout->setHorizontalSpacing(16);
    filtersLayout->setVerticalSpacing(12);

    auto smallLab = [](const QString& t) {
        auto *l = new QLabel(t);
        l->setStyleSheet(QStringLiteral("color: #9ca6bf; font-size: 12px; font-weight: 500;"));
        return l;
    };

    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText(QStringLiteral("Search by title or merchant"));

    typeFilter = new QComboBox();
    typeFilter->addItems({QStringLiteral("All"), QStringLiteral("Income"), QStringLiteral("Expense")});

    categoryFilter = new QComboBox();
    categoryFilter->addItems(
        {QStringLiteral("All"), QStringLiteral("Food"), QStringLiteral("Transport"), QStringLiteral("Shopping"),
            QStringLiteral("Salary"), QStringLiteral("Bills")});

    fromDateEdit = new QDateEdit();
    fromDateEdit->setCalendarPopup(true);
    fromDateEdit->setDate(QDate::currentDate().addMonths(-1));

    toDateEdit = new QDateEdit();
    toDateEdit->setCalendarPopup(true);
    toDateEdit->setDate(QDate::currentDate());

    clearFiltersButton = new QPushButton(QStringLiteral("Reset filters"));
    clearFiltersButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #0f1527;"
        "  color: #aab2c5;"
        "  border: 1px solid #2b3245;"
        "  border-radius: 10px;"
        "  padding: 10px 18px;"
        "  font-weight: 600;"
        "}"
        "QPushButton:hover { background-color: #1a2135; border-color: #3a4155; }");

    filtersLayout->addWidget(smallLab(QStringLiteral("Search")), 0, 0);
    filtersLayout->addWidget(searchEdit, 0, 1, 1, 2);
    filtersLayout->addWidget(smallLab(QStringLiteral("Type")), 0, 3);
    filtersLayout->addWidget(typeFilter, 0, 4);
    filtersLayout->addWidget(clearFiltersButton, 0, 5, 1, 1, Qt::AlignRight);

    filtersLayout->addWidget(smallLab(QStringLiteral("Category")), 1, 0);
    filtersLayout->addWidget(categoryFilter, 1, 1);
    filtersLayout->addWidget(smallLab(QStringLiteral("From")), 1, 2);
    filtersLayout->addWidget(fromDateEdit, 1, 3);
    filtersLayout->addWidget(smallLab(QStringLiteral("To")), 1, 4);
    filtersLayout->addWidget(toDateEdit, 1, 5);

    filtersOuter->addLayout(filtersLayout);
    mainLayout->addWidget(filtersCard);

    auto *tableCard = new QFrame();
    tableCard->setObjectName("finCard");
    auto *tableOuter = new QVBoxLayout(tableCard);
    tableOuter->setContentsMargins(16, 16, 16, 16);
    auto *tableTitle = new QLabel(QStringLiteral("All transactions"));
    tableTitle->setStyleSheet("font-size: 14px; font-weight: 600; color: #e5e9f4;");
    tableOuter->addWidget(tableTitle);
    tableOuter->addSpacing(8);

    transactionsTable = new QTableWidget();
    transactionsTable->setColumnCount(5);
    transactionsTable->setHorizontalHeaderLabels(
        {QStringLiteral("Date"), QStringLiteral("Title"), QStringLiteral("Type"), QStringLiteral("Category"),
            QStringLiteral("Amount")});
    transactionsTable->horizontalHeader()->setStretchLastSection(true);
    transactionsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    transactionsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    transactionsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    transactionsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    transactionsTable->setShowGrid(false);
    transactionsTable->verticalHeader()->setVisible(false);

    tableOuter->addWidget(transactionsTable, 1);
    mainLayout->addWidget(tableCard, 1);

    auto *buttonsLayout = new QHBoxLayout();
    addButton = new QPushButton(QStringLiteral("Add transaction"));
    editButton = new QPushButton(QStringLiteral("Edit"));
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
        "QPushButton:hover { background-color: #1a2742; }");
    deleteButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #2a1a24; color: #ff9db0;"
        "  border: 1px solid #5a3040; border-radius: 10px;"
        "  padding: 10px 22px; font-weight: 600; font-size: 13px;"
        "}"
        "QPushButton:hover { background-color: #3a222e; }");

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
        auto *amountItem = new QTableWidgetItem(QString::number(transaction.amount, 'f', 2));
        if (transaction.type == TransactionType::Income) {
            amountItem->setForeground(QBrush(QColor(QStringLiteral("#8cf4b8"))));
        } else {
            amountItem->setForeground(QBrush(QColor(QStringLiteral("#ff9191"))));
        }
        transactionsTable->setItem(row, 4, amountItem);
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
            backend_.ai(),
            backend_.auth(),
            backend_.transactions(),
            backend_.budgets(),
            backend_.email());
        if (!alerts.empty()) {
            QStringList messages;
            for (const auto& alert : alerts) {
                QString status = alert.success ? QStringLiteral("email sent") : QStringLiteral("email failed");
                if (!alert.success && !alert.error.empty()) {
                    status += QStringLiteral(" - ") + QString::fromStdString(alert.error);
                }
                messages << QString("Budget alert for %1: %2")
                                .arg(QString::fromStdString(alert.recipient))
                                .arg(status);
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
