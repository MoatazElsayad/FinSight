#ifndef GOALSWINDOW_H
#define GOALSWINDOW_H

#include "core/managers/FinanceTrackerBackend.h"

#include <QWidget>
#include <string>

class QDateEdit;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTableWidget;
class QTextEdit;

class GoalsWindow : public QWidget {
    Q_OBJECT

public:
    explicit GoalsWindow(finsight::core::managers::FinanceTrackerBackend& backend,
                         const std::string& userId,
                         QWidget* parent = nullptr);

    void setUserId(const std::string& userId);
    void refreshData();

signals:
    void dataChanged();

private:
    finsight::core::managers::FinanceTrackerBackend& backend_;
    std::string userId_;

    QLabel* activeCountLabel {nullptr};
    QLabel* completedCountLabel {nullptr};
    QLabel* totalTargetLabel {nullptr};

    QLineEdit* titleEdit {nullptr};
    QTextEdit* descriptionEdit {nullptr};
    QDoubleSpinBox* targetSpin {nullptr};
    QDoubleSpinBox* currentSpin {nullptr};
    QDateEdit* dueDateEdit {nullptr};
    QTableWidget* goalsTable {nullptr};
    QPushButton* updateButton {nullptr};
    QPushButton* progressButton {nullptr};
    QPushButton* deleteButton {nullptr};

    void setupUi();
    void addGoal();
    void updateSelectedGoal();
    void updateSelectedProgress();
    void deleteSelectedGoal();
    void fillFormFromSelection();
    void clearForm();
    std::string selectedGoalId() const;
};

#endif
