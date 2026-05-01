#ifndef GOALSWINDOW_H
#define GOALSWINDOW_H
#include "core/managers/FinanceTrackerBackend.h"
#include <QWidget>
class QTableWidget; class QLineEdit; class QDoubleSpinBox; class QDateEdit;
class GoalsWindow: public QWidget{
    Q_OBJECT
public: explicit GoalsWindow(finsight::core::managers::FinanceTrackerBackend& backend,const std::string& userId,QWidget* parent=nullptr); void setUserId(const std::string& userId); void refreshData(); signals: void dataChanged(); private: finsight::core::managers::FinanceTrackerBackend& backend_; std::string userId_; QTableWidget* table; QLineEdit* titleEdit; QDoubleSpinBox* targetSpin; QDoubleSpinBox* currentSpin; QDateEdit* dueEdit; void setupUi();};
#endif
