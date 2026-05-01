#ifndef SAVINGSWINDOW_H
#define SAVINGSWINDOW_H
#include "core/managers/FinanceTrackerBackend.h"
#include <QWidget>
class QLabel; class QTableWidget; class QDoubleSpinBox; class QLineEdit; class QDateEdit;
class SavingsWindow: public QWidget{
    Q_OBJECT
public: explicit SavingsWindow(finsight::core::managers::FinanceTrackerBackend& backend,const std::string& userId,QWidget* parent=nullptr); void setUserId(const std::string& userId); void refreshData();
signals: void dataChanged();
private: finsight::core::managers::FinanceTrackerBackend& backend_; std::string userId_; QLabel* balanceLabel; QLabel* monthlyLabel; QTableWidget* table; QDoubleSpinBox* amountSpin; QLineEdit* noteEdit; QDateEdit* dateEdit; QDoubleSpinBox* monthlyTargetSpin; QDoubleSpinBox* longTargetSpin; QDateEdit* targetDateEdit; void setupUi();};
#endif
