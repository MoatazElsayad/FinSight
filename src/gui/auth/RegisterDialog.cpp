#include "gui/auth/RegisterDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
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

void RegisterDialog::accept() {
    if (fullName().isEmpty() || email().isEmpty() || password().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Full name, email, and password are required.");
        return;
    }
    QDialog::accept();
}

void RegisterDialog::setupUi() {
    setWindowTitle("Register");
    resize(400, 280);

    auto *mainLayout = new QVBoxLayout(this);
    auto *formLayout = new QFormLayout();

    fullNameEdit = new QLineEdit();
    emailEdit = new QLineEdit();
    phoneEdit = new QLineEdit();
    genderCombo = new QComboBox();
    genderCombo->addItems({"Prefer not to say", "Male", "Female"});
    passwordEdit = new QLineEdit();
    passwordEdit->setEchoMode(QLineEdit::Password);

    formLayout->addRow("Full name:", fullNameEdit);
    formLayout->addRow("Email:", emailEdit);
    formLayout->addRow("Phone:", phoneEdit);
    formLayout->addRow("Gender:", genderCombo);
    formLayout->addRow("Password:", passwordEdit);
    mainLayout->addLayout(formLayout);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->button(QDialogButtonBox::Ok)->setText("Create Account");
    connect(buttonBox, &QDialogButtonBox::accepted, this, &RegisterDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);
}
