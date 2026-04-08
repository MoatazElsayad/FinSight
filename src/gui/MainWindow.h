#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QStackedWidget;
class QPushButton;
class DashboardWindow;
class TransactionsWindow;
class BudgetsWindow;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    QStackedWidget *stack;
    DashboardWindow *dashboardPage;
    TransactionsWindow *transactionsPage;
    BudgetsWindow *budgetsPage;

    QPushButton *dashboardButton;
    QPushButton *transactionsButton;
    QPushButton *budgetsButton;

    void setupUi();
    void connectSignals();
};

#endif
