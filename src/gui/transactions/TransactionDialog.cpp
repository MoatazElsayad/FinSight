#include "gui/transactions/TransactionDialog.h"

#include "gui/FinSightUi.h"

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
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include <exception>

using namespace finsight::core::models;

TransactionDialog::TransactionDialog(finsight::core::managers::FinanceTrackerBackend& backend,
                                     const std::string& userId,
                                     QWidget *parent)
    : QDialog(parent),
      backend_(backend),
      userId_(userId) {
    setupUi();
}

void TransactionDialog::setupUi() {
    setWindowTitle("Add Transaction");
    resize(520, 560);
    setStyleSheet(finsight::gui::ui::dialogStyle(QStringLiteral("TransactionDialog")));

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 22, 24, 20);
    mainLayout->setSpacing(16);

    auto *titleLabel = new QLabel(QStringLiteral("Transaction details"));
    titleLabel->setStyleSheet(finsight::gui::ui::cardTitleStyle());
    auto *subtitleLabel = new QLabel(QStringLiteral("Add the transaction details, or create a matching category before saving."));
    subtitleLabel->setWordWrap(true);
    subtitleLabel->setStyleSheet(finsight::gui::ui::mutedTextStyle());
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(subtitleLabel);

    auto *formLayout = new QFormLayout();
    formLayout->setHorizontalSpacing(14);
    formLayout->setVerticalSpacing(12);

    titleEdit = new QLineEdit();
    merchantEdit = new QLineEdit();

    typeCombo = new QComboBox();
    typeCombo->addItems({"Income", "Expense"});

    categoryCombo = new QComboBox();
    addCategoryButton = new QPushButton("Add category");
    addCategoryButton->setStyleSheet(finsight::gui::ui::secondaryButtonStyle());
    auto *categoryRow = new QHBoxLayout();
    categoryRow->setContentsMargins(0, 0, 0, 0);
    categoryRow->addWidget(categoryCombo, 1);
    categoryRow->addWidget(addCategoryButton);

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
    formLayout->addRow("Category:", categoryRow);
    formLayout->addRow("Date:", dateEdit);
    formLayout->addRow("Amount:", amountSpin);
    formLayout->addRow("Description:", descriptionEdit);

    mainLayout->addLayout(formLayout);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->button(QDialogButtonBox::Ok)->setText(QStringLiteral("Save"));
    buttonBox->button(QDialogButtonBox::Ok)->setStyleSheet(finsight::gui::ui::primaryButtonStyle());
    buttonBox->button(QDialogButtonBox::Cancel)->setStyleSheet(finsight::gui::ui::ghostButtonStyle());
    connect(buttonBox, &QDialogButtonBox::accepted, this, &TransactionDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &TransactionDialog::reject);
    connect(typeCombo, &QComboBox::currentTextChanged, this, [this]() {
        refreshCategoryChoices();
    });
    connect(addCategoryButton, &QPushButton::clicked, this, &TransactionDialog::addCategory);

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

bool TransactionDialog::categoriesChanged() const {
    return categoriesChanged_;
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

void TransactionDialog::addCategory() {
    if (userId_.empty()) {
        QMessageBox::warning(this, "Category Error", "Please log in before adding categories.");
        return;
    }

    QDialog categoryDialog(this);
    categoryDialog.setWindowTitle(QStringLiteral("Add Category"));
    categoryDialog.setModal(true);
    categoryDialog.resize(420, 240);
    categoryDialog.setStyleSheet(finsight::gui::ui::dialogStyle(QStringLiteral("QDialog")));

    auto *layout = new QVBoxLayout(&categoryDialog);
    layout->setContentsMargins(22, 20, 22, 18);
    layout->setSpacing(14);

    auto *header = new QLabel(QStringLiteral("Create category"));
    header->setStyleSheet(finsight::gui::ui::cardTitleStyle());
    auto *hint = new QLabel(QStringLiteral("The category will be added for the selected transaction type."));
    hint->setWordWrap(true);
    hint->setStyleSheet(finsight::gui::ui::mutedTextStyle());
    layout->addWidget(header);
    layout->addWidget(hint);

    auto *form = new QFormLayout();
    form->setHorizontalSpacing(12);
    form->setVerticalSpacing(12);

    auto *nameEdit = new QLineEdit();
    nameEdit->setPlaceholderText(QStringLiteral("Example: Groceries"));
    auto *iconEdit = new QLineEdit(transactionType() == TransactionType::Income
                                       ? QStringLiteral("wallet")
                                       : QStringLiteral("tag"));
    iconEdit->setPlaceholderText(QStringLiteral("Example: tag"));

    form->addRow(QStringLiteral("Name"), nameEdit);
    form->addRow(QStringLiteral("Icon"), iconEdit);
    layout->addLayout(form);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("Add category"));
    buttons->button(QDialogButtonBox::Ok)->setStyleSheet(finsight::gui::ui::primaryButtonStyle());
    buttons->button(QDialogButtonBox::Cancel)->setStyleSheet(finsight::gui::ui::ghostButtonStyle());
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, &categoryDialog, [&]() {
        if (nameEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(&categoryDialog, QStringLiteral("Category Error"), QStringLiteral("Category name is required."));
            return;
        }
        if (iconEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(&categoryDialog, QStringLiteral("Category Error"), QStringLiteral("Icon name is required."));
            return;
        }
        categoryDialog.accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, &categoryDialog, &QDialog::reject);

    if (categoryDialog.exec() != QDialog::Accepted) {
        return;
    }

    const QString name = nameEdit->text().trimmed();
    const QString icon = iconEdit->text().trimmed();

    try {
        const auto category = backend_.transactions().createCategory(
            userId_,
            name.toStdString(),
            transactionType() == TransactionType::Income ? CategoryKind::Income : CategoryKind::Expense,
            icon.toStdString(),
            false);
        categories_ = backend_.transactions().getCategoriesForUser(userId_);
        categoriesChanged_ = true;
        refreshCategoryChoices();
        setCategoryId(category.id);
        QMessageBox::information(this, "Category Added", "The new category is ready for this transaction.");
    } catch (const std::exception& error) {
        QMessageBox::warning(this, "Category Error", error.what());
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
