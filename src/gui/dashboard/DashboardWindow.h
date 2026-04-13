#ifndef DASHBOARDWINDOW_H
#define DASHBOARDWINDOW_H

#include "core/managers/FinanceTrackerBackend.h"
#include "core/models/Common.h"

#include <QWidget>

#include <QTextEdit>
#include <QTimer>

class QLabel;
class QTableWidget;
class QListWidget;
class QComboBox;
class QPushButton;
class QButtonGroup;
class QFrame;
class QGraphicsOpacityEffect;
class QPropertyAnimation;

class DashboardWindow : public QWidget {
    Q_OBJECT

public:
    explicit DashboardWindow(finsight::core::managers::FinanceTrackerBackend& backend,
                             const std::string& userId,
                             QWidget *parent = nullptr);

    void setUserId(const std::string& userId);
    void refreshData();

private:
    enum class TimeRange {
        Monthly,
        Yearly,
        Overall
    };

    finsight::core::managers::FinanceTrackerBackend& backend_;
    std::string userId_;

    TimeRange activeTimeRange_ {TimeRange::Monthly};

    QLabel *incomeValueLabel;
    QLabel *expensesValueLabel;
    QLabel *liquidCashValueLabel;
    QLabel *savingsRateValueLabel;

    QPushButton *monthlyFilterButton;
    QPushButton *yearlyFilterButton;
    QPushButton *overallFilterButton;
    QButtonGroup *timeFilterGroup;
    QComboBox *monthSelector;

    QTableWidget *recentTransactionsTable;

    // AI Summary components
    QPushButton *generateAISummaryButton;
    QLabel *aiSummaryTitle;
    QTextEdit *aiSummaryText;
    QListWidget *aiRecommendationsList;
    QLabel *aiStatusLabel;
    QLabel *aiModelLabel;

    QFrame *aiLoadingFrame {nullptr};
    QLabel *aiSparklesLabel {nullptr};
    QLabel *aiLoadingSubLabel {nullptr};
    QTimer aiProgressWatchdog_;
    QGraphicsOpacityEffect *aiSparklesOpacity_ {nullptr};
    QPropertyAnimation *aiSparklesPulse_ {nullptr};

    int aiRunGeneration_ {0};
    int aiWatchdogGeneration_ {0};
    bool aiStreamProgressSeenForRun_ {false};
    bool aiInsightAppliedForRun_ {false};
    bool aiFallbackJobStarted_ {false};
    finsight::core::models::YearMonth aiPendingYearMonth_ {};

    void setupUi();
    QWidget *createSummaryCard(const QString &title, QLabel *&valueLabel, const QString& accentColor);
    void configureMonthSelector();
    bool selectedYearMonth(finsight::core::models::YearMonth& period) const;
    bool isTransactionInScope(const finsight::core::models::Transaction& transaction,
                              const finsight::core::models::YearMonth& period) const;
    void generateAISummary();
    void onAiProgressWatchdogTimeout();
    void beginAiSummaryGeneration(const finsight::core::models::YearMonth& period);
    void dispatchAiStreamEvent(const QString& event, const QString& model, const QString& detail);
    void applyAiDashboardInsight(int generation, const finsight::core::models::AIDashboardInsight& insight);
    void applyAiDashboardRequestFailure(int generation, const QString& message);
    void resetAiSummaryButtonStyle();
    void cancelPendingAiSummaryUi();
};

#endif
