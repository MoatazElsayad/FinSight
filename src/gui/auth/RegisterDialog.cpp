#include "gui/auth/RegisterDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

RegisterDialog::RegisterDialog(QWidget *parent) : QDialog(parent) {
    setupUi();
}

QString RegisterDialog::fullName() const {
    return fullNameEdit->text().trimmed();
}

QString RegisterDialog::email() const {
    return emailEdit->text().trimmed();
}

QString RegisterDialog::phone() const {
    return phoneEdit->text().trimmed();
}

QString RegisterDialog::gender() const {
    return genderCombo->currentText();
}

QString RegisterDialog::password() const {
    return passwordEdit->text();
}

QString RegisterDialog::confirmPassword() const {
    return passwordConfirmEdit->text();
}

void RegisterDialog::accept() {
    if (fullName().isEmpty() || email().isEmpty() || password().isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Missing fields"),
                             QStringLiteral("Full name, email, and password are required."));
        return;
    }
    const QString em = email();
    const int at = em.indexOf(QLatin1Char('@'));
    if (at <= 0 || at == em.size() - 1 || !em.contains(QLatin1Char('.'))) {
        QMessageBox::warning(this, QStringLiteral("Invalid email"),
                             QStringLiteral("Enter a valid email address (e.g. name@example.com)."));
        return;
    }
    if (password().size() < 8) {
        QMessageBox::warning(this, QStringLiteral("Weak password"),
                             QStringLiteral("Password must be at least 8 characters."));
        return;
    }
    if (password() != confirmPassword()) {
        QMessageBox::warning(this, QStringLiteral("Password mismatch"),
                             QStringLiteral("Password and confirmation do not match."));
        return;
    }
    QDialog::accept();
}

void RegisterDialog::setupUi() {
    setWindowTitle(QStringLiteral("Create account"));
    resize(460, 520);
    setModal(true);

    setStyleSheet(
        "RegisterDialog {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "             stop:0 #0a1428, stop:1 #0f1a33);"
        "  color: #e5e9f4;"
        "}"
        "QLineEdit, QComboBox {"
        "  background-color: #1a2f5a;"
        "  border: 1px solid #2a4080;"
        "  border-radius: 10px;"
        "  color: #e5e9f4;"
        "  padding: 10px 14px;"
        "  font-size: 14px;"
        "  min-height: 22px;"
        "}"
        "QLineEdit:focus, QComboBox:focus {"
        "  border: 2px solid #5b8cff;"
        "  background-color: #203050;"
        "}"
    );

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 36, 40, 32);
    mainLayout->setSpacing(20);

    auto *title = new QLabel(QStringLiteral("Create your account"));
    title->setStyleSheet(
        QStringLiteral("font-size: 22px; font-weight: 800; color: #ffffff; letter-spacing: -0.3px;"));
    auto *subtitle = new QLabel(
        QStringLiteral("All data is stored locally on this device after you sign up."));
    subtitle->setStyleSheet(QStringLiteral("font-size: 12px; color: #8fa3bf;"));
    subtitle->setWordWrap(true);
    mainLayout->addWidget(title);
    mainLayout->addWidget(subtitle);

    auto *card = new QFrame();
    card->setStyleSheet(
        "QFrame {"
        "  background-color: rgba(20, 26, 39, 0.92);"
        "  border: 1px solid #2b3245;"
        "  border-radius: 14px;"
        "}");
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(22, 20, 22, 20);

    auto *formLayout = new QFormLayout();
    formLayout->setSpacing(14);
    formLayout->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    formLayout->setHorizontalSpacing(12);

    auto labelStyle = [](const QString& text) {
        auto *l = new QLabel(text);
        l->setStyleSheet(QStringLiteral("color: #b0bac9; font-size: 12px; font-weight: 600;"));
        return l;
    };

    fullNameEdit = new QLineEdit();
    fullNameEdit->setPlaceholderText(QStringLiteral("Your name"));
    emailEdit = new QLineEdit();
    emailEdit->setPlaceholderText(QStringLiteral("you@example.com"));
    phoneEdit = new QLineEdit();
    phoneEdit->setPlaceholderText(QStringLiteral("Optional"));
    genderCombo = new QComboBox();
    genderCombo->addItems(
        {QStringLiteral("Prefer not to say"), QStringLiteral("Male"), QStringLiteral("Female")});
    passwordEdit = new QLineEdit();
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setPlaceholderText(QStringLiteral("At least 8 characters"));
    passwordConfirmEdit = new QLineEdit();
    passwordConfirmEdit->setEchoMode(QLineEdit::Password);
    passwordConfirmEdit->setPlaceholderText(QStringLiteral("Re-enter password"));

    formLayout->addRow(labelStyle(QStringLiteral("Full name")), fullNameEdit);
    formLayout->addRow(labelStyle(QStringLiteral("Email")), emailEdit);
    formLayout->addRow(labelStyle(QStringLiteral("Phone")), phoneEdit);
    formLayout->addRow(labelStyle(QStringLiteral("Gender")), genderCombo);
    formLayout->addRow(labelStyle(QStringLiteral("Password")), passwordEdit);
    formLayout->addRow(labelStyle(QStringLiteral("Confirm")), passwordConfirmEdit);

    cardLayout->addLayout(formLayout);
    mainLayout->addWidget(card);

    auto *buttonBox = new QDialogButtonBox();
    auto *createBtn = new QPushButton(QStringLiteral("Create account"));
    createBtn->setDefault(true);
    createBtn->setMinimumHeight(46);
    createBtn->setStyleSheet(
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #5b8cff, stop:1 #4a7ae6);"
        "  color: #ffffff; border: none; border-radius: 10px;"
        "  font-size: 15px; font-weight: 700; padding: 0 28px;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #6b9cff, stop:1 #5a8af6);"
        "}");
    buttonBox->addButton(createBtn, QDialogButtonBox::AcceptRole);
    buttonBox->addButton(QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &RegisterDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    mainLayout->addWidget(buttonBox);
}
