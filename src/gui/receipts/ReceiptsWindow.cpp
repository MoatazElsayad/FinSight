#include "gui/receipts/ReceiptsWindow.h"

#include "gui/FinSightUi.h"

#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextEdit>
#include <QVBoxLayout>

#include <exception>

using namespace finsight::core::models;

namespace {

Date toDate(const QDate& date) {
    return Date {date.year(), date.month(), date.day()};
}

QDate toQDate(const Date& date) {
    return QDate(date.year, date.month, date.day);
}

QString money(double value) {
    return finsight::gui::ui::formatMoney(value);
}

QString statusText(ReceiptStatus status) {
    switch (status) {
    case ReceiptStatus::Uploaded:
        return QStringLiteral("Uploaded");
    case ReceiptStatus::Parsed:
        return QStringLiteral("Parsed");
    case ReceiptStatus::Confirmed:
        return QStringLiteral("Confirmed");
    }
    return QStringLiteral("Uploaded");
}

QLabel* formLabel(const QString& text) {
    auto* label = new QLabel(text);
    label->setStyleSheet(finsight::gui::ui::labelStyle());
    return label;
}

QLabel* parsedValueLabel(const QString& text = QStringLiteral("-")) {
    auto* label = new QLabel(text);
    label->setStyleSheet(QStringLiteral(
        "background-color: #0f1527;"
        "border: 1px solid #2b3245;"
        "border-radius: 10px;"
        "color: #e5e9f4;"
        "padding: 8px 10px;"
        "font-size: 12px;"));
    label->setWordWrap(true);
    return label;
}

}  // namespace

ReceiptsWindow::ReceiptsWindow(finsight::core::managers::FinanceTrackerBackend& backend,
                               const std::string& userId,
                               QWidget* parent)
    : QWidget(parent),
      backend_(backend),
      userId_(userId) {
    setupUi();
    refreshData();
}

void ReceiptsWindow::setUserId(const std::string& userId) {
    userId_ = userId;
    refreshCategoryCombo();
    refreshData();
}

void ReceiptsWindow::setupUi() {
    setStyleSheet(finsight::gui::ui::pageStyle(QStringLiteral("ReceiptsWindow")));

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(20);

    auto* title = new QLabel(QStringLiteral("Receipt Parser"));
    title->setStyleSheet(finsight::gui::ui::titleStyle());
    auto* subtitle = new QLabel(QStringLiteral("Paste receipt text or load a text file, review the parsed result, then confirm it as a transaction."));
    subtitle->setStyleSheet(finsight::gui::ui::subtitleStyle());
    subtitle->setWordWrap(true);
    mainLayout->addWidget(title);
    mainLayout->addWidget(subtitle);

    auto* uploadCard = new QFrame();
    uploadCard->setObjectName("finCard");
    auto* uploadLayout = new QVBoxLayout(uploadCard);
    uploadLayout->setContentsMargins(20, 18, 20, 18);
    auto* uploadTitle = new QLabel(QStringLiteral("Upload or paste receipt text"));
    uploadTitle->setStyleSheet(finsight::gui::ui::cardTitleStyle());
    uploadLayout->addWidget(uploadTitle);

    auto* fileRow = new QHBoxLayout();
    fileEdit = new QLineEdit(QStringLiteral("receipt.txt"));
    auto* browseButton = new QPushButton(QStringLiteral("Select file"));
    browseButton->setStyleSheet(finsight::gui::ui::secondaryButtonStyle());
    auto* parseButton = new QPushButton(QStringLiteral("Parse text"));
    parseButton->setStyleSheet(finsight::gui::ui::primaryButtonStyle());
    fileRow->addWidget(fileEdit, 1);
    fileRow->addWidget(browseButton);
    fileRow->addWidget(parseButton);
    uploadLayout->addLayout(fileRow);

    rawTextEdit = new QTextEdit();
    rawTextEdit->setPlaceholderText(QStringLiteral("Paste receipt text here. Dates work best in YYYY-MM-DD format."));
    rawTextEdit->setMinimumHeight(120);
    uploadLayout->addWidget(rawTextEdit);
    mainLayout->addWidget(uploadCard);

    auto* confirmCard = new QFrame();
    confirmCard->setObjectName("finCard");
    auto* confirmLayout = new QVBoxLayout(confirmCard);
    confirmLayout->setContentsMargins(20, 18, 20, 18);
    auto* confirmTitle = new QLabel(QStringLiteral("Parsed transaction"));
    confirmTitle->setStyleSheet(finsight::gui::ui::cardTitleStyle());
    parsedSummaryLabel = new QLabel(QStringLiteral("No parsed receipt yet."));
    parsedSummaryLabel->setStyleSheet(QStringLiteral("color: #9ca6bf; font-size: 12px;"));
    parsedSummaryLabel->setWordWrap(true);
    confirmLayout->addWidget(confirmTitle);
    confirmLayout->addWidget(parsedSummaryLabel);

    auto* parsedGrid = new QGridLayout();
    parsedGrid->setHorizontalSpacing(16);
    parsedGrid->setVerticalSpacing(10);
    parsedMerchantValue = parsedValueLabel();
    parsedAmountValue = parsedValueLabel();
    parsedDateValue = parsedValueLabel();
    parsedNotesValue = parsedValueLabel();
    parsedGrid->addWidget(formLabel(QStringLiteral("Parsed merchant")), 0, 0);
    parsedGrid->addWidget(parsedMerchantValue, 0, 1);
    parsedGrid->addWidget(formLabel(QStringLiteral("Parsed amount")), 0, 2);
    parsedGrid->addWidget(parsedAmountValue, 0, 3);
    parsedGrid->addWidget(formLabel(QStringLiteral("Parsed date")), 1, 0);
    parsedGrid->addWidget(parsedDateValue, 1, 1);
    parsedGrid->addWidget(formLabel(QStringLiteral("Notes")), 1, 2);
    parsedGrid->addWidget(parsedNotesValue, 1, 3);
    confirmLayout->addLayout(parsedGrid);

    auto* grid = new QGridLayout();
    grid->setHorizontalSpacing(16);
    grid->setVerticalSpacing(12);
    titleEdit = new QLineEdit();
    titleEdit->setPlaceholderText(QStringLiteral("Transaction title"));
    merchantEdit = new QLineEdit();
    merchantEdit->setPlaceholderText(QStringLiteral("Merchant"));
    categoryCombo = new QComboBox();
    amountSpin = new QDoubleSpinBox();
    amountSpin->setRange(0.01, 1000000000.0);
    amountSpin->setDecimals(2);
    dateEdit = new QDateEdit(QDate::currentDate());
    dateEdit->setCalendarPopup(true);
    auto* confirmButton = new QPushButton(QStringLiteral("Confirm as transaction"));
    confirmButton->setStyleSheet(finsight::gui::ui::primaryButtonStyle());

    grid->addWidget(formLabel(QStringLiteral("Title")), 0, 0);
    grid->addWidget(titleEdit, 0, 1);
    grid->addWidget(formLabel(QStringLiteral("Merchant")), 0, 2);
    grid->addWidget(merchantEdit, 0, 3);
    grid->addWidget(formLabel(QStringLiteral("Category")), 1, 0);
    grid->addWidget(categoryCombo, 1, 1);
    grid->addWidget(formLabel(QStringLiteral("Amount")), 1, 2);
    grid->addWidget(amountSpin, 1, 3);
    grid->addWidget(formLabel(QStringLiteral("Date")), 2, 0);
    grid->addWidget(dateEdit, 2, 1);
    grid->addWidget(confirmButton, 2, 3);
    confirmLayout->addLayout(grid);
    mainLayout->addWidget(confirmCard);

    auto* tableCard = new QFrame();
    tableCard->setObjectName("finCard");
    auto* tableLayout = new QVBoxLayout(tableCard);
    tableLayout->setContentsMargins(16, 16, 16, 16);
    auto* tableTitle = new QLabel(QStringLiteral("Receipt history"));
    tableTitle->setStyleSheet(finsight::gui::ui::cardTitleStyle());
    receiptsTable = new QTableWidget();
    receiptsTable->setColumnCount(5);
    receiptsTable->setHorizontalHeaderLabels({
        QStringLiteral("File"),
        QStringLiteral("Status"),
        QStringLiteral("Uploaded"),
        QStringLiteral("Parsed amount"),
        QStringLiteral("Id")
    });
    finsight::gui::ui::prepareTable(receiptsTable);
    receiptsTable->setColumnHidden(4, true);
    tableLayout->addWidget(tableTitle);
    tableLayout->addWidget(receiptsTable, 1);
    mainLayout->addWidget(tableCard, 1);

    refreshCategoryCombo();

    connect(browseButton, &QPushButton::clicked, this, &ReceiptsWindow::browseReceiptFile);
    connect(parseButton, &QPushButton::clicked, this, &ReceiptsWindow::uploadAndParse);
    connect(confirmButton, &QPushButton::clicked, this, &ReceiptsWindow::confirmTransaction);
}

void ReceiptsWindow::browseReceiptFile() {
    const QString path = QFileDialog::getOpenFileName(this,
                                                      QStringLiteral("Select Receipt Text"),
                                                      QString(),
                                                      QStringLiteral("Text files (*.txt);;All files (*)"));
    if (path.isEmpty()) {
        return;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, QStringLiteral("Receipt Error"), QStringLiteral("Could not read the selected file."));
        return;
    }
    fileEdit->setText(QFileInfo(path).fileName());
    rawTextEdit->setPlainText(QString::fromUtf8(file.readAll()));
}

void ReceiptsWindow::uploadAndParse() {
    if (userId_.empty()) {
        QMessageBox::warning(this, QStringLiteral("Receipt Error"), QStringLiteral("Please log in first."));
        return;
    }

    try {
        const auto receipt = backend_.receipts().uploadReceipt(userId_,
                                                               fileEdit->text().trimmed().toStdString(),
                                                               rawTextEdit->toPlainText().trimmed().toStdString(),
                                                               toDate(QDate::currentDate()));
        const auto parsed = backend_.receipts().parseReceipt(userId_, receipt.id, backend_.transactions());
        parsedReceiptId_ = parsed.receiptId;

        const QString merchant = QString::fromStdString(parsed.merchant);
        titleEdit->setText(merchant.isEmpty() ? QStringLiteral("Receipt transaction") : merchant);
        merchantEdit->setText(merchant);
        amountSpin->setValue(parsed.amount.value_or(0.01));
        dateEdit->setDate(parsed.transactionDate ? toQDate(*parsed.transactionDate) : QDate::currentDate());
        selectCategory(parsed.suggestedCategoryId);

        parsedSummaryLabel->setText(QStringLiteral("Review the parsed fields, adjust anything needed, then confirm."));
        parsedMerchantValue->setText(merchant.isEmpty() ? QStringLiteral("Unknown") : merchant);
        parsedAmountValue->setText(parsed.amount ? money(*parsed.amount) : QStringLiteral("Unknown"));
        parsedDateValue->setText(parsed.transactionDate ? QString::fromStdString(parsed.transactionDate->toString())
                                                        : QStringLiteral("Unknown"));
        parsedNotesValue->setText(QString::fromStdString(parsed.confidenceNotes));

        refreshData();
        emit dataChanged();
    } catch (const std::exception& error) {
        QMessageBox::warning(this, QStringLiteral("Receipt Error"), error.what());
    }
}

void ReceiptsWindow::confirmTransaction() {
    if (parsedReceiptId_.empty()) {
        QMessageBox::information(this, QStringLiteral("Confirm Receipt"), QStringLiteral("Parse receipt text first."));
        return;
    }
    if (categoryCombo->currentData().toString().isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Confirm Receipt"), QStringLiteral("Please choose a category."));
        return;
    }

    try {
        ReceiptConfirmation confirmation {
            .receiptId = parsedReceiptId_,
            .title = titleEdit->text().trimmed().toStdString(),
            .description = "Imported from receipt " + parsedReceiptId_,
            .merchant = merchantEdit->text().trimmed().toStdString(),
            .categoryId = categoryCombo->currentData().toString().toStdString(),
            .type = TransactionType::Expense,
            .amount = amountSpin->value(),
            .date = toDate(dateEdit->date()),
        };
        backend_.receipts().confirmReceiptAsTransaction(userId_, confirmation, backend_.transactions());
        parsedReceiptId_.clear();
        parsedSummaryLabel->setText(QStringLiteral("Receipt confirmed as a transaction."));
        parsedMerchantValue->setText(QStringLiteral("-"));
        parsedAmountValue->setText(QStringLiteral("-"));
        parsedDateValue->setText(QStringLiteral("-"));
        parsedNotesValue->setText(QStringLiteral("-"));
        refreshData();
        emit dataChanged();
        QMessageBox::information(this, QStringLiteral("Receipt Saved"), QStringLiteral("Transaction created from receipt."));
    } catch (const std::exception& error) {
        QMessageBox::warning(this, QStringLiteral("Receipt Error"), error.what());
    }
}

void ReceiptsWindow::refreshCategoryCombo() {
    const QString selected = categoryCombo ? categoryCombo->currentData().toString() : QString();
    categoryCombo->clear();
    for (const auto& category : backend_.transactions().getCategoriesForUser(userId_)) {
        if (category.kind == CategoryKind::Expense) {
            categoryCombo->addItem(QString::fromStdString(category.name), QString::fromStdString(category.id));
        }
    }
    const int index = categoryCombo->findData(selected);
    if (index >= 0) {
        categoryCombo->setCurrentIndex(index);
    }
}

void ReceiptsWindow::selectCategory(const std::string& categoryId) {
    int index = categoryCombo->findData(QString::fromStdString(categoryId));
    if (index < 0 && categoryCombo->count() > 0) {
        index = 0;
    }
    if (index >= 0) {
        categoryCombo->setCurrentIndex(index);
    }
}

void ReceiptsWindow::refreshData() {
    refreshCategoryCombo();
    receiptsTable->setRowCount(0);
    if (userId_.empty()) {
        return;
    }

    for (const auto& receipt : backend_.receipts().listReceipts(userId_)) {
        const auto parsed = backend_.receipts().findParsedReceipt(receipt.id);
        const int row = receiptsTable->rowCount();
        receiptsTable->insertRow(row);
        receiptsTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(receipt.fileName)));
        receiptsTable->setItem(row, 1, new QTableWidgetItem(statusText(receipt.status)));
        receiptsTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(receipt.uploadedAt.toString())));
        receiptsTable->setItem(row, 3, new QTableWidgetItem(parsed && parsed->amount ? money(*parsed->amount)
                                                                                      : QStringLiteral("-")));
        receiptsTable->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(receipt.id)));
    }
}
