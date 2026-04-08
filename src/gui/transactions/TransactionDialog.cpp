#include "gui/transactions/TransactionDialog.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QTextEdit>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QDate>

using namespace finsight::core::models;

TransactionDialog::TransactionDialog(QWidget *parent) : QDialog(parent) {
    setupUi();
}

void TransactionDialog::setupUi() {
    setWindowTitle("Add Transaction");
    resize(400, 300);

    auto *mainLayout = new QVBoxLayout(this);
    auto *formLayout = new QFormLayout();

    titleEdit = new QLineEdit();
    merchantEdit = new QLineEdit();

    typeCombo = new QComboBox();
    typeCombo->addItems({"Income", "Expense"});

    categoryCombo = new QComboBox();

    dateEdit = new QDateEdit();
    dateEdit->setCalendarPopup(true);
    dateEdit->setDate(QDate::currentDate());

    amountSpin = new QDoubleSpinBox();
    amountSpin->setRange(0.01, 100000000.0);
    amountSpin->setDecimals(2);
    amountSpin->setSingleStep(10.0);

    descriptionEdit = new QTextEdit();
    descriptionEdit->setPlaceholderText("Optional description");

    formLayout->addRow("Title:", titleEdit);
    formLayout->addRow("Merchant:", merchantEdit);
    formLayout->addRow("Type:", typeCombo);
    formLayout->addRow("Category:", categoryCombo);
    formLayout->addRow("Date:", dateEdit);
    formLayout->addRow("Amount:", amountSpin);
    formLayout->addRow("Description:", descriptionEdit);

    mainLayout->addLayout(formLayout);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &TransactionDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &TransactionDialog::reject);
    connect(typeCombo, &QComboBox::currentTextChanged, this, [this]() {
        refreshCategoryChoices();
    });

    mainLayout->addWidget(buttonBox);
}

QString TransactionDialog::title() const {
    return titleEdit->text().trimmed();
}

QString TransactionDialog::type() const {
    return typeCombo->currentText();
}

QString TransactionDialog::category() const {
    return categoryCombo->currentText();
}

QString TransactionDialog::merchant() const {
    return merchantEdit->text().trimmed();
}

QString TransactionDialog::date() const {
    return dateEdit->date().toString("yyyy-MM-dd");
}

double TransactionDialog::amount() const {
    return amountSpin->value();
}

QString TransactionDialog::description() const {
    return descriptionEdit->toPlainText().trimmed();
}

std::string TransactionDialog::categoryId() const {
    return categoryCombo->currentData().toString().toStdString();
}

TransactionType TransactionDialog::transactionType() const {
    return type() == "Income" ? TransactionType::Income : TransactionType::Expense;
}

void TransactionDialog::setAvailableCategories(const std::vector<Category>& categories) {
    categories_ = categories;
    refreshCategoryChoices();
}

void TransactionDialog::setDialogTitle(const QString &text) {
    setWindowTitle(text);
}

void TransactionDialog::setTitle(const QString &value) {
    titleEdit->setText(value);
}

void TransactionDialog::setType(const QString &value) {
    int index = typeCombo->findText(value);
    if (index >= 0) {
        typeCombo->setCurrentIndex(index);
    }
}

void TransactionDialog::setCategory(const QString &value) {
    int index = categoryCombo->findText(value);
    if (index >= 0) {
        categoryCombo->setCurrentIndex(index);
    }
}

void TransactionDialog::setCategoryId(const std::string& value) {
    int index = categoryCombo->findData(QString::fromStdString(value));
    if (index >= 0) {
        categoryCombo->setCurrentIndex(index);
    }
}

void TransactionDialog::setMerchant(const QString &value) {
    merchantEdit->setText(value);
}

void TransactionDialog::setDate(const QString &value) {
    QDate parsed = QDate::fromString(value, "yyyy-MM-dd");
    if (parsed.isValid()) {
        dateEdit->setDate(parsed);
    }
}

void TransactionDialog::setAmount(double value) {
    amountSpin->setValue(value);
}

void TransactionDialog::setDescription(const QString &value) {
    descriptionEdit->setPlainText(value);
}

void TransactionDialog::accept() {
    if (title().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Title cannot be empty.");
        return;
    }

    if (amount() <= 0.0) {
        QMessageBox::warning(this, "Validation Error", "Amount must be greater than 0.");
        return;
    }
    if (categoryCombo->currentIndex() < 0 || categoryId().empty()) {
        QMessageBox::warning(this, "Validation Error", "Please choose a category.");
        return;
    }

    QDialog::accept();
}

void TransactionDialog::refreshCategoryChoices() {
    const QString selectedCategoryId = categoryCombo->currentData().toString();
    const auto selectedKind = transactionType() == TransactionType::Income ? CategoryKind::Income : CategoryKind::Expense;

    categoryCombo->clear();
    for (const auto& categoryEntry : categories_) {
        if (categoryEntry.kind == selectedKind) {
            categoryCombo->addItem(QString::fromStdString(categoryEntry.name),
                                   QString::fromStdString(categoryEntry.id));
        }
    }

    int index = categoryCombo->findData(selectedCategoryId);
    if (index >= 0) {
        categoryCombo->setCurrentIndex(index);
    }
}
