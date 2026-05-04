#include "gui/transactions/TransactionsWindow.h"
#include "gui/transactions/TransactionDialog.h"
#include "gui/FinSightUi.h"

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

#include <exception>
#include <stdexcept>

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

    setStyleSheet(finsight::gui::ui::pageStyle(QStringLiteral("TransactionsWindow")));

    auto *headerColumn = new QVBoxLayout();
    auto *titleLabel = new QLabel(QStringLiteral("Transactions"));
    titleLabel->setStyleSheet(finsight::gui::ui::titleStyle());
    auto *subtitleLabel = new QLabel(
        QStringLiteral("Search and filter your activity, then add or adjust entries in one place."));
    subtitleLabel->setStyleSheet(finsight::gui::ui::subtitleStyle());
    subtitleLabel->setWordWrap(true);
    headerColumn->addWidget(titleLabel);
    headerColumn->addWidget(subtitleLabel);
    mainLayout->addLayout(headerColumn);

    auto *filtersCard = new QFrame();
    filtersCard->setObjectName("finCard");
    auto *filtersOuter = new QVBoxLayout(filtersCard);
    filtersOuter->setContentsMargins(20, 18, 20, 18);
    auto *filtersTitle = new QLabel(QStringLiteral("Filters"));
    filtersTitle->setStyleSheet(finsight::gui::ui::cardTitleStyle());
    filtersOuter->addWidget(filtersTitle);
    filtersOuter->addSpacing(12);

    auto *filtersLayout = new QGridLayout();
    filtersLayout->setHorizontalSpacing(16);
    filtersLayout->setVerticalSpacing(12);

    auto smallLab = [](const QString& t) {
        auto *l = new QLabel(t);
        l->setStyleSheet(finsight::gui::ui::labelStyle());
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
    clearFiltersButton->setStyleSheet(finsight::gui::ui::ghostButtonStyle());

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
    tableTitle->setStyleSheet(finsight::gui::ui::cardTitleStyle());
    tableOuter->addWidget(tableTitle);
    tableOuter->addSpacing(8);

    transactionsTable = new QTableWidget();
    transactionsTable->setColumnCount(5);
    transactionsTable->setHorizontalHeaderLabels(
        {QStringLiteral("Date"), QStringLiteral("Title"), QStringLiteral("Type"), QStringLiteral("Category"),
            QStringLiteral("Amount")});
    finsight::gui::ui::prepareTable(transactionsTable);

    tableOuter->addWidget(transactionsTable, 1);
    mainLayout->addWidget(tableCard, 1);

    auto *buttonsLayout = new QHBoxLayout();
    addButton = new QPushButton(QStringLiteral("Add transaction"));
    editButton = new QPushButton(QStringLiteral("Edit"));
    deleteButton = new QPushButton(QStringLiteral("Delete"));

    addButton->setStyleSheet(finsight::gui::ui::primaryButtonStyle());
    editButton->setStyleSheet(finsight::gui::ui::secondaryButtonStyle());
    deleteButton->setStyleSheet(finsight::gui::ui::dangerButtonStyle());

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

    if (userId_.empty()) {
        return;
    }

    try {
        for (const auto& category : backend_.transactions().getCategoriesForUser(userId_)) {
            if (category.kind == CategoryKind::Income || category.kind == CategoryKind::Expense) {
                categoryFilter->addItem(QString::fromStdString(category.name), QString::fromStdString(category.id));
            }
        }
    } catch (const std::exception& error) {
        showOperationError(QStringLiteral("Load Categories"), error);
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

    try {
        const auto transactions = backend_.transactions().filterTransactions(userId_, filter);
        for (const auto& transaction : transactions) {
            QString categoryName = QStringLiteral("Unknown");
            QString categoryId = QString::fromStdString(transaction.categoryId);
            try {
                const auto& category = backend_.transactions().requireCategory(transaction.categoryId);
                categoryName = QString::fromStdString(category.name);
            } catch (const std::out_of_range&) {
            }

            const int row = transactionsTable->rowCount();
            transactionsTable->insertRow(row);

            auto *dateItem = new QTableWidgetItem(QString::fromStdString(transaction.date.toString()));
            dateItem->setData(Qt::UserRole, QString::fromStdString(transaction.id));
            transactionsTable->setItem(row, 0, dateItem);
            transactionsTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(transaction.title)));
            transactionsTable->setItem(row, 2, new QTableWidgetItem(
                transaction.type == TransactionType::Income ? "Income" : "Expense"));
            auto *categoryItem = new QTableWidgetItem(categoryName);
            categoryItem->setData(Qt::UserRole, categoryId);
            transactionsTable->setItem(row, 3, categoryItem);
            auto *amountItem = new QTableWidgetItem(finsight::gui::ui::formatMoney(transaction.amount));
            if (transaction.type == TransactionType::Income) {
                amountItem->setForeground(QBrush(QColor(QStringLiteral("#8cf4b8"))));
            } else {
                amountItem->setForeground(QBrush(QColor(QStringLiteral("#ff9191"))));
            }
            transactionsTable->setItem(row, 4, amountItem);
        }
    } catch (const std::exception& error) {
        showOperationError(QStringLiteral("Load Transactions"), error);
    }
}

bool TransactionsWindow::ensureSignedIn(const QString& actionTitle) {
    if (!userId_.empty()) {
        return true;
    }

    QMessageBox::warning(
        this,
        actionTitle,
        QStringLiteral("Please log in before managing transactions."));
    return false;
}

void TransactionsWindow::showOperationError(const QString& actionTitle, const std::exception& error) {
    QMessageBox::warning(
        this,
        actionTitle,
        QStringLiteral("Something went wrong while processing your request.\n\nDetails: %1")
            .arg(QString::fromUtf8(error.what())));
}

void TransactionsWindow::onAddTransaction() {
    if (!ensureSignedIn(QStringLiteral("Add Transaction"))) {
        return;
    }

    TransactionDialog dialog(backend_, userId_, this);
    try {
        dialog.setAvailableCategories(backend_.transactions().getCategoriesForUser(userId_));
    } catch (const std::exception& error) {
        showOperationError(QStringLiteral("Add Transaction"), error);
        return;
    }

    const int result = dialog.exec();
    if (dialog.categoriesChanged()) {
        populateCategoryFilter();
    }

    if (result == QDialog::Accepted) {
        Transaction transaction {};
        try {
            transaction = backend_.transactions().addTransaction(Transaction {
                .userId = userId_,
                .title = dialog.title().toStdString(),
                .description = dialog.description().toStdString(),
                .categoryId = dialog.categoryId(),
                .type = dialog.transactionType(),
                .amount = dialog.amount(),
                .date = Date::fromString(dialog.date().toStdString()),
                .merchant = dialog.merchant().toStdString(),
            });
        } catch (const std::exception& error) {
            showOperationError(QStringLiteral("Add Transaction"), error);
            return;
        }

        try {
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
        } catch (const std::exception& error) {
            QMessageBox::warning(
                this,
                QStringLiteral("Budget Alerts"),
                QStringLiteral("The transaction was saved, but budget alerts could not be sent.\n\n%1")
                    .arg(QString::fromUtf8(error.what())));
        }
        refreshData();
        emit dataChanged();
    } else if (dialog.categoriesChanged()) {
        refreshData();
        emit dataChanged();
    }
}

void TransactionsWindow::onEditTransaction() {
    if (!ensureSignedIn(QStringLiteral("Edit Transaction"))) {
        return;
    }

    const auto transaction = selectedTransaction();
    if (!transaction) {
        QMessageBox::information(this, "Edit Transaction", "Please select a transaction first.");
        return;
    }

    TransactionDialog dialog(backend_, userId_, this);
    dialog.setDialogTitle("Edit Transaction");
    try {
        dialog.setAvailableCategories(backend_.transactions().getCategoriesForUser(userId_));
    } catch (const std::exception& error) {
        showOperationError(QStringLiteral("Edit Transaction"), error);
        return;
    }
    dialog.setDate(QString::fromStdString(transaction->date.toString()));
    dialog.setTitle(QString::fromStdString(transaction->title));
    dialog.setType(transaction->type == TransactionType::Income ? "Income" : "Expense");
    dialog.setCategoryId(transaction->categoryId);
    dialog.setAmount(transaction->amount);
    dialog.setDescription(QString::fromStdString(transaction->description));
    dialog.setMerchant(QString::fromStdString(transaction->merchant));

    const int result = dialog.exec();
    if (dialog.categoriesChanged()) {
        populateCategoryFilter();
    }

    if (result == QDialog::Accepted) {
        try {
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
        } catch (const std::exception& error) {
            showOperationError(QStringLiteral("Edit Transaction"), error);
            return;
        }
        refreshData();
        emit dataChanged();
    } else if (dialog.categoriesChanged()) {
        refreshData();
        emit dataChanged();
    }
}

void TransactionsWindow::onDeleteTransaction() {
    if (!ensureSignedIn(QStringLiteral("Delete Transaction"))) {
        return;
    }

    const auto transaction = selectedTransaction();
    if (!transaction) {
        QMessageBox::information(this, "Delete Transaction", "Please select a transaction first.");
        return;
    }

    const auto confirm = QMessageBox::question(
        this,
        QStringLiteral("Delete Transaction"),
        QStringLiteral("Delete \"%1\" permanently?").arg(QString::fromStdString(transaction->title)));
    if (confirm != QMessageBox::Yes) {
        return;
    }

    try {
        backend_.transactions().deleteTransaction(userId_, transaction->id);
    } catch (const std::exception& error) {
        showOperationError(QStringLiteral("Delete Transaction"), error);
        return;
    }
    refreshData();
    emit dataChanged();
}

std::optional<Transaction> TransactionsWindow::selectedTransaction() {
    const int currentRow = transactionsTable->currentRow();
    if (currentRow < 0 || transactionsTable->item(currentRow, 0) == nullptr) {
        return std::nullopt;
    }

    const std::string transactionId =
        transactionsTable->item(currentRow, 0)->data(Qt::UserRole).toString().toStdString();
    try {
        for (const auto& transaction : backend_.transactions().listTransactions(userId_)) {
            if (transaction.id == transactionId) {
                return transaction;
            }
        }
    } catch (const std::exception& error) {
        showOperationError(QStringLiteral("Select Transaction"), error);
        return std::nullopt;
    }
    return std::nullopt;
}
