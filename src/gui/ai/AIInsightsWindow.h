#ifndef AIINSIGHTSWINDOW_H
#define AIINSIGHTSWINDOW_H

#include "core/managers/FinanceTrackerBackend.h"

#include <QWidget>

class QLabel;
class QListWidget;
class QTextEdit;
class QComboBox;
class QPushButton;

class AIInsightsWindow : public QWidget {
    Q_OBJECT

public:
    explicit AIInsightsWindow(finsight::core::managers::FinanceTrackerBackend& backend,
                              const std::string& userId,
                              QWidget *parent = nullptr);

    void setUserId(const std::string& userId);
    void refreshData();

private:
    finsight::core::managers::FinanceTrackerBackend& backend_;
    std::string userId_;

    QLabel *statusLabel;
    QLabel *keyWinValue;
    QLabel *budgetPerformanceValue;
    QListWidget *topActionsList;
    QTextEdit *chatHistory;
    QComboBox *periodSelector;

    void setupUi();
    void applyAnalysis(double netSavings, double savingsRate, double largestCategorySpend);
};

#endif
