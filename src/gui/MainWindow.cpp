#include "gui/MainWindow.h"
#include "gui/auth/LoginDialog.h"
#include "gui/auth/RegisterDialog.h"
#include "gui/profile/ProfileWindow.h"
#include "gui/dashboard/DashboardWindow.h"
#include "gui/transactions/TransactionsWindow.h"
#include "gui/budgets/BudgetsWindow.h"

#include <QDate>
#include <QLabel>
#include <QMessageBox>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QStackedWidget>
#include <QFrame>
#include <QApplication>
#include <QButtonGroup>
#include <QSizePolicy>
#include <QScreen>
#include <QPixmap>
#include <QPainter>

// Conditionally include SVG support if available
#ifdef QT_SVG_LIB
#include <QSvgRenderer>
#endif

MainWindow::MainWindow(finsight::core::managers::FinanceTrackerBackend& backend,
                       QWidget *parent)
    : QMainWindow(parent),
      backend_(backend) {
    setupUi();
    connectSignals();
    showLoginPage();  // Start with login page
}

void MainWindow::setupUi() {
    auto *central = new QWidget(this);
    auto *mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    setStyleSheet(
        "QMainWindow, QWidget { background-color: #0a1428; color: #e5e9f4; }"
        "QPushButton {"
        "  background-color: transparent;"
        "  border: 1px solid transparent;"
        "  border-radius: 14px;"
        "  color: #dce4f8;"
        "  text-align: left;"
        "  padding: 14px 12px;"
        "  font-weight: 600;"
        "  font-size: 14px;"
        "}"
        "QPushButton:hover { background-color: #1a2f5a; }"
        "QPushButton:pressed { background-color: #233d72; }"
        "QPushButton:checked {"
        "  background-color: #2e5aa6;"
        "  border-color: #5b8cff;"
        "  color: #ffffff;"
        "}"
        "QFrame#navigationPanel {"
        "  background-color: #0f1a33;"
        "  border-right: 1px solid #1a2f5a;"
        "}"
    );

    auto *navLayout = new QVBoxLayout();
    navLayout->setContentsMargins(16, 16, 16, 16);
    navLayout->setSpacing(12);

    auto *logoLabel = new QLabel();
    logoLabel->setAlignment(Qt::AlignCenter);
    
#ifdef QT_SVG_LIB
    // Load SVG logo if available
    QSvgRenderer renderer("assets/logo.svg");
    QPixmap pixmap(52, 52);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    renderer.render(&painter);
    logoLabel->setPixmap(pixmap.scaledToWidth(52, Qt::SmoothTransformation));
#else
    // Fallback: Use styled text logo
    logoLabel->setText("F");
    logoLabel->setStyleSheet(
        "QLabel {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "               stop:0 #5b8cff, stop:1 #7ba3ff);"
        "  color: #ffffff;"
        "  border-radius: 6px;"
        "  font-size: 24px;"
        "  font-weight: 900;"
        "  text-align: center;"
        "}"
    );
#endif
    logoLabel->setFixedSize(52, 52);

    dashboardButton = new QPushButton("Dashboard");
    transactionsButton = new QPushButton("Transactions");
    budgetsButton = new QPushButton("Budgets");
    profileButton = new QPushButton("Profile");
    logoutButton = new QPushButton("Logout");

    dashboardButton->setCheckable(true);
    transactionsButton->setCheckable(true);
    budgetsButton->setCheckable(true);
    profileButton->setCheckable(true);

    auto *navGroup = new QButtonGroup(this);
    navGroup->setExclusive(true);
    navGroup->addButton(dashboardButton);
    navGroup->addButton(transactionsButton);
    navGroup->addButton(budgetsButton);
    navGroup->addButton(profileButton);
    dashboardButton->setChecked(true);

    navLayout->addWidget(logoLabel);
    navLayout->addSpacing(4);
    navLayout->addWidget(dashboardButton);
    navLayout->addWidget(transactionsButton);
    navLayout->addWidget(budgetsButton);
    navLayout->addWidget(profileButton);
    navLayout->addSpacing(16);
    navLayout->addWidget(logoutButton);
    navLayout->addStretch();

    navFrame = new QFrame();
    navFrame->setObjectName("navigationPanel");
    navFrame->setFixedWidth(120);
    navFrame->setStyleSheet(
        "QFrame {"
        "  background-color: #0f1a33;"
        "  border-right: 1px solid #1a2f5a;"
        "}"
    );
    navFrame->setLayout(navLayout);

    stack = new QStackedWidget();

    loginPage = new LoginDialog(backend_, this);
    dashboardPage = new DashboardWindow(backend_, userId_);
    transactionsPage = new TransactionsWindow(backend_, userId_);
    budgetsPage = new BudgetsWindow(backend_, userId_);
    profilePage = new ProfileWindow(backend_, userId_);

    stack->addWidget(loginPage);
    stack->addWidget(dashboardPage);
    stack->addWidget(transactionsPage);
    stack->addWidget(budgetsPage);
    stack->addWidget(profilePage);

    auto *rightSide = new QWidget();
    auto *rightLayout = new QVBoxLayout(rightSide);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    topBar = new QFrame();
    topBar->setObjectName("topBar");
    topBar->setStyleSheet(
        "QFrame#topBar {"
        "  background-color: #0b1221;"
        "  border-bottom: 1px solid #1c2a48;"
        "}"
    );
    topBar->setFixedHeight(72);

    auto *topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(24, 0, 24, 0);

    auto *currentViewLayout = new QVBoxLayout();
    currentViewLayout->setSpacing(2);
    auto *viewLabel = new QLabel("CURRENT VIEW");
    viewLabel->setStyleSheet("color: #8ec1ff; font-size: 10px; letter-spacing: 1.2px; font-weight: 700;");
    pageLabel = new QLabel("Dashboard");
    pageLabel->setObjectName("pageLabel");
    pageLabel->setStyleSheet("color: #ffffff; font-size: 20px; font-weight: 800;");
    currentViewLayout->addWidget(viewLabel);
    currentViewLayout->addWidget(pageLabel);

    auto *topBarSpacer = new QWidget();
    topBarSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    auto *actionsLayout = new QHBoxLayout();
    actionsLayout->setSpacing(10);
    auto *fullscreenButton = new QPushButton("⌖");
    auto *addButton = new QPushButton("＋");
    auto *themeButton = new QPushButton("☀");
    for (auto *button : {fullscreenButton, addButton, themeButton}) {
        button->setFixedSize(42, 42);
        button->setStyleSheet(
            "QPushButton {"
            "  background-color: #11192b;"
            "  border: 1px solid #25304a;"
            "  border-radius: 12px;"
            "  color: #ffffff;"
            "  font-size: 18px;"
            "}"
            "QPushButton:hover { background-color: #1a2742; }"
        );
    }
    actionsLayout->addWidget(fullscreenButton);
    actionsLayout->addWidget(addButton);
    actionsLayout->addWidget(themeButton);

    auto *profileLabel = new QLabel("Moataz");
    profileLabel->setStyleSheet("color: #ffffff; font-weight: 700; font-size: 13px; margin-left: 18px;");
    auto *profileBadge = new QLabel("EGP 28,721");
    profileBadge->setStyleSheet(
        "background-color: #2a3b6e;"
        "color: #f5b642;"
        "border-radius: 12px;"
        "padding: 8px 12px;"
        "font-weight: 700;"
    );

    topBarLayout->addLayout(currentViewLayout);
    topBarLayout->addWidget(topBarSpacer);
    topBarLayout->addLayout(actionsLayout);
    topBarLayout->addWidget(profileLabel, 0, Qt::AlignVCenter);
    topBarLayout->addWidget(profileBadge, 0, Qt::AlignVCenter);

    rightLayout->addWidget(topBar);
    rightLayout->addWidget(stack, 1);

    mainLayout->addWidget(navFrame);
    mainLayout->addWidget(rightSide, 1);

    setCentralWidget(central);
    setWindowTitle("FinSight");

    // Make window fullscreen
    showFullScreen();
}

void MainWindow::connectSignals() {
    connect(loginPage, &LoginDialog::loginSuccessful, this, &MainWindow::showMainInterface);
    connect(loginPage, &LoginDialog::registerRequested, this, [this]() {
        // Handle registration
        RegisterDialog registerDialog(this);
        if (registerDialog.exec() == QDialog::Accepted) {
            try {
                const auto user = backend_.auth().registerUser(
                    registerDialog.fullName().toStdString(),
                    registerDialog.email().toStdString(),
                    registerDialog.phone().toStdString(),
                    registerDialog.gender().toStdString(),
                    registerDialog.password().toStdString(),
                    today());
                showMainInterface(QString::fromStdString(user.id));
            } catch (const std::exception& error) {
                QMessageBox::warning(this, "Register Failed", error.what());
            }
        }
    });

    connect(dashboardButton, &QPushButton::clicked, this, [this]() {
        pageLabel->setText("Dashboard");
        dashboardPage->refreshData();
        stack->setCurrentWidget(dashboardPage);
    });

    connect(transactionsButton, &QPushButton::clicked, this, [this]() {
        pageLabel->setText("Transactions");
        transactionsPage->refreshData();
        stack->setCurrentWidget(transactionsPage);
    });

    connect(budgetsButton, &QPushButton::clicked, this, [this]() {
        pageLabel->setText("Budgets");
        budgetsPage->refreshData();
        stack->setCurrentWidget(budgetsPage);
    });
    connect(profileButton, &QPushButton::clicked, this, [this]() {
        pageLabel->setText("Profile");
        profilePage->refreshData();
        stack->setCurrentWidget(profilePage);
    });
    connect(logoutButton, &QPushButton::clicked, this, [this]() {
        clearCurrentUser();
        showLoginPage();
    });

    connect(transactionsPage, &TransactionsWindow::dataChanged, this, [this]() {
        refreshPages();
    });
    connect(budgetsPage, &BudgetsWindow::dataChanged, this, [this]() {
        refreshPages();
    });
    connect(profilePage, &ProfileWindow::profileUpdated, this, [this]() {
        refreshPages();
    });
}

void MainWindow::refreshPages() {
    dashboardPage->refreshData();
    transactionsPage->refreshData();
    budgetsPage->refreshData();
    profilePage->refreshData();
}

void MainWindow::setCurrentUser(const std::string& userId) {
    userId_ = userId;
    const auto session = backend_.sessions().startSession(userId_, today());
    activeSessionToken_ = session.token;
    dashboardPage->setUserId(userId_);
    transactionsPage->setUserId(userId_);
    budgetsPage->setUserId(userId_);
    profilePage->setUserId(userId_);
    refreshPages();
    stack->setCurrentWidget(dashboardPage);
}

void MainWindow::clearCurrentUser() {
    if (activeSessionToken_.has_value()) {
        backend_.sessions().endSession(*activeSessionToken_);
    }
    activeSessionToken_.reset();
    userId_.clear();
    dashboardPage->setUserId(userId_);
    transactionsPage->setUserId(userId_);
    budgetsPage->setUserId(userId_);
    profilePage->setUserId(userId_);
    refreshPages();
}

finsight::core::models::Date MainWindow::today() {
    const auto currentDate = QDate::currentDate();
    return finsight::core::models::Date {currentDate.year(), currentDate.month(), currentDate.day()};
}

void MainWindow::showLoginPage() {
    // Hide navigation and top bar when showing login page
    if (navFrame) {
        navFrame->hide();
    }
    if (topBar) {
        topBar->hide();
    }
    stack->setCurrentWidget(loginPage);
}

void MainWindow::showMainInterface(const QString& userId) {
    setCurrentUser(userId.toStdString());
    // Show navigation and top bar when showing main interface
    if (navFrame) {
        navFrame->show();
    }
    if (topBar) {
        topBar->show();
    }
    stack->setCurrentWidget(dashboardPage);
}
