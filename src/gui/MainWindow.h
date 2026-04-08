#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "core/managers/FinanceTrackerBackend.h"

#include <QMainWindow>
#include <optional>
#include <string>

class QStackedWidget;
class QPushButton;
class DashboardWindow;
class TransactionsWindow;
class BudgetsWindow;
class ProfileWindow;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(finsight::core::managers::FinanceTrackerBackend& backend,
                        QWidget *parent = nullptr);

    bool promptForAuthentication();

private:
    finsight::core::managers::FinanceTrackerBackend& backend_;
    std::string userId_;
    std::optional<std::string> activeSessionToken_;

    QStackedWidget *stack;
    DashboardWindow *dashboardPage;
    TransactionsWindow *transactionsPage;
    BudgetsWindow *budgetsPage;
    ProfileWindow *profilePage;

    QPushButton *dashboardButton;
    QPushButton *transactionsButton;
    QPushButton *budgetsButton;
    QPushButton *profileButton;
    QPushButton *logoutButton;

    void setupUi();
    void connectSignals();
    void refreshPages();
    void setCurrentUser(const std::string& userId);
    void clearCurrentUser();
    static finsight::core::models::Date today();
};

#endif
