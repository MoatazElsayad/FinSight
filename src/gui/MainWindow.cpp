#include "gui/MainWindow.h"
#include "gui/dashboard/DashboardWindow.h"
#include "gui/transactions/TransactionsWindow.h"
#include "gui/budgets/BudgetsWindow.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QStackedWidget>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setupUi();
    connectSignals();
}

void MainWindow::setupUi() {
    auto *central = new QWidget(this);
    auto *mainLayout = new QHBoxLayout(central);

    auto *navLayout = new QVBoxLayout();
    dashboardButton = new QPushButton("Dashboard");
    transactionsButton = new QPushButton("Transactions");
    budgetsButton = new QPushButton("Budgets");

    navLayout->addWidget(dashboardButton);
    navLayout->addWidget(transactionsButton);
    navLayout->addWidget(budgetsButton);
    navLayout->addStretch();

    stack = new QStackedWidget();

    dashboardPage = new DashboardWindow();
    transactionsPage = new TransactionsWindow();
    budgetsPage = new BudgetsWindow();

    stack->addWidget(dashboardPage);
    stack->addWidget(transactionsPage);
    stack->addWidget(budgetsPage);

    mainLayout->addLayout(navLayout);
    mainLayout->addWidget(stack, 1);

    setCentralWidget(central);
    setWindowTitle("FinSight");
    resize(1000, 600);
}

void MainWindow::connectSignals() {
    connect(dashboardButton, &QPushButton::clicked, this, [this]() {
        stack->setCurrentWidget(dashboardPage);
    });

    connect(transactionsButton, &QPushButton::clicked, this, [this]() {
        stack->setCurrentWidget(transactionsPage);
    });

    connect(budgetsButton, &QPushButton::clicked, this, [this]() {
        stack->setCurrentWidget(budgetsPage);
    });
}
