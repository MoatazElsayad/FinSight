#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "core/managers/FinanceTrackerBackend.h"

#include <QMainWindow>
#include <filesystem>
#include <optional>
#include <string>

namespace finsight::data::storage {
class BackendStore;
}

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
class CategoriesWindow;
class ReceiptsWindow;
class ReportsWindow;
class GoalsWindow;
class InvestmentsWindow;
class SavingsWindow;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(finsight::core::managers::FinanceTrackerBackend& backend,
                        finsight::data::storage::BackendStore* persistStore,
                        std::filesystem::path persistDirectory,
                        QWidget *parent = nullptr);

private:
    finsight::core::managers::FinanceTrackerBackend& backend_;
    finsight::data::storage::BackendStore *persistStore_ {nullptr};
    std::filesystem::path persistDirectory_;
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
    SavingsWindow *savingsPage;
    InvestmentsWindow *investmentsPage;
    GoalsWindow *goalsPage;
    ReportsWindow *reportsPage;
    ReceiptsWindow *receiptsPage;
    CategoriesWindow *categoriesPage;

    QPushButton *dashboardButton;
    QPushButton *transactionsButton;
    QPushButton *budgetsButton;
    QPushButton *profileButton;
    QPushButton *aiInsightsButton;
    QPushButton *savingsButton;
    QPushButton *investmentsButton;
    QPushButton *goalsButton;
    QPushButton *reportsButton;
    QPushButton *receiptsButton;
    QPushButton *categoriesButton;
    QPushButton *logoutButton;

    void setupUi();
    void connectSignals();
    void refreshPages();
    void setCurrentUser(const std::string& userId);
    void clearCurrentUser();
    void showMainInterface(const QString& userId);
    void showLoginPage();
    void persistNow();
    static finsight::core::models::Date today();
};

#endif
