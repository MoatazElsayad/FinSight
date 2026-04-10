#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "core/managers/FinanceTrackerBackend.h"

#include <QMainWindow>
#include <optional>
#include <string>

class QStackedWidget;
class QPushButton;
class QFrame;
class QLabel;
class QString;
class DashboardWindow;
class TransactionsWindow;
class BudgetsWindow;
class ProfileWindow;
class AIInsightsWindow;
class LoginDialog;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(finsight::core::managers::FinanceTrackerBackend& backend,
                        QWidget *parent = nullptr);

private:
    finsight::core::managers::FinanceTrackerBackend& backend_;
    std::string userId_;
    std::optional<std::string> activeSessionToken_;

    QStackedWidget *stack;
    QFrame *navFrame;
    QFrame *topBar;
    QLabel *pageLabel;
    LoginDialog *loginPage;
    DashboardWindow *dashboardPage;
    TransactionsWindow *transactionsPage;
    BudgetsWindow *budgetsPage;
    ProfileWindow *profilePage;
    AIInsightsWindow *aiInsightsPage;

    QPushButton *dashboardButton;
    QPushButton *transactionsButton;
    QPushButton *budgetsButton;
    QPushButton *profileButton;
    QPushButton *aiInsightsButton;
    QPushButton *logoutButton;

    void setupUi();
    void connectSignals();
    void refreshPages();
    void setCurrentUser(const std::string& userId);
    void clearCurrentUser();
    void showMainInterface(const QString& userId);
    void showLoginPage();
    static finsight::core::models::Date today();
};

#endif
