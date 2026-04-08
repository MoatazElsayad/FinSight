#include "gui/auth/LoginDialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent) {
    setupUi();
}

QString LoginDialog::email() const {
    return emailEdit->text().trimmed();
}

QString LoginDialog::password() const {
    return passwordEdit->text();
}

bool LoginDialog::wantsRegister() const {
    return wantsRegister_;
}

void LoginDialog::setupUi() {
    setWindowTitle("Login");
    resize(360, 220);

    auto *mainLayout = new QVBoxLayout(this);
    auto *titleLabel = new QLabel("Sign in to FinSight");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    mainLayout->addWidget(titleLabel);

    auto *formLayout = new QFormLayout();
    emailEdit = new QLineEdit();
    passwordEdit = new QLineEdit();
    passwordEdit->setEchoMode(QLineEdit::Password);
    formLayout->addRow("Email:", emailEdit);
    formLayout->addRow("Password:", passwordEdit);
    mainLayout->addLayout(formLayout);

    auto *buttonRow = new QHBoxLayout();
    auto *registerButton = new QPushButton("Register");
    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->button(QDialogButtonBox::Ok)->setText("Login");

    connect(registerButton, &QPushButton::clicked, this, [this]() {
        wantsRegister_ = true;
        accept();
    });
    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        wantsRegister_ = false;
        accept();
    });
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    buttonRow->addWidget(registerButton);
    buttonRow->addStretch();
    buttonRow->addWidget(buttonBox);
    mainLayout->addLayout(buttonRow);
}
