#include "gui/MainWindow.h"
#include "gui/auth/LoginDialog.h"
#include "gui/auth/RegisterDialog.h"
#include "gui/profile/ProfileWindow.h"
#include "gui/ai/AIInsightsWindow.h"
#include "gui/savings/SavingsWindow.h"
#include "gui/investments/InvestmentsWindow.h"
#include "gui/goals/GoalsWindow.h"
#include "gui/reports/ReportsWindow.h"
#include "gui/receipts/ReceiptsWindow.h"
#include "gui/categories/CategoriesWindow.h"
#include "gui/dashboard/DashboardWindow.h"
#include "gui/transactions/TransactionsWindow.h"
#include "gui/budgets/BudgetsWindow.h"

#include <QDate>
#include <QMessageBox>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QStackedWidget>
#include <QFrame>

MainWindow::MainWindow(finsight::core::managers::FinanceTrackerBackend& backend,
                       QWidget *parent)
    : QMainWindow(parent),
      backend_(backend) {
    setupUi();
    connectSignals();
    clearCurrentUser();
}

void MainWindow::setupUi() {
    auto *central = new QWidget(this);
    auto *mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    setStyleSheet(
        "QMainWindow, QWidget { background-color: #090e1c; color: #e5e9f4; }"
        "QPushButton {"
        "  background-color: #11192b;"
        "  border: 1px solid #27314a;"
        "  border-radius: 10px;"
        "  color: #dce4f8;"
        "  text-align: left;"
        "  padding: 10px 12px;"
        "  font-weight: 600;"
        "}"
        "QPushButton:hover { background-color: #1a2742; }"
        "QPushButton:pressed { background-color: #24385f; }"
    );

    auto *navLayout = new QVBoxLayout();
    navLayout->setContentsMargins(14, 14, 14, 14);
    navLayout->setSpacing(8);
    dashboardButton = new QPushButton("Dashboard");
    transactionsButton = new QPushButton("Transactions");
    budgetsButton = new QPushButton("Budgets");
    profileButton = new QPushButton("Profile");
    aiInsightsButton = new QPushButton("AI Insights");
    savingsButton = new QPushButton("Savings");
    investmentsButton = new QPushButton("Investments");
    goalsButton = new QPushButton("Goals");
    reportsButton = new QPushButton("Reports");
    receiptsButton = new QPushButton("Receipts / OCR");
    categoriesButton = new QPushButton("Categories");
    logoutButton = new QPushButton("Logout");

    navLayout->addWidget(dashboardButton);
    navLayout->addWidget(transactionsButton);
    navLayout->addWidget(budgetsButton);
    navLayout->addWidget(profileButton);
    navLayout->addWidget(aiInsightsButton);
    navLayout->addWidget(savingsButton);
    navLayout->addWidget(investmentsButton);
    navLayout->addWidget(goalsButton);
    navLayout->addWidget(reportsButton);
    navLayout->addWidget(receiptsButton);
    navLayout->addWidget(categoriesButton);
    navLayout->addSpacing(10);
    navLayout->addWidget(logoutButton);
    navLayout->addStretch();

    auto *navFrame = new QFrame();
    navFrame->setStyleSheet(
        "QFrame {"
        "  background-color: #0d1425;"
        "  border-right: 1px solid #25304a;"
        "}"
    );
    navFrame->setLayout(navLayout);

    stack = new QStackedWidget();

    dashboardPage = new DashboardWindow(backend_, userId_);
    transactionsPage = new TransactionsWindow(backend_, userId_);
    budgetsPage = new BudgetsWindow(backend_, userId_);
    profilePage = new ProfileWindow(backend_, userId_);
    aiInsightsPage = new AIInsightsWindow(backend_, userId_);
    savingsPage = new SavingsWindow(backend_, userId_);
    investmentsPage = new InvestmentsWindow(backend_, userId_);
    goalsPage = new GoalsWindow(backend_, userId_);
    reportsPage = new ReportsWindow(backend_, userId_);
    receiptsPage = new ReceiptsWindow(backend_, userId_);
    categoriesPage = new CategoriesWindow(backend_, userId_);

    stack->addWidget(dashboardPage);
    stack->addWidget(transactionsPage);
    stack->addWidget(budgetsPage);
    stack->addWidget(profilePage);
    stack->addWidget(aiInsightsPage);
    stack->addWidget(savingsPage);
    stack->addWidget(investmentsPage);
    stack->addWidget(goalsPage);
    stack->addWidget(reportsPage);
    stack->addWidget(receiptsPage);
    stack->addWidget(categoriesPage);

    mainLayout->addWidget(navFrame);
    mainLayout->addWidget(stack, 1);

    setCentralWidget(central);
    setWindowTitle("FinSight");
    resize(1320, 780);
}

void MainWindow::connectSignals() {
    connect(dashboardButton, &QPushButton::clicked, this, [this]() {
        dashboardPage->refreshData();
        stack->setCurrentWidget(dashboardPage);
    });

    connect(transactionsButton, &QPushButton::clicked, this, [this]() {
        transactionsPage->refreshData();
        stack->setCurrentWidget(transactionsPage);
    });

    connect(budgetsButton, &QPushButton::clicked, this, [this]() {
        budgetsPage->refreshData();
        stack->setCurrentWidget(budgetsPage);
    });
    connect(profileButton, &QPushButton::clicked, this, [this]() {
        profilePage->refreshData();
        stack->setCurrentWidget(profilePage);
    });
    connect(aiInsightsButton, &QPushButton::clicked, this, [this]() {
        aiInsightsPage->refreshData();
        stack->setCurrentWidget(aiInsightsPage);
    });
    connect(savingsButton, &QPushButton::clicked, this, [this]() { savingsPage->refreshData(); stack->setCurrentWidget(savingsPage); });
    connect(investmentsButton, &QPushButton::clicked, this, [this]() { investmentsPage->refreshData(); stack->setCurrentWidget(investmentsPage); });
    connect(goalsButton, &QPushButton::clicked, this, [this]() { goalsPage->refreshData(); stack->setCurrentWidget(goalsPage); });
    connect(reportsButton, &QPushButton::clicked, this, [this]() { reportsPage->refreshData(); stack->setCurrentWidget(reportsPage); });
    connect(receiptsButton, &QPushButton::clicked, this, [this]() { receiptsPage->refreshData(); stack->setCurrentWidget(receiptsPage); });
    connect(categoriesButton, &QPushButton::clicked, this, [this]() { categoriesPage->refreshData(); stack->setCurrentWidget(categoriesPage); });
    connect(logoutButton, &QPushButton::clicked, this, [this]() {
        clearCurrentUser();
        if (!promptForAuthentication()) {
            close();
        }
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
    connect(savingsPage, &SavingsWindow::dataChanged, this, [this]() { refreshPages(); });
    connect(investmentsPage, &InvestmentsWindow::dataChanged, this, [this]() { refreshPages(); });
    connect(goalsPage, &GoalsWindow::dataChanged, this, [this]() { refreshPages(); });
    connect(receiptsPage, &ReceiptsWindow::dataChanged, this, [this]() { refreshPages(); });
    connect(categoriesPage, &CategoriesWindow::dataChanged, this, [this]() { refreshPages(); });
}

void MainWindow::refreshPages() {
    dashboardPage->refreshData();
    transactionsPage->refreshData();
    budgetsPage->refreshData();
    profilePage->refreshData();
    aiInsightsPage->refreshData();
    savingsPage->refreshData();
    investmentsPage->refreshData();
    goalsPage->refreshData();
    reportsPage->refreshData();
    receiptsPage->refreshData();
    categoriesPage->refreshData();
}

bool MainWindow::promptForAuthentication() {
    while (true) {
        LoginDialog loginDialog(this);
        const int result = loginDialog.exec();
        if (result != QDialog::Accepted) {
            return false;
        }

        if (loginDialog.wantsRegister()) {
            RegisterDialog registerDialog(this);
            if (registerDialog.exec() != QDialog::Accepted) {
                continue;
            }

            try {
                const auto user = backend_.auth().registerUser(
                    registerDialog.fullName().toStdString(),
                    registerDialog.email().toStdString(),
                    registerDialog.phone().toStdString(),
                    registerDialog.gender().toStdString(),
                    registerDialog.password().toStdString(),
                    today());
                setCurrentUser(user.id);
                return true;
            } catch (const std::exception& error) {
                QMessageBox::warning(this, "Register Failed", error.what());
                continue;
            }
        }

        const auto user = backend_.auth().login(
            loginDialog.email().toStdString(),
            loginDialog.password().toStdString());
        if (!user) {
            QMessageBox::warning(this, "Login Failed", "Invalid email or password.");
            continue;
        }

        setCurrentUser(user->id);
        return true;
    }
}

void MainWindow::setCurrentUser(const std::string& userId) {
    userId_ = userId;
    const auto session = backend_.sessions().startSession(userId_, today());
    activeSessionToken_ = session.token;
    dashboardPage->setUserId(userId_);
    transactionsPage->setUserId(userId_);
    budgetsPage->setUserId(userId_);
    profilePage->setUserId(userId_);
    aiInsightsPage->setUserId(userId_);
    savingsPage->setUserId(userId_);
    investmentsPage->setUserId(userId_);
    goalsPage->setUserId(userId_);
    reportsPage->setUserId(userId_);
    receiptsPage->setUserId(userId_);
    categoriesPage->setUserId(userId_);
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
    aiInsightsPage->setUserId(userId_);
    savingsPage->setUserId(userId_);
    investmentsPage->setUserId(userId_);
    goalsPage->setUserId(userId_);
    reportsPage->setUserId(userId_);
    receiptsPage->setUserId(userId_);
    categoriesPage->setUserId(userId_);
    refreshPages();
}

finsight::core::models::Date MainWindow::today() {
    const auto currentDate = QDate::currentDate();
    return finsight::core::models::Date {currentDate.year(), currentDate.month(), currentDate.day()};
}
