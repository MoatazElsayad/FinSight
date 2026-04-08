#ifndef BUDGETSWINDOW_H
#define BUDGETSWINDOW_H

#include <QWidget>

class QComboBox;
class QDoubleSpinBox;
class QTableWidget;
class QPushButton;
class QLabel;

class BudgetsWindow : public QWidget {
    Q_OBJECT

public:
    explicit BudgetsWindow(QWidget *parent = nullptr);

private:
    QComboBox *monthCombo;
    QComboBox *categoryCombo;
    QDoubleSpinBox *amountSpin;

    QTableWidget *budgetsTable;

    QPushButton *addButton;
    QPushButton *editButton;
    QPushButton *deleteButton;
    QPushButton *clearButton;

    QLabel *totalBudgetLabel;
    QLabel *totalSpentLabel;
    QLabel *totalRemainingLabel;

    void setupUi();
    void loadDummyData();
    void addBudgetRow(const QString &month,
                      const QString &category,
                      double budgeted,
                      double spent);
    void refreshSummary();
    void clearInputs();

private slots:
    void onAddBudget();
    void onEditBudget();
    void onDeleteBudget();
    void onTableSelectionChanged();
};

#endif
