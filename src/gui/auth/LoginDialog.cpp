#include "gui/auth/LoginDialog.h"
#include "gui/auth/RegisterDialog.h"

#include <QDate>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QPixmap>
#include <QPainter>

// Conditionally include SVG support if available
#ifdef QT_SVG_LIB
#include <QSvgRenderer>
#endif

LoginDialog::LoginDialog(finsight::core::managers::FinanceTrackerBackend& backend, QWidget *parent)
    : QWidget(parent), backend_(backend) {
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
    // Main layout with gradient background
    setStyleSheet(
        "QWidget { "
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "             stop:0 #0a1428, stop:1 #0f1a33); "
        "  color: #e5e9f4; "
        "}"
    );

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Create a centered container
    auto *centeringLayout = new QHBoxLayout();
    centeringLayout->addStretch();

    auto *contentWidget = new QWidget();
    auto *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(48, 48, 48, 48);
    contentLayout->setSpacing(32);

    // Header with logo and title
    auto *headerLayout = new QVBoxLayout();
    headerLayout->setSpacing(16);
    headerLayout->setAlignment(Qt::AlignHCenter);

    auto *logoLabel = new QLabel();
    logoLabel->setAlignment(Qt::AlignCenter);
    
#ifdef QT_SVG_LIB
    // Load SVG logo if available
    QSvgRenderer renderer("assets/logo.svg");
    QPixmap pixmap(120, 120);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    renderer.render(&painter);
    logoLabel->setPixmap(pixmap.scaledToWidth(120, Qt::SmoothTransformation));
#else
    // Fallback: Use styled text logo
    logoLabel->setText("F");
    logoLabel->setStyleSheet(
        "QLabel {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "               stop:0 #5b8cff, stop:1 #7ba3ff);"
        "  color: #ffffff;"
        "  border-radius: 12px;"
        "  font-size: 72px;"
        "  font-weight: 900;"
        "  text-align: center;"
        "}"
    );
#endif
    logoLabel->setMaximumWidth(120);
    logoLabel->setMaximumHeight(120);

    auto *titleLabel = new QLabel("FinSight");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "font-size: 28px; "
        "font-weight: 800; "
        "color: #ffffff; "
        "background: transparent; "
    );

    auto *subtitleLabel = new QLabel("Financial Management Made Simple");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setStyleSheet(
        "font-size: 12px; "
        "color: #8fa3bf; "
        "background: transparent; "
    );

    headerLayout->addWidget(logoLabel, 0, Qt::AlignHCenter);
    headerLayout->addWidget(titleLabel, 0, Qt::AlignHCenter);
    headerLayout->addWidget(subtitleLabel, 0, Qt::AlignHCenter);
    contentLayout->addLayout(headerLayout);

    // Form section
    auto *formWidget = new QWidget();
    auto *formLayout = new QVBoxLayout(formWidget);
    formLayout->setSpacing(16);

    auto *emailLabel = new QLabel("Email Address");
    emailLabel->setStyleSheet(
        "font-size: 12px; "
        "font-weight: 600; "
        "color: #b0bac9; "
        "background: transparent; "
        "letter-spacing: 0.5px; "
    );

    emailEdit = new QLineEdit();
    emailEdit->setMinimumHeight(44);
    emailEdit->setStyleSheet(
        "QLineEdit { "
        "  background-color: #1a2f5a; "
        "  border: 1px solid #2a4080; "
        "  border-radius: 10px; "
        "  color: #e5e9f4; "
        "  padding: 12px 16px; "
        "  font-size: 14px; "
        "} "
        "QLineEdit:focus { "
        "  border: 2px solid #5b8cff; "
        "  background-color: #203050; "
        "} "
        "QLineEdit::placeholder { color: #6b7a94; } "
    );
    emailEdit->setPlaceholderText("your.email@example.com");

    auto *passwordLabel = new QLabel("Password");
    passwordLabel->setStyleSheet(
        "font-size: 12px; "
        "font-weight: 600; "
        "color: #b0bac9; "
        "background: transparent; "
        "letter-spacing: 0.5px; "
    );

    passwordEdit = new QLineEdit();
    passwordEdit->setMinimumHeight(44);
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setStyleSheet(
        "QLineEdit { "
        "  background-color: #1a2f5a; "
        "  border: 1px solid #2a4080; "
        "  border-radius: 10px; "
        "  color: #e5e9f4; "
        "  padding: 12px 16px; "
        "  font-size: 14px; "
        "} "
        "QLineEdit:focus { "
        "  border: 2px solid #5b8cff; "
        "  background-color: #203050; "
        "} "
        "QLineEdit::placeholder { color: #6b7a94; } "
    );
    passwordEdit->setPlaceholderText("Enter your password");

    formLayout->addWidget(emailLabel);
    formLayout->addWidget(emailEdit);
    formLayout->addWidget(passwordLabel);
    formLayout->addWidget(passwordEdit);
    formLayout->addSpacing(8);

    auto *rememberLabel = new QLabel("Remember me");
    rememberLabel->setStyleSheet(
        "font-size: 12px; "
        "color: #8fa3bf; "
        "background: transparent; "
    );
    formLayout->addWidget(rememberLabel);

    contentLayout->addWidget(formWidget);

    // Buttons section
    auto *buttonLayout = new QVBoxLayout();
    buttonLayout->setSpacing(12);

    auto *loginButton = new QPushButton("Sign In");
    loginButton->setMinimumHeight(48);
    loginButton->setStyleSheet(
        "QPushButton { "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "               stop:0 #5b8cff, stop:1 #4a7ae6); "
        "  color: #ffffff; "
        "  border: none; "
        "  border-radius: 10px; "
        "  font-size: 16px; "
        "  font-weight: 700; "
        "  letter-spacing: 0.5px; "
        "} "
        "QPushButton:hover { "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "               stop:0 #6b9cff, stop:1 #5a8af6); "
        "} "
        "QPushButton:pressed { "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "               stop:0 #4a7ae6, stop:1 #3a6ad6); "
        "} "
    );

    auto *registerButton = new QPushButton("Create Account");
    registerButton->setMinimumHeight(48);
    registerButton->setStyleSheet(
        "QPushButton { "
        "  background-color: transparent; "
        "  color: #5b8cff; "
        "  border: 2px solid #5b8cff; "
        "  border-radius: 10px; "
        "  font-size: 16px; "
        "  font-weight: 700; "
        "  letter-spacing: 0.5px; "
        "} "
        "QPushButton:hover { "
        "  background-color: rgba(91, 140, 255, 0.1); "
        "  border: 2px solid #7ba3ff; "
        "  color: #7ba3ff; "
        "} "
    );

    connect(loginButton, &QPushButton::clicked, this, &LoginDialog::attemptLogin);

    connect(registerButton, &QPushButton::clicked, this, [this]() {
        wantsRegister_ = true;
        emit registerRequested();
    });

    buttonLayout->addWidget(loginButton);
    buttonLayout->addWidget(registerButton);
    contentLayout->addLayout(buttonLayout);

    // Footer
    auto *footerLabel = new QLabel("By signing in, you agree to our Terms of Service");
    footerLabel->setAlignment(Qt::AlignCenter);
    footerLabel->setStyleSheet(
        "font-size: 11px; "
        "color: #6b7a94; "
        "background: transparent; "
    );
    contentLayout->addWidget(footerLabel);

    centeringLayout->addWidget(contentWidget);
    centeringLayout->addStretch();
    mainLayout->addStretch();
    mainLayout->addLayout(centeringLayout);
    mainLayout->addStretch();
}

void LoginDialog::attemptLogin() {
    wantsRegister_ = false;

    try {
        const auto user = backend_.auth().login(
            email().toStdString(),
            password().toStdString());

        if (!user) {
            QMessageBox::warning(this, "Login Failed", "Invalid email or password.");
            return;
        }

        emit loginSuccessful(QString::fromStdString(user->id));
    } catch (const std::exception& error) {
        QMessageBox::warning(this, "Login Failed", error.what());
    }
}
