#include "gui/auth/RegisterDialog.h"

#include "gui/FinSightUi.h"

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

    setStyleSheet(finsight::gui::ui::dialogStyle(QStringLiteral("RegisterDialog")));

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 36, 40, 32);
    mainLayout->setSpacing(20);

    auto *title = new QLabel(QStringLiteral("Create your account"));
    title->setStyleSheet(QStringLiteral("font-size: 22px; font-weight: 800; color: #ffffff;"));
    auto *subtitle = new QLabel(
        QStringLiteral("All data is stored locally on this device after you sign up."));
    subtitle->setStyleSheet(finsight::gui::ui::mutedTextStyle());
    subtitle->setWordWrap(true);
    mainLayout->addWidget(title);
    mainLayout->addWidget(subtitle);

    auto *card = new QFrame();
    card->setObjectName(QStringLiteral("finCard"));
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(22, 20, 22, 20);

    auto *formLayout = new QFormLayout();
    formLayout->setSpacing(14);
    formLayout->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    formLayout->setHorizontalSpacing(12);

    auto labelStyle = [](const QString& text) {
        auto *l = new QLabel(text);
        l->setStyleSheet(finsight::gui::ui::labelStyle());
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
    createBtn->setStyleSheet(finsight::gui::ui::primaryButtonStyle());
    buttonBox->addButton(createBtn, QDialogButtonBox::AcceptRole);
    auto *cancelButton = buttonBox->addButton(QDialogButtonBox::Cancel);
    cancelButton->setStyleSheet(finsight::gui::ui::ghostButtonStyle());

    connect(buttonBox, &QDialogButtonBox::accepted, this, &RegisterDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    mainLayout->addWidget(buttonBox);
}
